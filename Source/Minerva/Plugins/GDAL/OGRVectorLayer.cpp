
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/GDAL/OGRVectorLayer.h"
#include "Minerva/Plugins/GDAL/OGRConvert.h"

#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/Point.h"
#include "Minerva/Core/Data/Line.h"
#include "Minerva/Core/Data/Polygon.h"
#include "Minerva/Core/Data/MultiGeometry.h"

#include "Usul/Factory/RegisterCreator.h"
#include "Usul/Interfaces/IProgressBar.h"
#include "Usul/Scope/Caller.h"

#include "boost/bind.hpp"
#include "boost/filesystem.hpp"

#include "ogr_api.h"
#include "ogrsf_frmts.h"

using namespace Minerva::Layers::GDAL;


USUL_FACTORY_REGISTER_CREATOR ( OGRVectorLayer );

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

OGRVectorLayer::OGRVectorLayer() : BaseClass(),
  _filename(),
  _defaultStyle ( new Minerva::Core::Data::Style ),
  _verticalOffset ( 0.0 )
{
  typedef Minerva::Core::Data::LineStyle LineStyle;
  typedef Minerva::Core::Data::PolyStyle PolyStyle;

  LineStyle::RefPtr lineStyle ( new LineStyle );
  lineStyle->width ( 1.0f );
  lineStyle->color ( Usul::Math::Vec4f ( 1.0f, 1.0f, 0.0f, 1.0f ) );

  PolyStyle::RefPtr polyStyle ( new PolyStyle );
  polyStyle->fill ( true );
  polyStyle->outline ( true );
  polyStyle->color ( Usul::Math::Vec4f ( 0.8f, 0.8f, 0.8f, 1.0f ) );

  using Minerva::Core::Data::PointStyle;
  PointStyle::RefPtr pointStyle ( new PointStyle );
  pointStyle->size ( 5 );
  pointStyle->primitiveId ( PointStyle::SPHERE );
  pointStyle->color ( Usul::Math::Vec4f ( 1.0, 0.0, 0.0, 1.0 ) );

  _defaultStyle->linestyle ( lineStyle.get() );
  _defaultStyle->polystyle ( polyStyle.get() );
  _defaultStyle->pointstyle ( pointStyle.get() );

  this->_addMember ( "filename", _filename );
  this->_addMember ( "style", _defaultStyle );
  this->_addMember ( "verticalOffset", _verticalOffset );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

OGRVectorLayer::~OGRVectorLayer()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read the file.
//
///////////////////////////////////////////////////////////////////////////////

void OGRVectorLayer::read ( const std::string &filename, Usul::Interfaces::IUnknown *caller, Usul::Interfaces::IUnknown *progress )
{
  _filename = filename;

  this->name ( boost::filesystem::basename ( filename ) );

  OGRDataSource *dataSource ( OGRSFDriverRegistrar::Open ( filename.c_str(), FALSE ) );
  if( 0x0 == dataSource )
  {
    throw std::runtime_error ( "Error 5296869660: Could not open file: " + filename );
  }

  // Get the number of layers.
  const int layers ( dataSource->GetLayerCount() );

  // Check to make sure we have layers.
  if ( layers <= 0 )
    throw std::runtime_error ( "Error 2518191347: " + filename + " contains no layers." );

  // Loop over the layers.
  for ( int i = 0; i < layers; ++i )
  {
    this->_addLayer ( dataSource->GetLayer ( i ), progress );
    
    // Notify any listeners that the data has changed.
    this->_notifyDataChangedListeners();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add a layer.
//
///////////////////////////////////////////////////////////////////////////////

void OGRVectorLayer::_addLayer ( OGRLayer* layer, Usul::Interfaces::IUnknown *unknown )
{
  if ( 0x0 == layer )
    return;
  
  // Query for a progress bar.
  Usul::Interfaces::IProgressBar::UpdateProgressBar progress ( 0.0, 1.0, unknown );

  // Get the spatial reference.
  OGRSpatialReference *src ( layer->GetSpatialRef() );

  // We are going to wgs 84.
  OGRSpatialReference dst;
  dst.SetWellKnownGeogCS ( "WGS84" );
  OGRCoordinateTransformation *transform ( ::OGRCreateCoordinateTransformation( src, &dst ) );

  // Make sure the transformation is destroyed.
  Usul::Scope::Caller::RefPtr destroyTransform ( Usul::Scope::makeCaller ( boost::bind<void> ( ::OCTDestroyCoordinateTransformation, transform ) ) );
  
  // Get the number of features.
  const unsigned int numFeatures ( layer->GetFeatureCount() );

  layer->ResetReading();

  OGRFeature *feature ( 0x0 );
  
  unsigned int i ( 0 );

  // Get the features.
  while ( 0x0 != ( feature = layer->GetNextFeature() ) )
  {
    // Update the progress.
    progress ( ++i, numFeatures );
    
    // Get the geometry.
    OGRGeometry *ogrGeometry ( feature->GetGeometryRef() );

    if ( 0x0 != ogrGeometry )
    {
      // Make a data object.
      Minerva::Core::Data::DataObject::RefPtr dataObject ( new Minerva::Core::Data::DataObject );
      dataObject->style ( _defaultStyle );

      Minerva::Core::Data::Geometry::RefPtr geometry ( OGRConvert::geometry ( ogrGeometry, transform, _verticalOffset ) );

      if ( geometry )
      {
        if ( _verticalOffset != 0.0 )
        {
          geometry->altitudeMode ( Minerva::Core::Data::ALTITUDE_MODE_RELATIVE_TO_GROUND );
        }

        dataObject->geometry ( geometry.get() );
      }

      // Add the data object.
      this->add ( dataObject, false );
    }

    // Destroy the feature.
    OGRFeature::DestroyFeature ( feature );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Deserialize.
//
///////////////////////////////////////////////////////////////////////////////

void OGRVectorLayer::deserialize( const XmlTree::Node &node )
{
  BaseClass::deserialize ( node );
  this->read ( _filename );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Serialize.
//
///////////////////////////////////////////////////////////////////////////////

void OGRVectorLayer::serialize ( XmlTree::Node &parent ) const
{
  Serialize::XML::DataMemberMap dataMemberMap ( _dataMemberMap );

  // Don't serialize the layers.
  dataMemberMap.erase ( "layers" );

  // Serialize.
  dataMemberMap.serialize ( parent );
}
