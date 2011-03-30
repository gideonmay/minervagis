
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

#ifndef _MINERVA_CORE_TILE_ENGINE_RECURSIVE_TILE_CLASS_H_
#define _MINERVA_CORE_TILE_ENGINE_RECURSIVE_TILE_CLASS_H_

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Data/Container.h"
#include "Minerva/Core/Layers/RasterLayer.h"
#include "Minerva/Core/TileEngine/Mesh.h"
#include "Minerva/Core/TileEngine/Typedefs.h"

#include "Minerva/Common/Extents.h"
#include "Minerva/Common/IElevationData.h"
#include "Minerva/Common/TileKey.h"
#include "Minerva/Common/ITileVectorData.h"
#include "Minerva/Common/ITile.h"

#include "Usul/Jobs/Job.h"
#include "Usul/Math/Vector4.h"
#include "Usul/Math/Vector2.h"
#include "Usul/Pointers/Pointers.h"
#include "Usul/Threads/RecursiveMutex.h"
#include "Usul/Threads/Guard.h"

#include "osg/Group"
#include "osg/Image"
#include "osg/observer_ptr"
#include "osg/Texture2D"
#include "osg/Vec3d"

#include "boost/shared_ptr.hpp"

namespace Usul { namespace Jobs { class Manager; } }
namespace osgUtil { class CullVisitor; }
namespace Minerva { namespace Core { namespace TileEngine { class Body; } } }

namespace Minerva {
namespace Core {
namespace TileEngine {


class MINERVA_EXPORT Tile : 
  public osg::Group,
  public Minerva::Common::ITile
{
public:

  // OSG Plumbing.
  //META_Node ( Minerva, Tile );

  // Declare smart pointers.
  USUL_DECLARE_REF_POINTERS ( Tile );

  // Usul::Interfaces::IUnknown members.
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  // Dirty flags.
  enum Flags
  {
    VERTICES   = 0x00000001,
    TEXTURE    = 0x00000004,
    CHILDREN   = 0x00000008,
    IMAGE      = 0x00000010,
    VECTOR     = 0x00000020,
    ALL        = VERTICES | TEXTURE | VECTOR
  };

  // Useful typedefs.
  typedef osg::Group BaseClass;
  typedef Usul::Threads::RecursiveMutex Mutex;
  typedef Usul::Threads::Guard<Mutex> Guard;
  typedef std::vector < Tile::RefPtr > Tiles;
  typedef Tiles Children;
  typedef osg::ref_ptr<osg::Image> ImagePtr;
  typedef osg::BoundingSphere BSphere;
  typedef osg::ref_ptr<osg::Node> NodePtr;
  typedef boost::shared_ptr<Mesh> MeshPtr;
  typedef Minerva::Core::Layers::RasterLayer RasterLayer;
  typedef Minerva::Core::Data::Container TileVectorData;
  typedef std::pair < TileVectorData::RefPtr, bool > TileVectorDataPair;
  typedef Minerva::Common::ITileVectorData::Jobs TileVectorJobs;
  typedef Minerva::Common::IElevationData::QueryPtr ElevationDataPtr;
  typedef Minerva::Common::Extents Extents;
  typedef Minerva::Common::TileKey TileKey;
  typedef Usul::Interfaces::IUnknown IUnknown;

  // Constructors.
  Tile ( TileKey::RefPtr key,
         double splitDistance = 1,
         Body *body = 0x0,
         osg::Image * image = 0x0,
         ElevationDataPtr elevation = static_cast<Minerva::Common::IElevationData*> ( 0x0 ),
         TileVectorData::RefPtr tileVectorData = 0x0 );
  Tile ( const Tile &, const osg::CopyOp &copyop = osg::CopyOp::SHALLOW_COPY );

  // Add vector data.
  void                      addVectorData ( osg::Node* );

  // Clear the tile.
  void                      clear ( bool children );

  // Get the child at index i.
  Tile::RefPtr              childAt ( unsigned int i ) const;

  // Build raster and vector data.
  void                      buildElevationData ( Usul::Jobs::Job::RefPtr );
  void                      buildPerTileVectorData ( Usul::Jobs::Job::RefPtr );
  void                      buildRaster ( Usul::Jobs::Job::RefPtr );

  // Compute the bounding sphere.
  virtual BSphere           computeBound() const;

  // Set the flag that says we're dirty.
  void                      dirty ( bool state, unsigned int flags, bool dirtyChildren );

  // Mark the dirty state if we cross this extents.
  void                      dirty ( bool state, unsigned int flags, bool dirtyChildren, const Extents& extents);

  // Get the individual dirty states.
  bool                      verticesDirty() const;
  bool                      textureDirty() const;

  // Get the elevation at lat, lon.
  double                    elevation ( double lat, double lon );

  // Set/get the elevation data.
  void                      elevationData ( ElevationDataPtr data );
  ElevationDataPtr          elevationData() const;

  // Get the extents.
  Extents                   extents() const;

  // Get the image.
  ImagePtr                  image();

  // Is this tile a leaf?
  bool                      isLeaf() const;

  // Get the body's job manager.
  Usul::Jobs::Manager *     jobManager();

  // Convience function that re-directs to the body.
  void                      latLonHeightToXYZ ( double lat, double lon, double elevation, osg::Vec3d& point ) const;

  // Return level of this tile. Zero is the top.
  unsigned int              level() const;

  // Return the mutex. Use with caution.
  Mutex &                   mutex() const;

  // Get the size.
  MeshSize                  meshSize() const;

  // Remove vector data.
  void                      removeVectorData ( osg::Node* );

  // Set the texture data.
  void                      textureData ( osg::Image* image );

  // Minerva::Common::ITile
  virtual Tile *            tile();

  // Traverse the children.
  virtual void              traverse ( osg::NodeVisitor & );
  
  // Get/Set the split distance.
  double                    splitDistance() const;
  void                      splitDistance( double distance, bool children );
  
  // Split the tile.
  void                      split ( Usul::Jobs::Job::RefPtr );
  
  // Update stuff.
  void                      update();
  void                      updateAlpha();
  void                      updateMesh();
  void                      updateTexture();
  void                      updateTileVectorData();

protected:

  // Use reference counting.
  virtual ~Tile();

  void                      _cancelTileJob();
  void                      _cancelTileVectorJobs();

  void                      _cull ( osgUtil::CullVisitor &cv );

  /// Clear children.
  void                      _clearChildren ( bool traverse, bool cancelJob );

  void                      _deleteMe();
  
  // Load the image.
  void                      _launchImageRequest();
  void                      _launchElevationRequest();

  void                      _perTileVectorDataClear();
  void                      _perTileVectorDataDelete();
  bool                      _perTileVectorDataIsInherited() const;
  TileVectorData::RefPtr    _perTileVectorDataGet();

  // Build a tile.
  Tile::RefPtr              _buildTile ( TileKey::RefPtr key, 
                                         double splitDistance, 
                                         Usul::Jobs::Job::RefPtr job );
  
  void                      _setDirtyAlways ( bool state, unsigned int flags, bool dirtyChildren );
  void                      _setDirtyIfIntersect ( bool state, unsigned int flags, bool dirtyChildren, const Extents& extents );

  // Set the show border state.
  void                      _setShowBorders ( bool show );

  // Set the show skirt state.
  void                      _setShowSkirts ( bool show );

  // Update the border-showing state.
  void                      _updateShowBorders();

  // Update stuff.
  void                      _updateTiles();

private:

  // No assignment.
  Tile &operator = ( const Tile & );

  void                      _destroy();

  mutable Mutex *_mutex;
  Body *_body;
  const TileKey::RefPtr _info;
  double _splitDistance;
  MeshPtr _mesh;
  unsigned int _flags;
  Children _children;
  ImagePtr _image;
  ElevationDataPtr _elevation;
  osg::ref_ptr < osg::Texture2D > _texture;
  Usul::Jobs::Job::RefPtr _imageJob;
  Usul::Jobs::Job::RefPtr _elevationJob;
  Usul::Jobs::Job::RefPtr _tileJob;
  osg::BoundingSphere _boundingSphere;
  osg::ref_ptr<osg::Group> _vector;
  TileVectorDataPair _tileVectorData;
  TileVectorJobs _tileVectorJobs;
  bool _childrenNeedCleared;
};


} // namespace TileEngine
} // namespace Core
} // namespace Minerva


#endif // _MINERVA_CORE_TILE_ENGINE_RECURSIVE_TILE_CLASS_H_
