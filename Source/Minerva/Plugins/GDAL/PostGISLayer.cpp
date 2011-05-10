
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/GDAL/PostGISLayer.h"
#include "Minerva/Plugins/GDAL/OGRConvert.h"

#include "Minerva/Core/Data/TimeSpan.h"
#include "Minerva/Core/Data/Transform.h"

#include "Minerva/Core/Visitor.h"

#include "Minerva/OsgTools/ConvertVector.h"

#include "Usul/Bits/Bits.h"
#include "Usul/Components/Manager.h"
#include "Usul/Convert/Vector2.h"
#include "Usul/Jobs/Job.h"
#include "Usul/Jobs/Manager.h"
#include "Usul/Math/NaN.h"
#include "Usul/Threads/Safe.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Strings/Format.h"
#include "Usul/Strings/Split.h"

#include "boost/algorithm/string/erase.hpp"
#include "boost/bind.hpp"

#include "ogr_api.h"
#include "ogr_geometry.h"
#include "ogrsf_frmts.h"
#include "cpl_error.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>

using namespace Minerva::Layers::GDAL;

USUL_IO_TEXT_DEFINE_READER_TYPE_VECTOR_4 ( osg::Vec4 );
USUL_IO_TEXT_DEFINE_WRITER_TYPE_VECTOR_4 ( osg::Vec4 );
SERIALIZE_XML_DECLARE_VECTOR_4_WRAPPER ( osg::Vec4 );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

PostGISLayer::PostGISLayer() : BaseClass(),
  _dataSource ( 0x0 ),
  _tablename(),
  _labelColumn(),
  _renderBin ( 0 ),
  _connection(),
  _labelData ( new LabelData ),
  _alpha ( 1.0f ),
  _updating ( false ),
  _firstDateColumn(),
  _lastDateColumn(),
  _style ( 0x0 )
{
  this->_registerMembers();
  
  this->dirtyData ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy Constructor.
//
///////////////////////////////////////////////////////////////////////////////

PostGISLayer::PostGISLayer ( const PostGISLayer& layer )  :
  BaseClass( layer ),
  _dataSource ( 0x0 ),
  _tablename( layer._tablename ),
  _labelColumn( layer._labelColumn ),
  _renderBin ( layer._renderBin ),
  _connection( layer._connection ),
  _labelData ( new LabelData ( *layer._labelData ) ),
  _alpha ( layer._alpha ),
  _updating ( false ),
  _firstDateColumn( layer._firstDateColumn ),
  _lastDateColumn( layer._lastDateColumn ),
  _style ( layer._style )
{
  this->_registerMembers();
  
  this->dirtyData ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Register members.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::_registerMembers()
{
  SERIALIZE_XML_ADD_MEMBER ( _tablename );
  SERIALIZE_XML_ADD_MEMBER ( _labelColumn );
  SERIALIZE_XML_ADD_MEMBER ( _renderBin );
  SERIALIZE_XML_ADD_MEMBER ( _connection );
  this->_addMember ( "label", _labelData );
  SERIALIZE_XML_ADD_MEMBER ( _alpha );
  SERIALIZE_XML_ADD_MEMBER ( _firstDateColumn );
  SERIALIZE_XML_ADD_MEMBER ( _lastDateColumn );
  this->_addMember ( "style", _style );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

PostGISLayer::~PostGISLayer()
{
  if ( 0x0 != _dataSource )
  {
    ::OGR_DS_Destroy ( _dataSource );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clone this layer.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Feature* PostGISLayer::clone() const
{
  return new PostGISLayer ( *this );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the connection.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::connection ( ConnectionInfo *connection )
{
  Guard guard ( this->mutex() );
  _connection = connection;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the connection.
//
///////////////////////////////////////////////////////////////////////////////

ConnectionInfo* PostGISLayer::connection ()
{
  Guard guard ( this->mutex() );
  return _connection;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the connection.
//
///////////////////////////////////////////////////////////////////////////////

const ConnectionInfo* PostGISLayer::connection () const
{
  Guard guard ( this->mutex() );
  return _connection.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the tablename.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::tablename ( const std::string& table )
{
  Guard guard ( this->mutex() );
  _tablename = table;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the tablename.
//
///////////////////////////////////////////////////////////////////////////////

const std::string& PostGISLayer::tablename() const
{
  Guard guard ( this->mutex() );
  return _tablename;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the label column.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::labelColumn( const std::string& column )
{
  Guard guard ( this->mutex() );
  _labelColumn = column;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the label column.
//
///////////////////////////////////////////////////////////////////////////////

const std::string& PostGISLayer::labelColumn() const
{
  Guard guard ( this->mutex() );
  return _labelColumn;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the render bin.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::renderBin( Usul::Types::Uint32 bin )
{
  Guard guard ( this->mutex() );
  _renderBin = bin;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the render bin.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Types::Uint32 PostGISLayer::renderBin( ) const
{
  Guard guard ( this->mutex() );
  return _renderBin;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the show label.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::showLabel( bool b )
{
  Guard guard ( this->mutex() );
  _labelData->show ( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the show label.
//
///////////////////////////////////////////////////////////////////////////////

bool PostGISLayer::showLabel() const
{
  Guard guard ( this->mutex() );
  return _labelData->show();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Label the data object.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::_setDataObjectMembers ( Minerva::Core::Data::DataObject* dataObject, OGRFeature* feature, OGRGeometry* geometry )
{
  // Label parameters.
  dataObject->showLabel ( this->showLabel() );

  // If we have a column to use for a label.
  if( this->showLabel() && !this->labelColumn().empty() )
  {
    std::string value ( feature->GetFieldAsString ( feature->GetFieldIndex ( this->labelColumn().c_str() ) ) );
    dataObject->label ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the vector data.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::buildVectorData()
{
  // Scope the reading flag.
  Usul::Scope::Caller::RefPtr scope ( Usul::Scope::makeCaller ( 
    boost::bind ( &PostGISLayer::updating, this, true ),
    boost::bind ( &PostGISLayer::updating, this, false ) ) );
  
  // Clear what we have.
  this->clear();

  // Build the data objects.
  this->_buildDataObjects();
  
  // Our data is no longer dirty.
  this->dirtyData ( false );
  
  // Our scene needs rebuilt.
  this->dirtyScene ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Modify the vector data.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::modifyVectorData()
{
  // For now clear what we have and then rebuild.
  // Need a way to tell if the query has changed.  Then I think this can be handled better.
  this->clear();
  this->buildVectorData();
  
  // Our data is no longer dirty.
  this->dirtyData ( false );
  
  // Our scene needs rebuilt.
  this->dirtyScene ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::updateNotify ( Minerva::Core::Data::CameraState* camera, Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation )
{
  // See if our data is dirty.
  if ( true == this->dirtyData() && false == this->isUpdating() )
  {
    // Create a job to update the file.
    Usul::Jobs::Job::RefPtr job ( Usul::Jobs::create 
      ( boost::bind ( &PostGISLayer::modifyVectorData, this ) ) );

    if ( true == job.valid() )
    {
      // Set the updating flag now so we don't launch another job before this one starts.
      this->updating ( true );

      // Add job to manager.
      Usul::Jobs::Manager::instance().addJob ( job.get() );
    }
  }

  BaseClass::updateNotify ( camera, planet, elevation );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the alpha value.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::alpha ( float a )
{
  Guard guard( this->mutex() );
  _alpha = a;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the alpha value.
//
///////////////////////////////////////////////////////////////////////////////

float PostGISLayer::alpha () const
{
  Guard guard( this->mutex() );
  return _alpha;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extents.
//
///////////////////////////////////////////////////////////////////////////////

PostGISLayer::Extents PostGISLayer::calculateExtents()
{
  this->_connect();
  
  OGRDataSource *dataSource ( Usul::Threads::Safe::get ( this->mutex(), _dataSource ) );
  
  if ( 0x0 == dataSource )
    return Extents();
  
  // Tablename.
  std::string tablename ( this->tablename() );
  
  OGRLayer *layer ( dataSource->GetLayerByName ( tablename.c_str() ) );
  
  OGREnvelope envelope;
  layer->GetExtent ( &envelope );
  
  Extents::Vertex lowerLeft ( envelope.MinX, envelope.MinY );
  Extents::Vertex upperRight ( envelope.MaxX, envelope.MaxY ); 
  
  return Extents ( lowerLeft, upperRight );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the projection as "Well Known Text".
//
///////////////////////////////////////////////////////////////////////////////

std::string PostGISLayer::projectionWKT()
{
  this->_connect();
  
  OGRDataSource *dataSource ( Usul::Threads::Safe::get ( this->mutex(), _dataSource ) );
  
  if ( 0x0 == dataSource )
    return std::string();
  
  // Tablename.
  std::string tablename ( this->tablename() );
  
  OGRLayer *layer ( dataSource->GetLayerByName ( tablename.c_str() ) );
  
  OGRSpatialReference* srs ( layer->GetSpatialRef() );
  if ( 0x0 != srs )
  {
    char *projection ( 0x0 );
    srs->exportToWkt ( &projection );
    const std::string result ( projection );
    ::CPLFree ( projection );
  }
  
  return std::string();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the updating state.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::updating( bool b )
{
  Guard guard ( this );
  _updating = b;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the updating state.
//
///////////////////////////////////////////////////////////////////////////////

bool PostGISLayer::isUpdating() const
{
  Guard guard ( this );
  return _updating;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the data objects.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::_buildDataObjects()
{
  this->_connect();
  
  OGRDataSource* vectorData ( Usul::Threads::Safe::get ( this->mutex(), _dataSource ) );
  
  // Return if no data.
  if ( 0x0 == vectorData )
    return;

  // Make the query.
  std::string query ( Usul::Strings::format ( "SELECT * ", " FROM ", this->tablename() ) );

  // Make a layer.
  OGRLayer *layer ( vectorData->ExecuteSQL ( query.c_str(), 0x0, 0x0 ) );

  if ( 0x0 == layer )
    return;

  // Get the spatial reference.
  OGRSpatialReference *src ( layer->GetSpatialRef() );

  // We are going to wgs 84.
  OGRSpatialReference dst;
  dst.SetWellKnownGeogCS ( "WGS84" );
  OGRCoordinateTransformation *transform ( ::OGRCreateCoordinateTransformation( src, &dst ) );

  // Make sure the transformation is destroyed.
  Usul::Scope::Caller::RefPtr destroyTransform ( Usul::Scope::makeCaller ( boost::bind<void> ( ::OCTDestroyCoordinateTransformation, transform ) ) );

  layer->ResetReading();
  
  // The data table.
  std::string dataTable ( this->tablename() );
  
  // Loop through the results.
  OGRFeature *feature ( 0x0 );
  while ( 0x0 != ( feature = layer->GetNextFeature() ) )
  {
    try
    {
      // Make the data object.
      Minerva::Core::Data::DataObject::RefPtr data ( new Minerva::Core::Data::DataObject );
      
      // Set date parameters.
      const std::string firstDateColumn ( this->firstDateColumn () );
      const std::string lastDateColumn ( this->lastDateColumn () );

      // Get first and last date if we have valid columns for them.
      if ( false == firstDateColumn.empty() && false == lastDateColumn.empty() )
      {
        // Get first and last date.
        std::string firstDate ( feature->GetFieldAsString ( firstDateColumn.c_str() ) );
        std::string lastDate  ( feature->GetFieldAsString ( lastDateColumn.c_str()  ) );
       
        // Increment last day so animation works properly.
        Minerva::Core::Data::Date last ( lastDate ); 
        last.increment ( Minerva::Core::Data::Date::INCREMENT_DAY, 1.0 );
       
        Minerva::Core::Data::TimeSpan::RefPtr span ( new Minerva::Core::Data::TimeSpan );
        span->begin ( Minerva::Core::Data::Date ( firstDate ) );
        span->end ( last );
        data->timePrimitive ( span.get() );
      }
      
      // Get the geometry.
      OGRGeometry *ogrGeometry ( feature->GetGeometryRef() );

      Minerva::Core::Data::Geometry::RefPtr geometry ( Minerva::Layers::GDAL::OGRConvert::geometry ( ogrGeometry, transform ) );
      data->geometry ( geometry );
      
      Minerva::Core::Data::Style::RefPtr style ( this->style() );
      data->style ( style );

      if ( geometry.valid() )
      {
        // Set the geometry's data.
        geometry->renderBin ( this->renderBin() );
      }

      // Set the common members.
      this->_setDataObjectMembers ( data.get(), feature, ogrGeometry );
      
      // Add the data to the container.
      this->add ( data.get(), false );
    }
    catch ( const std::exception& e )
    {
      std::cout << "Error 6442903120: " << e.what() << std::endl;
    }
    catch ( ... )
    {
      std::cout << "Error 1112177078: Exception caught while adding data to layer." << std::endl;
    }
  }
  
  // Notify now that the data has changed.
  this->_notifyDataChangedListeners();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the date column.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::firstDateColumn( const std::string& dateColumn )
{
  Guard guard ( this );
  _firstDateColumn = dateColumn;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the date column.
//
///////////////////////////////////////////////////////////////////////////////

const std::string& PostGISLayer::firstDateColumn() const
{
  Guard guard ( this );
  return _firstDateColumn;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the date column.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::lastDateColumn( const std::string& dateColumn )
{
  Guard guard ( this );
  _lastDateColumn = dateColumn;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the date column.
//
///////////////////////////////////////////////////////////////////////////////

const std::string& PostGISLayer::lastDateColumn() const
{
  Guard guard ( this );
  return _lastDateColumn;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Serialize.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::serialize ( XmlTree::Node &parent ) const
{
  Serialize::XML::DataMemberMap dataMemberMap ( Usul::Threads::Safe::get ( this->mutex(), _dataMemberMap ) );
  
  // Don't serialize the layers.
  dataMemberMap.erase ( "layers" );
  
  // Serialize.
  dataMemberMap.serialize ( parent );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the style.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::style ( Style::RefPtr style )
{
  Guard guard ( this->mutex() );
  _style = style;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the style.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Style::RefPtr PostGISLayer::style() const
{
  Guard guard ( this->mutex() );
  return _style;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Connect to the database.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayer::_connect()
{
  // Throw now if there is no connection.
  if ( 0x0 == this->connection() )
    throw std::runtime_error ( "Error 1770160824: A valid connection is needed." );
  
  // Set our extents.
  this->extents ( this->calculateExtents() );
  
  const std::string driverName ( "PostgreSQL" );
  OGRSFDriver *driver ( OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName ( driverName.c_str() ) );
  
  // Return if we didn't find a driver.
  if ( 0x0 == driver )
    return;
  
  // Get the data source.
  const std::string connectionString ( Usul::Strings::format ( "PG:", this->connection()->connectionString() ) );
  
  Guard guard ( this->mutex() );
  
  if ( 0x0 != _dataSource )
  {
    ::OGR_DS_Destroy ( _dataSource );
  }
  
  _dataSource = driver->CreateDataSource ( connectionString.c_str(), 0x0 );
}
