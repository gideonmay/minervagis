
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
//  A recursive tile.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include "Minerva/Core/ElevationData.h"
#include "Minerva/Core/TileEngine/Tile.h"
#include "Minerva/Core/TileEngine/Body.h"
#include "Minerva/Core/Data/Container.h"
#include "Minerva/Core/Jobs/BuildElevation.h"
#include "Minerva/Core/Jobs/BuildTiles.h"
#include "Minerva/Core/Algorithms/Composite.h"
#include "Minerva/Core/Algorithms/SubRegion.h"
#include "Minerva/Core/Algorithms/ResampleElevation.h"
#include "Minerva/Core/Visitors/FindRasterLayers.h"
#include "Minerva/OsgTools/StateSet.h"
#include "Minerva/OsgTools/Group.h"
#include "Minerva/Common/ITileVectorJob.h"

#include "Usul/Bits/Bits.h"
#include "Usul/Errors/Assert.h"
#include "Usul/Functions/Execute.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Math/MinMax.h"
#include "Usul/Math/NaN.h"
#include "Usul/Predicates/CloseFloat.h"
#include "Usul/Threads/Safe.h"
#include "Usul/Jobs/Manager.h"

#include "osgUtil/CullVisitor"

#include "osg/Material"
#include "osg/Texture2D"

#include "osgDB/ReadFile"

#include "boost/bind.hpp"

#include <algorithm>
#include <limits>

using namespace Minerva::Core::TileEngine;


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to remove job from queue and cancel it.
//  Make sure needed mutexes are locked before calling.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  inline void removeAndCancelJob ( Body* body, Usul::Jobs::Job::RefPtr& job )
  {
    if ( true == job.valid() )
    {
      // Remove the job from the queue.
      if ( 0x0 != body && 0x0 != body->jobManager() )
        body->jobManager()->removeQueuedJob ( job );
      
      // Cancel the job and set to null.
      job->cancel();
      job = 0x0;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
///////////////////////////////////////////////////////////////////////////////

Tile::Tile ( TileKey::RefPtr info, double splitDistance, 
             Body *body, osg::Image* image, ElevationDataPtr elevation,
             TileVectorData::RefPtr tileVectorData ) : 
  BaseClass(),
  _mutex ( new Tile::Mutex ),
  _body ( body ),
  _info ( info ),
  _splitDistance ( splitDistance ),
  _mesh ( MeshPtr ( static_cast<Mesh*> ( 0x0 ) ) ),
  _flags ( Tile::ALL ),
  _children ( 4 ),
  _image ( image ),
  _elevation ( elevation ),
  _texture ( new osg::Texture2D ),
  _imageJob ( 0x0 ),
  _elevationJob ( 0x0 ),
  _tileJob ( 0x0 ),
  _boundingSphere(),
  _vector ( new osg::Group ),
  _tileVectorData ( tileVectorData, true ),
  _tileVectorJobs(),
  _childrenNeedCleared ( false )
{
  // We want thread safe ref and unref.
  this->setThreadSafeRefUnref ( true );

  // Set the image.
  if ( true == _image.valid() )
    _texture->setImage ( _image.get() );

  // Set filter parameters.
  _texture->setFilter ( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
  _texture->setFilter ( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );

  _texture->setMaxAnisotropy ( ( 0x0 == _body ) ? _texture->getMaxAnisotropy() : _body->maxAnisotropy() );
  _texture->setUseHardwareMipMapGeneration ( true );

  // Set texture coordinate wrapping parameters.
  _texture->setWrap ( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
  _texture->setWrap ( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
  
  _texture->setBorderWidth ( 0.0 );
  _texture->setBorderColor ( osg::Vec4 ( 0.0, 0.0, 0.0, 0.0 ) );

  // For some reason this is now needed or else we cannot see the images.
  OsgTools::State::StateSet::setMaterialDefault ( this );

  // This makes sure that the update visitor is always called.
  // This isn't optimal, but it fixes multi-threading issues.
  this->setNumChildrenRequiringUpdateTraversal ( this->getNumChildrenRequiringUpdateTraversal() + 1 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
///////////////////////////////////////////////////////////////////////////////

Tile::Tile ( const Tile &tile, const osg::CopyOp &option ) : 
  BaseClass ( tile, option ),
  _mutex ( new Tile::Mutex ),
  _body ( tile._body ),
  _info ( tile._info ),
  _splitDistance ( tile._splitDistance ),
  _mesh ( MeshPtr ( static_cast<Mesh*> ( 0x0 ) ) ),
  _flags ( Tile::ALL ),
  _children ( tile._children ),
  _image ( tile._image ),
  _elevation ( tile._elevation ),
  _imageJob ( 0x0 ),
  _elevationJob ( 0x0 ),
  _tileJob ( 0x0 ),
  _boundingSphere ( tile._boundingSphere ),
  _vector ( new osg::Group ),
  _tileVectorData ( 0x0, true ),
  _tileVectorJobs(),
  _childrenNeedCleared ( tile._childrenNeedCleared )
{

  // Remove if you are ready to test copying. Right now, I'm not sure what 
  // it means. This constructor is here to satisfy the META_Node macro.
  USUL_ASSERT ( false );

  // Handle release build too.
  throw std::runtime_error ( "Error 4113256974: Do not call Tile's copy constructor" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor
//
///////////////////////////////////////////////////////////////////////////////

Tile::~Tile()
{
  Usul::Functions::safeCall ( boost::bind ( &Tile::_destroy, this ), "2021499927" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destroy
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_destroy()
{
  // Clear this tile and all it's children.
  this->clear ( true );

  // At the moment this is redundant but safe.
  this->_cancelTileVectorJobs();
  this->_perTileVectorDataDelete();

  // Don't delete!
  _body = 0x0;

  // Reset the boost shared pointer.
  _mesh = MeshPtr ( static_cast < Mesh * > ( 0x0 ) );

  // Done with the mutex.
  delete _mutex; _mutex = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query the interfaces
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown *Tile::queryInterface ( unsigned long iid )
{
  switch ( iid )
  {
  case Usul::Interfaces::IUnknown::IID:
  case Minerva::Common::ITile::IID:
    return static_cast < Minerva::Common::ITile * > ( this );
  default:
    return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the mesh.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::updateMesh()
{
  // If they are both not dirty then return.
  if ( false == this->verticesDirty() )
    return;

  // Get needed variables.
  Extents extents ( this->extents() );
  MeshSize size ( this->meshSize() );
  ElevationDataPtr elevation;
  Body::RefPtr body ( 0x0 );
  {
    Guard guard ( this->mutex() );
    elevation = _elevation;
    body = _body;
  }

  // Handle bad state.
  if ( 0x0 == body )
    return;

  // Depth of skirt.  TODO: This function needs to be tweeked.
  const double offset ( Usul::Math::maximum<double> ( ( 3500 - ( this->level() * 150 ) ), ( 10 * std::numeric_limits<double>::epsilon() ) ) );

  // Make a new mesh.
  MeshPtr mesh ( new Mesh ( size[0], size[1], offset, extents ) );

  // Make a new bounding sphere.
  BSphere boundingSphere;

  // Build the mesh.
  osg::ref_ptr<osg::Node> ground ( mesh->buildMesh ( *body, elevation, boundingSphere ) );

  // Unset these dirty flags.
  this->dirty ( false, Tile::VERTICES, false );

  // Set the ground's alpha.
  OsgTools::State::StateSet::setAlpha ( this, body->alpha() );

  osg::ref_ptr<osg::Group> group ( new osg::Group );
  group->addChild ( ground.get() );
  group->addChild ( Usul::Threads::Safe::get ( this->mutex(), _vector.get() ) );

  // This will add the root node of the container.
  {
    Guard guard ( this );
    group->addChild ( this->_perTileVectorDataGet()->getScene() );
  }

  // Add the low lod if there isn't currently one.
  if ( 0 == this->getNumChildren() )
  {
    this->addChild ( group.get() );
  }

  // Replace the low lod.
  else
  {
    this->setChild ( 0, group.get() );
  }

  // Set needed variables.
  {
    Guard guard ( this );
    _mesh = mesh;
    _boundingSphere = boundingSphere;
  }
  
  this->dirtyBound();
}


//////////////////////////////////////////////////////////////////////////////
//
//  Update the per-tile vector data.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::updateTileVectorData()
{
  typedef Minerva::Common::ITileVectorJob ITileVectorJob;

  // Copy our container of jobs.
  TileVectorJobs jobs ( Usul::Threads::Safe::get ( this->mutex(), _tileVectorJobs ) );

  // These are the ones we'll remove.
  TileVectorJobs removeMe;

  // Need to update.
  bool needToUpdate ( false );

  // Loop through the container of jobs.
  for ( TileVectorJobs::iterator i = jobs.begin(); i != jobs.end(); ++i )
  {
    // Get needed interface.
    ITileVectorJob::QueryPtr job ( *i );
    if ( true == job.valid() )
    {
      // See if the job is done.
      if ( true == job->isVectorJobDone() )
      {
        // We want to remove this job.
        removeMe.push_back ( *i );

        // Add the data to our container.
        ITileVectorJob::Data data;
        job->takeVectorData ( data );
        for ( ITileVectorJob::Data::iterator d = data.begin(); d != data.end(); ++d )
        {
          // The very first time we add new data we have to clear the 
          // existing data that we inherited from the parent.
          if ( true == this->_perTileVectorDataIsInherited() )
          {
            this->_perTileVectorDataClear();
          }

          // Add the new data.
          ITileVectorJob::Data::value_type layer ( *d );
          TileVectorData::RefPtr tileVectorData ( this->_perTileVectorDataGet() );
          Usul::Functions::safeCall ( boost::bind ( &TileVectorData::add, tileVectorData.get(), layer.get(), true ), "1356847360" );
          needToUpdate = true;
        }
      }
    }
    else
    {
      // Needed interface not available so purge.
      removeMe.push_back ( *i );
    }
  }

  // Remove these jobs.
  if ( false == removeMe.empty() )
  {
    Guard guard ( this );
    for ( TileVectorJobs::iterator j = removeMe.begin(); j != removeMe.end(); ++j )
    {
      Usul::Interfaces::IUnknown::RefPtr job ( *j );
      Usul::Functions::safeCall ( boost::bind ( &TileVectorJobs::remove, &_tileVectorJobs, job ), "1975135769" );
    }
  }

  // Update the borders.
  this->_updateShowBorders();

  // Update container if we need to.
  if ( true == needToUpdate )
  {
    // Get the body as an unknown pointer.
    // Note: I do not think Usul::Threads::Safe::get is safe in this case 
    // because it returns a pointer and we catch it with a smart-pointer. 
    // The _body member could be deferenced and deleted after assignment 
    // to the returned pointer but before catching it with the smart-pointer.
    Usul::Interfaces::IUnknown::QueryPtr body;
    Usul::Threads::Safe::set ( this->mutex(), _body, body );

    // Need to pass the body because it implements the necessary interfaces 
    // to convert the coordinates from lon-lat-elev to x-y-z.
    Minerva::Common::IElevationDatabase::QueryPtr elevation ( body );
    Minerva::Common::IPlanetCoordinates::QueryPtr planet ( body );
    
    this->_perTileVectorDataGet()->updateNotify ( 0x0, planet.get(), elevation.get() );
  }
}


//////////////////////////////////////////////////////////////////////////////
//
//  Update the texture.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::updateTexture()
{
  // Return now if we don't have an image.
  if ( false == this->textureDirty() )
    return;

  // Get needed variables.
  ImagePtr image ( 0x0 );
  osg::ref_ptr < osg::Texture2D > texture ( 0x0 );
  Body::RefPtr body ( 0x0 );
  {
    Guard guard ( this->mutex() );
    image = _image;
    texture = _texture;
    body = _body;
  }
  
  // Set the image.
  if ( ( true == texture.valid() ) )
  {
    // Set the image if not null.
    if ( true == image.valid() )
      texture->setImage ( image.get() );

    // Set the proper texture state. This enables us to get a blank planet.
    const unsigned int flags ( ( ( true == image.valid() ) ? osg::StateAttribute::ON : osg::StateAttribute::OFF ) | osg::StateAttribute::PROTECTED );

    // Get the state set.
    osg::ref_ptr< osg::StateSet > ss ( this->getOrCreateStateSet() );
    ss->setTextureAttributeAndModes ( 0, texture.get(), flags );

    // Turn lighting on or off, depending on if there is an image.
    OsgTools::State::StateSet::setLighting ( this, ( false == image.valid() ) );
    
    // Let the body know we have a new texture.
    if ( 0x0 != body )
    {
      body->needsRedraw ( true );
    }

    // Texture no longer dirty.
    this->dirty ( false, Tile::TEXTURE, false );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::update()
{
  if ( Usul::Threads::Safe::get ( this->mutex(), _childrenNeedCleared ) )
  {
    this->_clearChildren ( false, true );
  }
  
  // Get needed variables.
  unsigned int flags ( 0 );
  Usul::Jobs::Job::RefPtr imageJob ( 0x0 );
  Body::RefPtr body ( 0x0 );
  {
    Guard guard ( this );
    flags = _flags;
    imageJob = _imageJob;
    body = _body;
  }

  // If we have a job, see if it has finished.
  if ( imageJob.valid() )
  {
    if ( imageJob->isDone() )
    {
      // Capture the success state.
      const bool imageJobSuccess ( imageJob->success() );
      
      // Clear the image job.
      {
        Guard guard ( this );
        _imageJob = 0x0;
        imageJob = 0x0;
      }

      // If the job did not succeed, launch a new request.
      if ( false == imageJobSuccess )
        this->_launchImageRequest();
    }
  }

  // If we don't have a job, see if we need to create one.
  else
  {
    if ( Usul::Bits::has ( flags, Tile::IMAGE ) )
      this->_launchImageRequest();
  }

  // Make sure the mesh is updated.
  this->updateMesh();

  // Make sure our texture is updated.
  this->updateTexture();

  // Make sure the per-tile vector data is up-to-date.
  this->updateTileVectorData();

  // Add the tiles to the scene.
  this->_updateTiles();

  // See if we should draw skirts and borders.  Make sure these are called after update mesh.
  this->_setShowSkirts ( body->useSkirts() );
  this->_updateShowBorders();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Traverse the children.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::traverse ( osg::NodeVisitor &nv )
{
  if ( osg::NodeVisitor::UPDATE_VISITOR == nv.getVisitorType() )
    this->update();

  // If it's a cull visitor...
  if ( osg::NodeVisitor::CULL_VISITOR == nv.getVisitorType() )
  {
    // Get needed variables.
    unsigned int flags ( 0 );
    Usul::Jobs::Job::RefPtr imageJob ( 0x0 );
    Usul::Jobs::Job::RefPtr tileJob ( 0x0 );
    Body::RefPtr body ( 0x0 );
    {
      Guard guard ( this );
      flags = _flags;
      tileJob = _tileJob;
      body = _body;
    }
    
    // Get cull visitor.
    osgUtil::CullVisitor *cv ( dynamic_cast < osgUtil::CullVisitor * > ( &nv ) );

    // See what we are allowed to do.
    const bool allowSplit ( body->allowSplit() );
    const bool keepDetail ( body->keepDetail() );

    // Do we have a high lod?
    const bool hasDetail ( this->getNumChildren() == 2 );

    // Is this the default behavoir?
    const bool defaultMode ( true == allowSplit && false == keepDetail );

    // Do we need to check if we need more detail?
    const bool checkDetail ( true == allowSplit && true == keepDetail && false == hasDetail );

    // Check if we can split.  Don't freeze if we are waiting for a job.
    const bool splitIfNeeded ( ( false == tileJob.valid() && this->getNumChildren() == 0 ) || true == defaultMode || true == checkDetail );

    // Check to see if we are culled.
    if ( 0x0 == cv || cv->isCulled ( *this ) )
    {
      // We are culled, so cancel these jobs.
      this->_cancelTileVectorJobs();
      this->_cancelTileJob();

      // Do not clear children if we are suppose to keep detail.
      if ( false == keepDetail )
      {
        // Clear our children.
        Usul::Threads::Safe::set ( this->mutex(), true, _childrenNeedCleared );
      }

      // Do not traverse further.
      return;
    }

    if ( false == splitIfNeeded )
    {
      const unsigned int child ( ( false == allowSplit && false == keepDetail ) ? 0 : this->getNumChildren() - 1 );
      this->getChild ( child )->accept ( *cv );
    }

    // Spilt.
    else
    {
      this->_cull ( *cv );
    }
  }

  // Not a cull visitor...
  else
  {
    if ( this->getNumChildren() > 0 )
    {
      this->getChild ( this->getNumChildren() - 1 )->accept ( nv );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Cull traversal.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_cull ( osgUtil::CullVisitor &cv )
{
  // Get needed variables.
  MeshPtr mesh_;
  Usul::Jobs::Job::RefPtr tileJob ( 0x0 );
  Body::RefPtr body ( 0x0 );
  double splitDistance ( 0 );
  {
    Guard guard ( this );
    mesh_ = _mesh;
    tileJob = _tileJob;
    body = _body;
    splitDistance = _splitDistance;
  }

  // Handle bad state.
  if ( ( 0x0 == mesh_ ) || 
       ( mesh_->rows() < 2 ) || 
       ( mesh_->columns() < 2 ) ||
       ( 0x0 == body ) ||
       ( 0x0 == body->jobManager() ) )
  {
    return;
  }

  // Four corners and center of the tile.
  Mesh &mesh ( *mesh_ );
  const osg::Vec3f &eye ( cv.getViewPointLocal() );

  // Check with smallest distance.
  const double dist ( mesh.getSmallestDistanceSquared ( eye ) );
  const bool farAway ( ( dist > ( splitDistance * splitDistance ) ) );
  const unsigned int numChildren ( this->getNumChildren() );
  USUL_ASSERT ( numChildren > 0 );

  // Check for nan in eye values (it's happened).
  const bool eyeIsNan ( Usul::Math::nan ( eye[0] ) || Usul::Math::nan ( eye[1] ) || Usul::Math::nan ( eye[2] ) );

  // Check if we've gone too deep.
  const bool tooDeep ( this->level() >= body->maxLevel() );

  // Should we traverse the low lod?
  bool low ( farAway || eyeIsNan || tooDeep );

  // Finally, ask the callback.
  low = !( body->shouldSplit ( !low, this ) );
  
  if ( low )
  {
    // Remove high level of detail.
    if ( numChildren > 1 )
    {
      // Clear all the children.
      Usul::Threads::Safe::set ( this->mutex(), true, _childrenNeedCleared );
    }

    // We no longer need to make the high level of detail.
    this->_cancelTileJob();
  }

  else
  {
    // Add high level if necessary.
    if ( 1 == numChildren )
    {
      bool hasChildren ( false );
      {
        Guard guard ( this->mutex() );
        hasChildren = ( _children[TileKey::LOWER_LEFT].valid()  ) &&
                      ( _children[TileKey::LOWER_RIGHT].valid() ) &&
                      ( _children[TileKey::UPPER_LEFT].valid()  ) &&
                      ( _children[TileKey::UPPER_RIGHT].valid() );
      }

      // Make tiles if we are not caching them, or if this is the first time.
      if ( false == hasChildren )
      {
        // Use the low lod while we are waiting for the job.
        if ( tileJob.valid() )
        {
          low = true;
        }
        else
        {
          Guard guard ( this->mutex() );

          // Make a new job to tile the child tiles.
          _tileJob = new Minerva::Core::Jobs::BuildTiles ( Tile::RefPtr ( this ) );
        
          // Add the job to the job manager.
          _body->jobManager()->addJob ( _tileJob.get() );
        }
      }
    }
  }

  // Traverse low level of detail.
  if ( low || tileJob.valid() )
  {
    this->getChild ( 0 )->accept ( cv );
  }

  // Traverse last child.
  else
  {
    const unsigned int last ( this->getNumChildren() - 1 );
    this->getChild ( last )->accept ( cv );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the tiles.  See if we need to add the high lod.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_updateTiles()
{
  Usul::Jobs::Job::RefPtr tileJob ( 0x0 );
  {
    Guard guard ( this );
    tileJob = _tileJob;
  }

  if ( tileJob.valid() && tileJob->isDone() )
  {
    // Did it work?
    if ( true == tileJob->success() )
    {
      Guard guard ( this->mutex() );

      // Add the children.
      osg::ref_ptr<osg::Group> group ( new osg::Group );
      group->addChild ( _children[TileKey::LOWER_LEFT]  );
      group->addChild ( _children[TileKey::LOWER_RIGHT] );
      group->addChild ( _children[TileKey::UPPER_LEFT]  );
      group->addChild ( _children[TileKey::UPPER_RIGHT] );
      this->addChild ( group.get() );
    }

    // It did not work.
    else
    {
      // Some of the children may have been created, so clear them all.
      // Do not cancel the job since it already finished.
      this->_clearChildren ( false, false );
    }

    {
      Guard guard ( this->mutex() );
      _tileJob = 0x0;
      tileJob = 0x0;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Split the tile.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::split ( Usul::Jobs::Job::RefPtr job )
{
  Body::RefPtr body ( Usul::Threads::Safe::get ( this->mutex(), _body ) );
  
  // Handle no body.
  if ( false == body.valid() )
    return;
  
  const double half ( this->splitDistance() * 0.5 );

  TileKey::ChildrenKeys keys;
  _info->split ( keys );

  Tile::RefPtr t0 ( this->_buildTile ( keys[TileKey::LOWER_LEFT], half, job ) ); // lower left  tile
  Tile::RefPtr t1 ( this->_buildTile ( keys[TileKey::LOWER_RIGHT], half, job ) ); // lower right tile
  Tile::RefPtr t2 ( this->_buildTile ( keys[TileKey::UPPER_LEFT], half, job ) ); // upper left  tile
  Tile::RefPtr t3 ( this->_buildTile ( keys[TileKey::UPPER_RIGHT], half, job ) ); // upper right tile
  
  // Have we been cancelled?
  if ( job.valid() && true == job->canceled() )
    job->cancel();
  
  // Need to notify vector data so it can re-adjust.
  Minerva::Core::Data::Container::RefPtr vector ( body->vectorData() );
  if ( vector.valid() )
  {
    Usul::Interfaces::IUnknown::QueryPtr unknown ( body );

    // Notify that the elevation has changed.
    vector->elevationChangedNotify ( t0->extents(), t0->level(), t0->elevationData(), unknown.get() );
    vector->elevationChangedNotify ( t1->extents(), t1->level(), t1->elevationData(), unknown.get() );
    vector->elevationChangedNotify ( t2->extents(), t2->level(), t2->elevationData(), unknown.get() );
    vector->elevationChangedNotify ( t3->extents(), t3->level(), t3->elevationData(), unknown.get() );
  }
  
  {
    Guard guard ( this->mutex() );
    _children[TileKey::LOWER_LEFT]  = t0.get();
    _children[TileKey::LOWER_RIGHT] = t1.get();
    _children[TileKey::UPPER_LEFT]  = t2.get();
    _children[TileKey::UPPER_RIGHT] = t3.get();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build a tile.
//
///////////////////////////////////////////////////////////////////////////////

Tile::RefPtr Tile::_buildTile ( TileKey::RefPtr info,
                                double splitDistance, 
                                Usul::Jobs::Job::RefPtr job )
{
  // If our logic is correct, this should be true.
  USUL_ASSERT ( this->referenceCount() >= 1 );
  USUL_ASSERT ( info.valid() );

  // Have we been cancelled?
  if ( job.valid() && true == job->canceled() )
    job->cancel();
  
  Extents extents ( info->extents() );

  // Get this tile's vector data that falls within the extents.
  TileVectorData::RefPtr tvd ( new TileVectorData );
  tvd->add ( this->_perTileVectorDataGet()->getItemsWithinExtents ( extents.minLon(), extents.minLat(), extents.maxLon(), extents.maxLat() ) );

  // Make the tile.
  Body::RefPtr body ( Usul::Threads::Safe::get ( this->mutex(), _body ) );
  
  Tile::RefPtr tile ( new Tile ( info, splitDistance, body.get(), 0x0, static_cast<Minerva::Common::IElevationData*> ( 0x0 ), tvd.get() ) );

  // Tell the vector data to update now. Otherwise, when the new tile draws it 
  // will still be blank until the first vector job finishes.
  Minerva::Common::IElevationDatabase::QueryPtr elevation ( body );
  Minerva::Common::IPlanetCoordinates::QueryPtr planet ( body );
  
  tvd->updateNotify ( 0x0, planet.get(), elevation.get() );

  // Tell the tile to start building its elevation data.
  tile->buildElevationData ( job );

  // Use a quarter of the parent's elevation for the child.
  if ( false == tile->elevationData().valid() )
  {
    ElevationDataPtr parentElevation ( Usul::Threads::Safe::get ( this->mutex(), _elevation ) );
    if ( parentElevation.valid() )
    {
      tile->elevationData ( Minerva::Core::Algorithms::resampleElevation ( Tile::RefPtr ( this ), extents ) );
    }
  }

  // Have we been cancelled?
  if ( job.valid() && true == job->canceled() )
    job->cancel();

  // Build the raster.  Make sure this is done before mesh is built and texture updated.
  tile->buildRaster ( job );

#if 0
  //TODO: Make a copy of the sub image.
  
  // Check to see if the tile has a valid image.
  if ( false == tile->image().valid() && 0x0 != this->image() )
  {
    // Use the specified region of our image.
    tile->textureData ( this->image().get(), region );
  }
#endif

  // Have we been cancelled?
  if ( job.valid() && true == job->canceled() )
    job->cancel();

  tile->updateMesh();
  tile->updateTexture();

  // Have we been cancelled?
  if ( job.valid() && true == job->canceled() )
    job->cancel();

  // Now build the per-tile vector data.
  tile->buildPerTileVectorData ( job );

  // Have we been cancelled?
  if ( job.valid() && true == job->canceled() )
    job->cancel();

  return tile;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build raster.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::buildRaster ( Usul::Jobs::Job::RefPtr job )
{
  // Get the body.
  Body::RefPtr body ( Usul::Threads::Safe::get ( this->mutex(), _body ) );
  
  // Handle no body.
  if ( false == body.valid() )
    return;

  // Width and height for the image.
  const ImageSize imageSize ( _info->imageSize() );
  const unsigned int width ( imageSize[0] );
  const unsigned int height ( imageSize[1] );

  // Get the rasters.
  typedef Minerva::Core::Visitors::FindRasterLayers Visitor;
  typedef Visitor::RasterLayers Rasters;
  typedef Minerva::Core::Layers::RasterLayer RasterLayer;
  
  Rasters rasters;
  Visitor::RefPtr visitor ( new Visitor ( this->extents(), rasters ) );
  body->rasterData()->accept ( *visitor );

  // Image.
  osg::ref_ptr<osg::Image> result ( 0x0 );

  // Build the list of images to be composited.
  for ( Rasters::iterator iter = rasters.begin(); iter != rasters.end(); ++iter )
  {
    // Have we been cancelled?
    if ( ( 0x0 != job ) && ( true == job->canceled() ) )
      job->cancel();

    // The layer.
    RasterLayer::RefPtr raster ( *iter );
    if ( raster.valid() )
    {
      // Image for the layer.
      osg::ref_ptr<osg::Image> image ( 0x0 );

      Extents e ( raster->extents() );

      // Should the layer be shown?
      const bool shown ( raster->visibility() );
      const bool isLevelRange ( raster->isInLevelRange ( this->level() ) );

      // Only use this layer if it's shown and intersects our extents.
      if ( shown && this->extents().intersects ( e ) && true == isLevelRange )
      {
        // Get the image for the layer.
        image = raster->texture ( *_info, width, height, job, 0x0 );
      }

      // Composite if it's valid...
      if ( true == image.valid() )
      {
        // Is this the first image?
        if ( false == result.valid() )
        {
          // We always make an image and composite to handle formats other than GL_RGBA.
          result = new osg::Image;
          result->allocateImage ( width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE );
          ::memset ( result->data(), 0, result->getImageSizeInBytes() );
        }

        // Copy the alphas.
        typedef RasterLayer::Alphas Alphas;
        Alphas alphas ( raster->alphas() );
        float alpha ( raster->alpha() );
        
        // Composite.
        Minerva::Core::Algorithms::Composite::raster ( *result, *image, alphas, alpha );
      }
    }
  }

  // Always set our image, even if it's null.
  this->textureData ( result.get() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build per-tile vector data.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::buildPerTileVectorData ( Usul::Jobs::Job::RefPtr job )
{
  // Get the body.
  Body::RefPtr body ( Usul::Threads::Safe::get ( this->mutex(), _body ) );
  if ( false == body.valid() )
    return;

  // Get the needed interface.
  Body::Container::RefPtr vectorData ( body->vectorData() );
  if ( false == vectorData.valid() )
    return;

  // Need the extents.
  Extents e ( this->extents() );

  // Ask for the container of jobs that we later poll.
  TileVectorJobs tileVectorJobs ( vectorData->launchVectorJobs ( 
    e.minLon(), e.minLat(), e.maxLon(), e.maxLat(), this->level(), body->jobManager(), 
    Usul::Interfaces::IUnknown::QueryPtr ( body ) ) );

  // Have we been cancelled?
  if ( ( 0x0 != job ) && ( true == job->canceled() ) )
    job->cancel();

  // Purge any jobs that are null.
  tileVectorJobs.remove_if ( std::bind2nd ( std::equal_to<TileVectorJobs::value_type>(), TileVectorJobs::value_type ( 0x0 ) ) );

  // Save the jobs.
  Usul::Threads::Safe::set ( this->mutex(), tileVectorJobs, _tileVectorJobs );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the level. Zero is the top.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int Tile::level() const
{
  return _info->level();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the mutex.
//
///////////////////////////////////////////////////////////////////////////////

Tile::Mutex &Tile::mutex() const
{
  return *_mutex;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag that says we're dirty.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::dirty ( bool state, unsigned int flags, bool dirtyChildren )
{
  this->_setDirtyAlways ( state, flags, dirtyChildren );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Mark the dirty state, only if we cross this extents.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::dirty ( bool state, unsigned int flags, bool dirtyChildren, const Extents& extents )
{
  this->_setDirtyIfIntersect ( state, flags, dirtyChildren, extents );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Tell the body it can delete us.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_deleteMe()
{
  Guard guard ( this );

  // Add this tile to the body.
  if ( 0x0 != _body )
  {
    _body->_addTileToBeDeleted ( this );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag that says we're dirty.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_setDirtyAlways ( bool state, unsigned int flags, bool dirtyChildren )
{
  Guard guard ( this );

  _flags = Usul::Bits::set ( _flags, flags, state );

  if ( dirtyChildren )
  {
    Usul::Functions::executeMemberFunctions ( _children, &Tile::_setDirtyAlways, state, flags, dirtyChildren );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Mark the dirty state, only if we cross this extents.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_setDirtyIfIntersect ( bool state, unsigned int flags, bool dirtyChildren, const Extents& extents )
{
  Guard guard ( this );

  if ( extents.intersects ( _info->extents() ) )
  {
    // Set our dirty state.
    this->dirty ( state, flags, false );

    // Visit our children.
    if ( dirtyChildren )
    {
      Usul::Functions::executeMemberFunctions ( _children, &Tile::_setDirtyIfIntersect, state, flags, dirtyChildren, extents );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Are the vertices dirty?
//
///////////////////////////////////////////////////////////////////////////////

bool Tile::verticesDirty() const
{
  Guard guard ( this );
  return Usul::Bits::has ( _flags, Tile::VERTICES );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the texture dirty?
//
///////////////////////////////////////////////////////////////////////////////

bool Tile::textureDirty() const
{
  Guard guard ( this );
  return Usul::Bits::has ( _flags, Tile::TEXTURE );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear the scene.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::clear ( bool children )
{
  // Clear children first if we should.
  if ( true == children )
  {
    this->_clearChildren ( children, true );
  }

  // Cancel jobs.
  {
    Guard guard ( this );
    Helper::removeAndCancelJob ( _body, _imageJob );
    Helper::removeAndCancelJob ( _body, _tileJob );
  }

  // Set the body to null. We have to do this because the tiles 
  // in jobs may live longer than the body.
  {
    Guard guard ( this );
    _body = 0x0;
  }

  // Delete the per-tile vector data and cancel the jobs.
  this->_cancelTileVectorJobs();
  this->_perTileVectorDataDelete();

  // Set dirty flags.
  this->dirty ( false, Tile::ALL, false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extents.
//
///////////////////////////////////////////////////////////////////////////////

Extents Tile::extents() const
{
  return _info->extents();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Load the image.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_launchImageRequest()
{
  Guard guard ( this );

  // Cancel the one we have, if any.
  Helper::removeAndCancelJob ( _body, _imageJob );
  
  // Start the request to pull in texture.
  if ( 0x0 != _body )
  {
    _imageJob = _body->textureRequest ( this );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the image.
//
///////////////////////////////////////////////////////////////////////////////

Tile::ImagePtr Tile::image()
{
  Guard guard ( this );
  return _image;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the texture data.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::textureData ( osg::Image* image )
{
  // Set image atomiclly.
  {
    Guard guard ( this );
    
    // Set our image.
    _image = image;
    
    // Our image is no longer dirty.
    this->dirty ( false, Tile::IMAGE, false );
    
    // Our texture needs to be updated.
    this->dirty ( true, Tile::TEXTURE, false );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the size.
//
///////////////////////////////////////////////////////////////////////////////

MeshSize Tile::meshSize() const
{
  return _info->meshSize();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the split distance.
//
///////////////////////////////////////////////////////////////////////////////

double Tile::splitDistance() const
{
  Guard guard ( this );
  return _splitDistance;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the bounding sphere.
//
///////////////////////////////////////////////////////////////////////////////

osg::BoundingSphere Tile::computeBound() const
{
  Guard guard ( this );
  return _boundingSphere;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear children.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_clearChildren ( bool traverse, bool cancelJob )
{
  Guard guard ( this );

  this->removeChild ( 1, this->getNumChildren() - 1 );

  // Clear all the children.
  Usul::Functions::executeMemberFunctions ( _children, &Tile::clear, traverse );

  // Mark the children as ready to be deleted.
  Usul::Functions::executeMemberFunctions ( _children, &Tile::_deleteMe );

  // Assign children to null.
  _children.assign ( _children.size(), 0x0 );

  // Clear the tile job.
  if ( true == cancelJob )
  {
    // Without this call to cancel we always have to wait for the job to 
    // finish, even if we've already chosen to go down the low path, which 
    // happens when we are zoomed in and then "view all".
    Helper::removeAndCancelJob ( _body, _tileJob );
    
    // Clear the per-tile vector data jobs.
    this->_cancelTileVectorJobs();
  }
  
  _childrenNeedCleared = false;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear all the jobs.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_cancelTileJob()
{
  Guard guard ( this );

  Helper::removeAndCancelJob ( _body, _tileJob );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the split distance.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::splitDistance ( double distance, bool children )
{
  Guard guard ( this );
  _splitDistance = distance;
  
  // Set the children's split distance if we should
  if ( children )
  {
    const double childDistance ( distance / 2.0 );
    if ( _children[0].valid() ) _children[0]->splitDistance ( childDistance, children );
    if ( _children[1].valid() ) _children[1]->splitDistance ( childDistance, children );
    if ( _children[2].valid() ) _children[2]->splitDistance ( childDistance, children );
    if ( _children[3].valid() ) _children[3]->splitDistance ( childDistance, children );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is this tile a leaf?
//
///////////////////////////////////////////////////////////////////////////////

bool Tile::isLeaf() const
{
  Guard guard ( this->mutex() );
  return !( _children[0].valid() && _children[1].valid() && _children[2].valid() && _children[3].valid() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the child at index i.
//
///////////////////////////////////////////////////////////////////////////////

Tile::RefPtr Tile::childAt ( unsigned int i ) const
{
  Guard guard ( this->mutex() );
  return ( i < _children.size() ? _children[i] : 0x0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the elevation data.
//
///////////////////////////////////////////////////////////////////////////////

Tile::ElevationDataPtr Tile::elevationData() const
{
  Guard guard ( this->mutex() );
  return _elevation;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the elevation data.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::elevationData ( ElevationDataPtr data )
{
  Guard guard ( this->mutex() );
  _elevation = data;
  this->dirty ( true, Tile::VERTICES, false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the overall alpha.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::updateAlpha()
{
  Guard guard ( this );
  if ( 0x0 != _body )
  {
    OsgTools::State::StateSet::setAlpha ( this, _body->alpha() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add vector data.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::addVectorData ( osg::Node* node )
{
  if ( 0x0 != node )
  {
    Guard guard ( this );
    _vector->addChild ( node );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove vector data.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::removeVectorData ( osg::Node* node )
{
  if ( 0x0 != node )
  {
    Guard guard ( this );
    _vector->removeChild ( node );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convience function that re-directs to the body.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::latLonHeightToXYZ ( double lat, double lon, double elevation, osg::Vec3d& point ) const
{
  Body* body ( Usul::Threads::Safe::get ( this->mutex(), _body ) );

  if ( 0x0 != body )
  {
    body->latLonHeightToXYZ ( lat, lon, elevation, point );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the job manager.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Jobs::Manager *Tile::jobManager()
{
  Guard guard ( this );
  return ( ( 0x0 == _body ) ? 0x0 : _body->jobManager() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the border state.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_setShowBorders ( bool show )
{
  Guard guard ( this );

  if ( 0x0 != _mesh.get() )
  {
    _mesh->showBorder ( show );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the border state.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_updateShowBorders()
{
  Guard guard ( this );

  const bool hasTileVectorJobs ( false == _tileVectorJobs.empty() );
  const bool hasTileJob ( true == _tileJob.valid() );
  const bool hasImageJob ( true == _imageJob.valid() );
  const bool isBusy ( hasTileVectorJobs || hasTileJob || hasImageJob );
  const bool allowedToShow ( ( 0x0 == _body ) ? true : _body->useBorders() );

  this->_setShowBorders ( allowedToShow && isBusy );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the show skirt state.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_setShowSkirts ( bool show )
{
  Guard guard ( this->mutex() );
  if ( 0x0 != _mesh.get() )
  {
    _mesh->showSkirts ( show );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the per-tile vector data. Create it if needed.
//
///////////////////////////////////////////////////////////////////////////////

Tile::TileVectorData::RefPtr Tile::_perTileVectorDataGet()
{
  Guard guard ( this );
  if ( false == _tileVectorData.first.valid() )
  {
    _tileVectorData = TileVectorDataPair ( TileVectorData::RefPtr ( new TileVectorData ), false );
  }
  return _tileVectorData.first;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear the per-tile vector data.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_perTileVectorDataClear()
{
  Guard guard ( this );
  if ( true == _tileVectorData.first.valid() )
  {
    _tileVectorData.first->clear();
  }
  _tileVectorData.second = false;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Delete the per-tile vector data.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_perTileVectorDataDelete()
{
  Guard guard ( this );
  if ( true == _tileVectorData.first.valid() )
  {
    _tileVectorData.first = TileVectorData::RefPtr ( 0x0 );
  }
  _tileVectorData.second = false;
}


///////////////////////////////////////////////////////////////////////////////
//
//  See if the data is inherited.
//
///////////////////////////////////////////////////////////////////////////////

bool Tile::_perTileVectorDataIsInherited() const
{
  Guard guard ( this );
  return ( true == _tileVectorData.second );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Cancel the per-tile vector data jobs.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::_cancelTileVectorJobs()
{
  typedef Minerva::Common::ITileVectorJob ITileVectorJob;

  // Get the jobs and clear our member at the same time.
  TileVectorJobs jobs;
  {
    Guard guard ( this );
    jobs.swap ( _tileVectorJobs );
  }

  // Loop through jobs.
  for ( TileVectorJobs::iterator i = jobs.begin(); i != jobs.end(); ++i )
  {
    // Get needed interface.
    ITileVectorJob::QueryPtr job ( *i );
    if ( true == job.valid() )
    {
      job->cancelVectorJob();
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the elevation at lat, lon.
//
///////////////////////////////////////////////////////////////////////////////

double Tile::elevation ( double lat, double lon )
{
  // Get the body and mesh.
  Body::RefPtr body;
  MeshPtr mesh;
  {
    Guard guard ( this );
    body = _body;
    mesh = _mesh;
  }

  LandModel::RefPtr land ( body.valid() ? body->landModel() : 0x0 );

  // Return zero if no land model, or no mesh.
  if ( false == land.valid() ||  0x0 == mesh.get())
    return 0.0;

  return mesh->elevation ( lat, lon, *land );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get this tile.
//
//  Note: do not need to guard here because we're returning the pointer to 
//  this object. The only thing to watch out for is this tile getting deleted 
//  in another thread while this function is being executed. However, the 
//  likely scenario is the caller already has an IUnknown::RefPtr to this 
//  object when they query for ITile, so this object should not getd deleted.
//
///////////////////////////////////////////////////////////////////////////////

Tile *Tile::tile()
{
  return this;
}


///////////////////////////////////////////////////////////////////////////////
//
//  IUnknown glue.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::ref()
{
  BaseClass::ref();
}


///////////////////////////////////////////////////////////////////////////////
//
//  IUnknown glue.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::unref ( bool allowDeletion )
{
  if ( true == allowDeletion )
  {
    BaseClass::unref();
  }
  else
  {
    BaseClass::unref_nodelete();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the elevation data for this tile.
//
///////////////////////////////////////////////////////////////////////////////

void Tile::buildElevationData ( Usul::Jobs::Job::RefPtr job )
{
  // Have we been cancelled?
  if ( job.valid() && true == job->canceled() )
    job->cancel();

  Body::RefPtr body ( Usul::Threads::Safe::get ( this->mutex(), _body ) );
  const MeshSize size ( _info->meshSize() );
  const Extents extents ( this->extents() );

  // Handle no body.
  if ( false == body.valid() )
    return;

  // Elevation data.
  Minerva::Core::Data::Container::RefPtr elevationData ( body->elevationData() );

  // Build the data for elevation.
  if ( 0x0 != elevationData )
  {
    // Get the rasters.
    typedef Minerva::Core::Visitors::FindRasterLayers Visitor;
    typedef Visitor::RasterLayers Rasters;
    typedef Minerva::Core::Layers::RasterLayer RasterLayer;
    
    Rasters rasters;
    Visitor::RefPtr visitor ( new Visitor ( extents, rasters ) );
    elevationData->accept ( *visitor );
    
    Minerva::Common::IElevationData::RefPtr answer ( 0x0 );
    
    for ( Rasters::const_iterator iter = rasters.begin(); iter != rasters.end(); ++iter )
    {
      //RasterLayer::_checkForCanceledJob ( job );
      
      RasterLayer::RefPtr raster ( *iter );
      if ( raster.valid() )
      {      
        Extents e ( raster->extents() );
        
        // Should the layer be shown?
        const bool shown ( raster->visibility() );
        const bool isLevelRange ( raster->isInLevelRange ( this->level() ) );
        
        if ( ( true == shown ) && ( true == extents.intersects ( e ) ) && ( true == isLevelRange ) )
        {
          Minerva::Common::IElevationData::RefPtr elevationData ( 
            raster->elevationData ( 
                                   *_info,
                                   size[0], size[1], 
                                   job.get(), 0x0 ) );
          if ( elevationData.valid() )
          {
            if ( false == answer.valid() )
            {
              answer = new Minerva::Core::ElevationData ( size[0], size[1] );
            }
            
            for ( unsigned int i = 0; i < size[0]; ++i )
            {
              for ( unsigned int j = 0; j < size[1]; ++j )
              {
                // Get the current value.
                const float value ( elevationData->value ( i, j ) );
                
                // See if the value is in the list of no data values.
                const bool isNoData ( Usul::Predicates::CloseFloat<Minerva::Common::IElevationData::ValueType>::compare ( value, elevationData->noDataValue(), 10 ) );
                
                // Set the value, if it isn't a no data.
                if ( false == isNoData )
                {
                  answer->value ( i, j, value );
                }
              }
            }
          }
        }
      }
    }
    
    this->elevationData ( answer );
  }
}
