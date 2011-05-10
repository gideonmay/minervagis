
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Perry L Miller IV
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Body class.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/TileEngine/Body.h"
#include "Minerva/Core/Data/Camera.h"
#include "Minerva/Core/Data/CameraState.h"
#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/Line.h"
#include "Minerva/Core/Utilities/Atmosphere.h"
#include "Minerva/Core/Visitor.h"

#include "Minerva/OsgTools/ConvertVector.h"
#include "Minerva/OsgTools/ConvertMatrixTransform.h"
#include "Minerva/OsgTools/Group.h"
#include "Minerva/OsgTools/Visitor.h"

#include "Usul/Factory/RegisterCreator.h"
#include "Usul/Functions/Execute.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Math/MinMax.h"
#include "Usul/Registry/Database.h"
#include "Usul/Threads/Named.h"
#include "Usul/Threads/Safe.h"

#include "osg/MatrixTransform"

#include "osgUtil/LineSegmentIntersector"

#include "boost/bind.hpp"

#include <limits>
#include <stdexcept>

using namespace Minerva::Core::TileEngine;

USUL_FACTORY_REGISTER_CREATOR ( Body );
MINERVA_IMPLEMENT_NODE_CLASS ( Body );
USUL_IMPLEMENT_IUNKNOWN_MEMBERS ( Body, Body::BaseClass );

USUL_IO_TEXT_DEFINE_READER_TYPE_VECTOR_4 ( Extents );
USUL_IO_TEXT_DEFINE_WRITER_TYPE_VECTOR_4 ( Extents );
SERIALIZE_XML_DECLARE_VECTOR_4_WRAPPER ( Extents );

const unsigned int ELEVATION_CONTAINER ( 0 );
const unsigned int RASTER_CONTAINER ( 1 );
const unsigned int VECTOR_CONTAINER ( 2 );

///////////////////////////////////////////////////////////////////////////////
//
//  The number for the renderbin used by the vector data.
//
///////////////////////////////////////////////////////////////////////////////

const int VECTOR_RENDER_BIN_NUMBER ( 10000 );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
///////////////////////////////////////////////////////////////////////////////

Body::Body ( unsigned int numberOfRows,
             unsigned int numberOfColumns,
             Extents extents,
             LandModel *land, 
             Usul::Jobs::Manager *manager, 
             const MeshSize &ms, 
             const ImageSize &is, 
             double splitDistance ) : BaseClass(),
  _transform ( new osg::MatrixTransform ),
  _landModel ( land ),
  _container ( new Container ),
  _manager ( manager ),
  _maxLevel ( 50 ),
  _splitDistance ( splitDistance ),
  _meshSize ( ms ),
  _useSkirts ( true ),
  _useBorders ( true ),
  _splitCallback ( 0x0 ),
  _deleteTiles(),
  _topTiles(),
  _allowSplit ( true ),
  _keepDetail ( false ),
  _sky ( 0x0 ),
  _needsRedraw ( false ),
  _log ( 0x0 ),
  _name( "Body" ),
  _imageSize ( is ),
  _alpha ( 1.0f ),
  _maxAnisotropy ( Usul::Registry::Database::instance()["default_max_anisotropy"].get<float> ( 16.0f, true ) ),
  _numberOfRows ( numberOfRows ),
  _numberOfColumns ( numberOfColumns ),
  _extents ( extents ),
  SERIALIZE_XML_INITIALIZER_LIST
{
  _container->add ( new Container );
  _container->add ( new Container );
  _container->add ( new Container );
  
  // Serialization setup.
  this->_addMember ( "land_model", _landModel );
  this->_addMember ( "container", _container );
  this->_addMember ( "max_level", _maxLevel );
  this->_addMember ( "split_distance", _splitDistance );
  this->_addMember ( "mesh_size", _meshSize );
  this->_addMember ( "use_skirts", _useSkirts );
  this->_addMember ( "use_borders", _useBorders );
  this->_addMember ( "split_callback", _splitCallback );
  this->_addMember ( "name", _name );
  this->_addMember ( "image_size", _imageSize );
  this->_addMember ( "alpha", _alpha );
  this->_addMember ( "max_anisotropic_texture_filtering", _maxAnisotropy );
  this->_addMember ( "number_of_rows", _numberOfRows );
  this->_addMember ( "number_of_columns", _numberOfColumns );
  this->_addMember ( "extents", _extents );

  // Set the names.
  _container->feature ( ELEVATION_CONTAINER )->name ( "Elevation" );
  _container->feature ( RASTER_CONTAINER )->name ( "Rasters" );
  _container->feature ( VECTOR_CONTAINER )->name ( "Vector" );

  // Add the vector data to the transform. Make sure it is drawn last.
  Container::RefPtr vector ( _container->feature ( VECTOR_CONTAINER )->asContainer() );
  osg::ref_ptr<osg::Node> vectorData ( vector->getScene() );
  vectorData->getOrCreateStateSet()->setRenderBinDetails ( VECTOR_RENDER_BIN_NUMBER, "RenderBin" );
  _transform->addChild ( vectorData.get() );
  
  this->_addTiles ( numberOfRows, numberOfColumns, extents );

#if 0
  // Make the sky.
  _sky = new Minerva::Core::Utilities::Atmosphere;
  
  // Set parameters.
  _sky->innerRadius ( land->size() );
  _sky->outerRadius ( _sky->innerRadius() + 400000 );
  
  // Add the sky to the scene.
  _transform->addChild ( _sky );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor
//
///////////////////////////////////////////////////////////////////////////////

Body::~Body()
{
  Usul::Functions::safeCall ( boost::bind ( &Body::_destroy, this ), "3973302267" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destroy
//
///////////////////////////////////////////////////////////////////////////////

void Body::_destroy()
{
  this->clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to safely clear and set to null.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  template < class SmartPointerType > void safeClear ( SmartPointerType ptr )
  {
    if ( true == ptr.valid() )
    {
      ptr->clear();
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear the body.
//
///////////////////////////////////////////////////////////////////////////////

void Body::clear()
{
  Guard guard ( this );

  _transform = 0x0;
  _landModel = 0x0;

  Helper::safeClear ( _container ); _container = 0x0;

  _splitCallback = 0x0;

  Usul::Functions::executeMemberFunctions ( _topTiles,    &Tile::clear, true ); _topTiles.clear();
  Usul::Functions::executeMemberFunctions ( _deleteTiles, &Tile::clear, true ); _deleteTiles.clear();

  _sky = 0x0;
  _log = 0x0;

  _name.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query the interfaces
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown *Body::queryInterface ( unsigned long iid )
{
  switch ( iid )
  {
  case Usul::Interfaces::IUnknown::IID:
  case Minerva::Common::IPlanetCoordinates::IID:
    return static_cast < Minerva::Common::IPlanetCoordinates* > ( this );
  case Minerva::Common::IElevationDatabase::IID:
    return static_cast < Minerva::Common::IElevationDatabase * > ( this );
  default:
    return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add a tile.
//
///////////////////////////////////////////////////////////////////////////////

void Body::_addTile ( unsigned int row, unsigned int column, const Extents& extents )
{
  Guard guard ( this );

  // Make the tile.
  Minerva::Common::TileKey::RefPtr info ( new Minerva::Common::TileKey );
  info->row ( row );
  info->column ( column );
  info->level ( 0 );
  info->meshSize ( this->meshSize() );
  info->imageSize ( _imageSize );
  info->extents ( extents );

  Tile::RefPtr tile ( new Tile ( info, _splitDistance, this ) );

  // Add tile to the transform.
  _transform->addChild ( tile.get() );

  // Add tile to top-level tiles.
  _topTiles.push_back ( tile );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add tiles.
//
///////////////////////////////////////////////////////////////////////////////

void Body::_addTiles ( unsigned int numberOfRows, unsigned int numberOfColumns, const Extents& extents )
{
  const double startingLon ( extents.minLon() );
  const double startingLat ( extents.minLat() );
  
  const double deltaLon ( ( extents.maxLon() - extents.minLon() ) / numberOfColumns );
  const double deltaLat ( ( extents.maxLat() - extents.minLat() ) / numberOfRows );
  
  for ( unsigned int row = 0; row < numberOfRows; ++row )
  {
    const double minLat ( startingLat + ( deltaLat * row ) );
    const double maxLat ( minLat + deltaLat );
    
    for ( unsigned int column = 0; column < numberOfColumns; ++column )
    {
      const double minLon ( startingLon + ( deltaLon * column ) );
      const double maxLon ( minLon + deltaLon );
      
      Extents extents ( minLon, minLat, maxLon, maxLat );
      
      this->_addTile ( row, column, extents );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Called by the tile when it's no longer needed.
//
///////////////////////////////////////////////////////////////////////////////

void Body::_addTileToBeDeleted ( Tile::RefPtr tile )
{
  Guard guard ( this );

  if ( true == tile.valid() )
  {
    _deleteTiles.push_back ( tile );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the scene.
//
///////////////////////////////////////////////////////////////////////////////

const osg::Node *Body::scene() const
{
  Guard guard ( this );
  return _transform.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the scene.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node *Body::scene()
{
  Guard guard ( this );
  return _transform.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Append raster data.
//
///////////////////////////////////////////////////////////////////////////////

void Body::rasterAppend ( Minerva::Core::Data::Feature * layer )
{
  Guard guard ( this );

  Container::RefPtr rasters ( _container->feature ( RASTER_CONTAINER )->asContainer() );
  if ( ( true == rasters.valid() ) && ( 0x0 != layer ) )
  {
    // Append the layer to the existing group.
    rasters->add ( layer );
    
    // Get the extents.
    Extents e ( layer->extents() );

    // Dirty the tiles.
    this->dirtyTextures ( e );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove raster layer.
//
///////////////////////////////////////////////////////////////////////////////

void Body::rasterRemove ( Minerva::Core::Data::Feature *layer )
{
  Guard guard ( this );
  
  Container::RefPtr rasters ( _container->feature ( RASTER_CONTAINER )->asContainer() );
  if ( ( true == rasters.valid() ) && ( 0x0 != layer ) )
  {
    // Append the layer to the existing group.
    rasters->remove ( layer );
    
    // Get the extents.
    Extents e ( layer->extents() );
    
    // Dirty the tiles.
    this->dirtyTextures ( e );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Raster has chagned.
//
///////////////////////////////////////////////////////////////////////////////

void Body::rasterChanged ( Minerva::Core::Data::Feature *layer )
{
  if ( 0x0 != layer )
  {
    // Get the extents.
    Extents e ( layer->extents() );
    
    // Dirty the tiles.
    this->dirtyTextures ( e );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  dirty textures.
//
///////////////////////////////////////////////////////////////////////////////

void Body::dirtyTextures ( const Extents& e )
{
  this->_dirtyTiles ( Tile::IMAGE, e );
}


///////////////////////////////////////////////////////////////////////////////
//
//  dirty vertices.
//
///////////////////////////////////////////////////////////////////////////////

void Body::dirtyVertices()
{
  this->_dirtyTiles ( Tile::VERTICES );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Dirty the tiles.
//
///////////////////////////////////////////////////////////////////////////////

void Body::_dirtyTiles ( unsigned int flags )
{
  Tiles tiles ( Usul::Threads::Safe::get ( this->mutex(), _topTiles ) );
  for ( Tiles::iterator iter = tiles.begin(); iter != tiles.end(); ++iter )
  {
    Tile::RefPtr tile ( *iter );
    if ( tile.valid() )
      tile->dirty ( true, flags, true );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Dirty the tiles within extents.
//
///////////////////////////////////////////////////////////////////////////////

void Body::_dirtyTiles ( unsigned int flags, const Extents& extents )
{
  Tiles tiles ( Usul::Threads::Safe::get ( this->mutex(), _topTiles ) );
  for ( Tiles::iterator iter = tiles.begin(); iter != tiles.end(); ++iter )
  {
    Tile::RefPtr tile ( *iter );
    if ( tile.valid() )
      tile->dirty ( true, flags, true, extents );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the mesh's alpha.
//
///////////////////////////////////////////////////////////////////////////////

void Body::updateTilesAlpha()
{
  osg::ref_ptr<osg::NodeVisitor> visitor ( OsgTools::MakeVisitor<osg::Group>::make 
    ( boost::bind ( &Body::_updateTileAlpha, this, _1 ) ) );
  _transform->accept ( *visitor );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the tile's alpha.
//
///////////////////////////////////////////////////////////////////////////////

void Body::_updateTileAlpha ( osg::Group *group )
{
  Tile::RefPtr tile ( dynamic_cast < Tile * > ( group ) );
  if ( true == tile.valid() )
  {
    tile->updateAlpha();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the raster data.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Container::RefPtr Body::rasterData()
{
  Guard guard ( this );
  return Container::RefPtr ( _container->feature ( RASTER_CONTAINER )->asContainer() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert lat,lon,height to x,y,z.
//
///////////////////////////////////////////////////////////////////////////////

void Body::latLonHeightToXYZ ( double lat, double lon, double elevation, osg::Vec3d& point ) const
{
  Guard guard ( this );

  if ( true == _landModel.valid() )
  {
    _landModel->latLonHeightToXYZ ( lat, lon, elevation, point.x(), point.y(), point.z() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert x,y,z to lat, lon, height.
//
///////////////////////////////////////////////////////////////////////////////

void Body::xyzToLatLonHeight ( const osg::Vec3d& point, double& lat, double& lon, double& elevation ) const
{
  Guard guard ( this );

  if ( true == _landModel.valid() )
  {
    _landModel->xyzToLatLonHeight ( point.x(), point.y(), point.z(), lat, lon, elevation );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert to planet coordinates.
//
///////////////////////////////////////////////////////////////////////////////

void Body::convertToPlanet ( const Usul::Math::Vec3d& orginal, Usul::Math::Vec3d& planetPoint ) const
{
  osg::Vec3d out;
  this->latLonHeightToXYZ ( orginal[1], orginal[0], orginal[2], out );
  planetPoint.set ( out[0], out[1], out[2] );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert from planet coordinates.
//
///////////////////////////////////////////////////////////////////////////////

void Body::convertFromPlanet ( const Usul::Math::Vec3d& planetPoint, Usul::Math::Vec3d& lonLatPoint ) const
{
  this->xyzToLatLonHeight ( osg::Vec3d ( planetPoint[0], planetPoint[1], planetPoint[2] ), lonLatPoint[1], lonLatPoint[0], lonLatPoint[2] );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Matrix to place items on the planet (i.e. local coordinates to world coordinates).
//
///////////////////////////////////////////////////////////////////////////////

osg::Matrixd Body::planetRotationMatrix ( double lat, double lon, double elevation, double heading ) const
{
  Guard guard ( this );
  
  typedef LandModel::Matrix Matrix;
  Matrix matrix ( _landModel.valid() ? _landModel->planetRotationMatrix ( lat, lon, elevation, heading ) : Matrix() );

  osg::Matrixd result;
  OsgTools::Convert::matrix ( matrix, result );
  return result;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the job manager for this body.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Jobs::Manager *Body::jobManager()
{
  JobManager::GuardType guard ( _manager.mutex() );
  return _manager.value();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the job manager.
//
///////////////////////////////////////////////////////////////////////////////

void Body::jobManager ( Usul::Jobs::Manager *manager )
{
  JobManager::GuardType guard ( _manager.mutex() );
  _manager = manager;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Request texture.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Jobs::BuildRaster::RefPtr Body::textureRequest ( Tile* tile )
{
  Guard guard ( this );

  if ( 0x0 == this->jobManager() )
  {
    // Have to throw because there's no value we can return.
    throw std::runtime_error ( "Error 3925869673: Job manager is null" );
  }

  BuildRaster::RefPtr job ( new BuildRaster ( Tile::RefPtr ( tile ) ) );
  this->jobManager()->addJob ( job.get() );

  return job;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the maximum level.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int Body::maxLevel() const
{
  Guard guard ( this );
  return _maxLevel;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the maximum level.
//
///////////////////////////////////////////////////////////////////////////////

void Body::maxLevel ( unsigned int level )
{
  Guard guard ( this );
  _maxLevel = level;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the mesh size for the extents.
//
///////////////////////////////////////////////////////////////////////////////

MeshSize Body::meshSize() const
{
  Guard guard ( this );
  return _meshSize;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag to use skirts.
//
///////////////////////////////////////////////////////////////////////////////

void Body::useSkirts ( bool use )
{
  Guard guard ( this );

  // Set the flag.
  _useSkirts = use;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the flag to use skirts.
//
///////////////////////////////////////////////////////////////////////////////

bool Body::useSkirts() const
{
  Guard guard ( this );
  return _useSkirts;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag to use borders.
//
///////////////////////////////////////////////////////////////////////////////

void Body::useBorders ( bool use )
{
  Guard guard ( this );

  // Set the flag.
  _useBorders = use;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the flag to use borders.
//
///////////////////////////////////////////////////////////////////////////////

bool Body::useBorders() const
{
  Guard guard ( this );
  return _useBorders;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Append elevation data.
//
///////////////////////////////////////////////////////////////////////////////

void Body::elevationAppend ( Minerva::Core::Data::Feature * layer )
{
  Guard guard ( this );

  Container::RefPtr elevation ( _container->feature ( ELEVATION_CONTAINER )->asContainer() );
  if ( ( true == elevation.valid() ) && ( 0x0 != layer ) )
  {
    // Append the layer to the existing group.
    elevation->add ( layer );
    
    // Get the extents.
    Extents e ( layer->extents() );

    this->dirtyVertices();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the elevation data.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Container::RefPtr Body::elevationData()
{
  Guard guard ( this );
  return Container::RefPtr ( _container->feature ( ELEVATION_CONTAINER )->asContainer() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  See if the tile should split.
//
///////////////////////////////////////////////////////////////////////////////

bool Body::shouldSplit ( bool suggestion, Tile *tile )
{
  SplitCallback::RefPtr callback ( this->splitCallback() );
  return ( ( true == callback.valid() ) ? callback->shouldSplit ( suggestion, tile ) : suggestion );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the split callback..
//
///////////////////////////////////////////////////////////////////////////////

void Body::splitCallback ( SplitCallback *cb )
{
  Guard guard ( this );
  _splitCallback = cb;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the split callback..
//
///////////////////////////////////////////////////////////////////////////////

Body::SplitCallback::RefPtr Body::splitCallback()
{
  Guard guard ( this );
  if ( false == _splitCallback.valid() )
  {
    _splitCallback = new Minerva::Core::TileEngine::Callbacks::PassThrough;
  }
  return SplitCallback::RefPtr ( _splitCallback );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Serialize the members.
//
///////////////////////////////////////////////////////////////////////////////

void Body::serialize ( XmlTree::Node &parent ) const
{
  Guard guard ( this );

  // Copy the map.
  Serialize::XML::DataMemberMap m ( _dataMemberMap );

  // Write members to the node from local map.
  m.serialize ( parent );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Deserialize the members.
//
///////////////////////////////////////////////////////////////////////////////

void Body::deserialize ( const XmlTree::Node &node )
{
  Guard guard ( this );

  // Copy the map.
  Serialize::XML::DataMemberMap m ( _dataMemberMap );

  // Initialize locals and members from the the node.
  m.deserialize ( node );

  // Add the tiles.
  _transform->removeChild ( 0, _transform->getNumChildren() );
  _topTiles.clear();
  this->_addTiles ( _numberOfRows, _numberOfColumns, _extents );
  
  // Re-add these scenes to the transform because a new one was just created.

  // Make sure the line is drawn last.
  Container::RefPtr vector ( _container->feature ( VECTOR_CONTAINER )->asContainer() );
  if ( vector.valid() )
  {
    osg::ref_ptr<osg::Node> vectorData ( vector->getScene() );
    vectorData->getOrCreateStateSet()->setRenderBinDetails ( VECTOR_RENDER_BIN_NUMBER, "RenderBin" );
    _transform->addChild ( vectorData.get() );
  }

  this->dirtyTextures ( _extents );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the split distance.
//
///////////////////////////////////////////////////////////////////////////////

void Body::splitDistance ( double d, bool children )
{
  Guard guard ( this );
  _splitDistance = d;

  for ( Tiles::iterator iter = _topTiles.begin(); iter != _topTiles.end(); ++iter )
  {
    // Set new split distance.
    Tiles::value_type tile ( *iter );
    tile->splitDistance ( d, children );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the split distance.
//
///////////////////////////////////////////////////////////////////////////////

double Body::splitDistance() const
{
  Guard guard ( this );
  return _splitDistance;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the elevation from a tile.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  bool elevationFromTile ( Tile::RefPtr tile, const Extents::Vertex& p, double& elevation )
  {
    if ( false == tile.valid() )
      return false;

    Extents e ( tile->extents() );

    if ( e.contains ( p ) )
    {
      if ( tile->isLeaf() )
      {
        elevation = tile->elevation ( p[1], p[0] );
        return true;
      }
      else
      {
        // Ask the children.
        if ( Detail::elevationFromTile ( tile->childAt ( 0 ), p, elevation ) ) return true;
        if ( Detail::elevationFromTile ( tile->childAt ( 1 ), p, elevation ) ) return true;
        if ( Detail::elevationFromTile ( tile->childAt ( 2 ), p, elevation ) ) return true;
        if ( Detail::elevationFromTile ( tile->childAt ( 3 ), p, elevation ) ) return true;
      }
    }

    return false;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the elevation at a lat, lon.
//
///////////////////////////////////////////////////////////////////////////////

double Body::elevation ( double lat, double lon ) const
{
  Tiles tiles ( Usul::Threads::Safe::get ( this->mutex(), _topTiles ) );
  
  Extents::Vertex v ( lon, lat );
  double elevation ( 0.0 );

  for ( Tiles::const_iterator iter = tiles.begin(); iter != tiles.end(); ++iter )
  {
    if ( Detail::elevationFromTile ( *iter, v, elevation ) ) 
      break;
  }
  
  return elevation;
  
  // Should we use a geoid here?  Keeping this for reference.
  // http://en.wikipedia.org/wiki/Geoid
#if 0
  ossimGpt point ( lat, lon );
  double height (  ossimElevManager::instance()->getHeightAboveMSL( point ) );
  if( ossim::isnan ( height ) )
    height = 0.0;
  
  return height + ossimGeoidManager::instance()->offsetFromEllipsoid( point );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Purge all tiles that are ready.
//
///////////////////////////////////////////////////////////////////////////////

void Body::purgeTiles()
{
  // Swap with the list to delete.
  Tiles deleteMe;
  {
    Guard guard ( this );
    if ( false == _deleteTiles.empty() )
    {
      _deleteTiles.swap ( deleteMe );
    }
  }

  // Clear the list of tiles to be deleted.
  if ( false == deleteMe.empty() )
  {
    Usul::Functions::safeCall ( boost::bind ( &Tiles::clear, &deleteMe ), "4101333810" );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update.
//
///////////////////////////////////////////////////////////////////////////////

void Body::updateNotify ( Minerva::Core::Data::Camera* camera )
{
  Guard guard ( this );

  // Query for needed interfaces.
  Minerva::Common::IElevationDatabase::QueryPtr elevation ( this );
  Minerva::Common::IPlanetCoordinates::QueryPtr planet ( this );
  
  typedef Minerva::Core::Data::CameraState CameraState;
  typedef CameraState::Matrix Matrix;
  
  double longitude ( 0.0 );
  double latitude ( 0.0 );
  double altitude ( 0.0 );
  double heading ( 0.0 );
  double tilt ( 0.0 );
  double roll ( 0.0 );
  Matrix matrix;
  
  if ( camera )
  {
    longitude = camera->longitude();
    latitude = camera->latitude();
    altitude = camera->altitude();
    heading = camera->heading();
    tilt = camera->tilt();
    roll = camera->roll();
    matrix = camera->viewMatrix ( this->landModel() );
  }
  
  CameraState::RefPtr cameraState ( new CameraState ( longitude, latitude, altitude, heading, tilt, roll, matrix ) );
  
  // Update the vector group.
  _container->updateNotify ( cameraState, planet.get(), elevation.get() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the number of children.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int Body::getNumChildNodes() const
{
  return _container->size();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the child node (ITreeNode).
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Feature::RefPtr Body::getChildNode ( unsigned int which )
{
  Guard guard ( this );
  return _container->feature ( which );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add vector data.
//
///////////////////////////////////////////////////////////////////////////////

void Body::vectorAppend ( Minerva::Core::Data::Feature *feature )
{
  Guard guard ( this );
  
  Container::RefPtr vector ( _container->feature ( VECTOR_CONTAINER )->asContainer() );
  if ( vector.valid() )
  {
    // Add the layer to our group.
    vector->add ( feature );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the vector data.
//
///////////////////////////////////////////////////////////////////////////////

Body::Container::RefPtr Body::vectorData()
{
  Guard guard ( this );
  return Container::RefPtr ( _container->feature ( VECTOR_CONTAINER )->asContainer() );;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove vector data.
//
///////////////////////////////////////////////////////////////////////////////

void Body::vectorRemove ( Minerva::Core::Data::Feature* feature )
{
  Guard guard ( this );
  
  // Add the layer to our group.
  Container::RefPtr vector ( _container->feature ( VECTOR_CONTAINER )->asContainer() );
  if ( vector.valid() )
  {
    vector->remove ( feature );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the elevation at a lat, lon (IElevationDatabase).
//
///////////////////////////////////////////////////////////////////////////////

double Body::elevationAtLatLong ( double lat, double lon ) const
{
  return this->elevation ( lat, lon );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag that says to allow spliting.
//
///////////////////////////////////////////////////////////////////////////////

void Body::allowSplit ( bool b )
{
  Guard guard ( this );
  _allowSplit = b;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the flag that says to allow spliting.
//
///////////////////////////////////////////////////////////////////////////////

bool Body::allowSplit() const
{
  Guard guard ( this );
  return _allowSplit;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the needs redraw state.
//
///////////////////////////////////////////////////////////////////////////////

void Body::needsRedraw ( bool b )
{
  Guard guard ( this );
  _needsRedraw = b;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the needs redraw state.
//
///////////////////////////////////////////////////////////////////////////////

bool Body::needsRedraw() const
{
  Guard guard ( this );
  return _needsRedraw;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag that says to keep detail.
//
///////////////////////////////////////////////////////////////////////////////

void Body::keepDetail ( bool b )
{
  Guard guard ( this );
  _keepDetail = b;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the flag that says to keep detail.
//
///////////////////////////////////////////////////////////////////////////////

bool Body::keepDetail() const
{
  Guard guard ( this );
  return _keepDetail;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the log.
//
///////////////////////////////////////////////////////////////////////////////

void Body::logSet ( LogPtr lp )
{
  Guard guard ( this );

  _log = lp;

#if 0
TODO: Visitor to set the log.
  if ( true == _rasters.valid() )
    _rasters->logSet ( lp );

  if ( true == _elevation.valid() )
    _elevation->logSet ( lp );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the log.
//
///////////////////////////////////////////////////////////////////////////////

Body::LogPtr Body::logGet()
{
  Guard guard ( this );
  return LogPtr ( _log );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the land model.
//
///////////////////////////////////////////////////////////////////////////////

LandModel* Body::landModel() const
{
  Guard guard ( this );
  return _landModel.get();
}



///////////////////////////////////////////////////////////////////////////////
//
//  Set the alpha value.
//
///////////////////////////////////////////////////////////////////////////////

void Body::alpha ( float a )
{
  Usul::Threads::Safe::set ( this->mutex(), a, _alpha );
  this->updateTilesAlpha();
}


///////////////////////////////////////////////////////////////////////////////
//
// Get the alpha value. 
//
///////////////////////////////////////////////////////////////////////////////

float Body::alpha() const
{
  Guard guard ( this->mutex() );
  return _alpha;
}


///////////////////////////////////////////////////////////////////////////////
//
// Return overall extents.
//
///////////////////////////////////////////////////////////////////////////////

Extents Body::extents() const
{
  Extents e ( 0, 0, 0, 0 );
  Tiles tiles ( Usul::Threads::Safe::get ( this->mutex(), _topTiles ) );
  for ( Tiles::const_iterator i = tiles.begin(); i != tiles.end(); ++i )
  {
    Tile::RefPtr tile ( *i );
    if ( true == tile.valid() )
    {
      e.expand ( tile->extents() );
    }
  }
  return e;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Intersect only with the tiles (no vector data).
//
///////////////////////////////////////////////////////////////////////////////

bool Body::intersectWithTiles ( const Usul::Math::Vec3d& point0, const Usul::Math::Vec3d& point1, Usul::Math::Vec3d& point )
{
  Tiles tiles ( Usul::Threads::Safe::get ( this->mutex(), _topTiles ) );

  // Make a temporary group to hold the tiles.
  osg::ref_ptr<osg::Group> group ( new osg::Group );
  for ( Tiles::iterator iter = tiles.begin(); iter != tiles.end(); ++iter )
  {
    group->addChild ( (*iter).get() );
  }
  
  // Points to intersect with.
  osg::Vec3d pt0 ( Usul::Convert::Type<Usul::Math::Vec3d,osg::Vec3d>::convert ( point0 ) );
  osg::Vec3d pt1 ( Usul::Convert::Type<Usul::Math::Vec3d,osg::Vec3d>::convert ( point1 ) );
  
  // Make the intersector.
  typedef osgUtil::LineSegmentIntersector Intersector;
  osg::ref_ptr<Intersector> intersector ( new Intersector ( pt0, pt1 ) );
  
  // Declare the pick-visitor.
  typedef osgUtil::IntersectionVisitor Visitor;
  osg::ref_ptr<Visitor> visitor ( new Visitor );
  visitor->setIntersector ( intersector.get() );
  
  // Intersect the scene.
  group->accept ( *visitor );
  
  // Get the hit-list for our line-segment.
  typedef osgUtil::LineSegmentIntersector::Intersections Intersections;
  const Intersections &hits = intersector->getIntersections();
  if ( hits.empty() )
    return false;
  
  // Set the hit.
  osg::Vec3d hit ( intersector->getFirstIntersection().getWorldIntersectPoint() );
  point = Usul::Convert::Type<osg::Vec3d,Usul::Math::Vec3d>::convert ( hit );
  
  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the max degree of anisotropic filtering.
//  See http://en.wikipedia.org/wiki/Anisotropic_filtering
//
///////////////////////////////////////////////////////////////////////////////

float Body::maxAnisotropy() const
{
  Guard guard ( this->mutex() );
  return _maxAnisotropy;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the container.
//
///////////////////////////////////////////////////////////////////////////////

Body::Container::RefPtr Body::container() const
{
  Guard guard ( this );
  return _container;
}
