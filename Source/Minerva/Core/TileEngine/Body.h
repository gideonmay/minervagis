
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

#ifndef _MINERVA_CORE_TILE_ENGINE_BODY_CLASS_H_
#define _MINERVA_CORE_TILE_ENGINE_BODY_CLASS_H_

#include "Minerva/Core/Macros.h"
#include "Minerva/Core/TileEngine/LandModel.h"
#include "Minerva/Core/TileEngine/SplitCallbacks.h"
#include "Minerva/Core/TileEngine/Tile.h"
#include "Minerva/Core/TileEngine/Typedefs.h"
#include "Minerva/Core/Jobs/BuildRaster.h"
#include "Minerva/Core/Data/Container.h"
#include "Minerva/Core/Utilities/SkyDome.h"

#include "Minerva/Common/IElevationDatabase.h"
#include "Minerva/Common/IPlanetCoordinates.h"

#include "Usul/Interfaces/ILog.h"
#include "Usul/Jobs/Manager.h"
#include "Usul/Math/Vector2.h"
#include "Usul/Math/Vector3.h"
#include "Usul/Threads/Variable.h"

#include "osg/MatrixTransform"

#include <list>

namespace Minerva { namespace Core { namespace Data { class Camera; } } }

namespace Minerva {
namespace Core {
namespace TileEngine {


class MINERVA_EXPORT Body : public Usul::Base::Object,
                            public Minerva::Common::IPlanetCoordinates,
                            public Minerva::Common::IElevationDatabase
{
public:

  // Useful typedefs.
  typedef Usul::Base::Object BaseClass;
  typedef Usul::Math::Vec2d Vec2d;
  typedef Usul::Math::Vec3d Vec3d;
  typedef Minerva::Core::TileEngine::Callbacks::SplitCallback SplitCallback;
  typedef osg::ref_ptr<osg::MatrixTransform> MatrixTransformPtr;
  typedef Minerva::Core::Data::Container Container;
  typedef Minerva::Core::Layers::RasterLayer RasterLayer;
  typedef Usul::Interfaces::IUnknown IUnknown;
  typedef std::list<Tile::RefPtr> Tiles;
  typedef Minerva::Core::Jobs::BuildRaster BuildRaster;
  typedef Usul::Interfaces::ILog::RefPtr LogPtr;  

  // Helper macro for repeated code.
  MINERVA_DEFINE_NODE_CLASS ( Body );

  /// Usul::Interfaces::IUnknown members.
  USUL_DECLARE_IUNKNOWN_MEMBERS;
  
  // Constructors
  Body ( 
    unsigned int numberOfRows = 1,
    unsigned int numberOfColumns = 2,
    Extents extents = Extents ( -180, -90, 180, 90 ),
    LandModel *land = 0x0, 
    Usul::Jobs::Manager *manager = 0x0, 
    const MeshSize &ms = MeshSize ( 16, 16 ), 
    const ImageSize &is = ImageSize ( 256, 256 ),
    double splitDistance = 1 );
  
  // Set/get the flag that says to allow spliting.
  void                      allowSplit ( bool );
  bool                      allowSplit() const;

  // Set an alpha value.
  void                      alpha ( float );

  // Get the alpha value.
  float                     alpha() const;

  // Clear the body.
  void                      clear();
  
  /// Get the container.
  Container::RefPtr         container() const;

  // Deserialize this instance.
  virtual void              deserialize ( const XmlTree::Node &node );

  // Dirty textures and vertices.
  void                      dirtyTextures ( const Extents& e );
  void                      dirtyVertices();

  // Get the elevation data.
  Container::RefPtr         elevationData();

  // Get the elevation at a lat, lon.
  double                    elevation ( double lat, double lon ) const;

  // Get the elevation at a lat, lon (IElevationDatabase).
  virtual double            elevationAtLatLong ( double lat, double lon ) const;
  
  // Append elevation data.
  void                      elevationAppend ( Minerva::Core::Data::Feature * );

  // Return overall extents of the body.
  Extents                   extents() const;
  
  // Intersect only with the tiles (no vector data).
  bool                      intersectWithTiles ( const Usul::Math::Vec3d& pt0, const Usul::Math::Vec3d& pt1, Usul::Math::Vec3d& point );

  // Set/get the job manager for this body.
  void                      jobManager ( Usul::Jobs::Manager * );
  Usul::Jobs::Manager *     jobManager();

  // Set/get the flag that says to keep detail.
  void                      keepDetail ( bool );
  bool                      keepDetail() const;

  // Convert lat, lon, height to x,y,z.
  void                      latLonHeightToXYZ ( double lat, double lon, double elevation, osg::Vec3d& point ) const;
  void                      xyzToLatLonHeight ( const osg::Vec3d& point, double& lat, double& lon, double& elevation ) const;

  /// Convert to planet coordinates.
  virtual void              convertToPlanet ( const Usul::Math::Vec3d& orginal, Usul::Math::Vec3d& planetPoint ) const;
  virtual void              convertFromPlanet ( const Usul::Math::Vec3d& planetPoint, Usul::Math::Vec3d& lonLatPoint ) const;
  
  // Matrix to place items on the planet (i.e. local coordinates to world coordinates).
  virtual osg::Matrixd      planetRotationMatrix ( double lat, double lon, double elevation, double heading ) const;

  // Get the land model.
  LandModel*                landModel() const;
  
  // Set/get the log.
  void                      logSet ( LogPtr );
  LogPtr                    logGet();

  // Get the max degree of anisotropic filtering.
  // See http://en.wikipedia.org/wiki/Anisotropic_filtering
  float                     maxAnisotropy() const;

  // Set/get the maximum level.
  void                      maxLevel ( unsigned int level );
  unsigned int              maxLevel() const;

  // Return the mesh size.
  MeshSize                  meshSize() const;
  
  // Set/get the needs redraw state.
  void                      needsRedraw ( bool b );
  bool                      needsRedraw() const;

  // Purge tiles that are ready.
  void                      purgeTiles();

  // Append raster data.
  void                      rasterAppend ( Minerva::Core::Data::Feature * );
  
  // Remove raster layer
  void                      rasterRemove ( Minerva::Core::Data::Feature * );

  // Raster has chagned.
  void                      rasterChanged ( Minerva::Core::Data::Feature * );
  
  // Get the raster data.
  Container::RefPtr         rasterData();

  // Get the scene.
  const osg::Node *         scene() const;
  osg::Node *               scene();

  // Serialize this instance.
  virtual void              serialize ( XmlTree::Node &parent ) const;

  // See if the tile should split.
  bool                      shouldSplit ( bool suggestion, Tile * );

  // Set/get the callback used for tile splitting. Returns the existing callback.
  SplitCallback::RefPtr     splitCallback();
  void                      splitCallback ( SplitCallback * );

  /// Get/Set the split distance.
  void                      splitDistance ( double distance, bool children = true );
  double                    splitDistance() const;

  // Request texture.
  BuildRaster::RefPtr       textureRequest ( Tile* );

  // Set/get the flag that says to use borders.
  void                      useBorders ( bool );
  bool                      useBorders() const;

  // Set/get the flag that says to use skirts.
  void                      useSkirts ( bool );
  bool                      useSkirts() const;

  // Update the tile's alpha.
  void                      updateTilesAlpha();

  /// Update.
  void                      updateNotify ( Minerva::Core::Data::Camera* camera );

  /// Get the vector data.
  Container::RefPtr         vectorData();
  
  /// Add vector data.
  void                      vectorAppend ( Minerva::Core::Data::Feature *feature );
  
  /// Remove vector data.
  void                      vectorRemove ( Minerva::Core::Data::Feature *feature );
  
protected:

  // Use reference counting.
  virtual ~Body();
  
  // Add a tile for the given extents.
  void                      _addTile ( unsigned int row, unsigned int column, const Extents& extents );
  void                      _addTiles ( unsigned int numberOfRows, unsigned int numberOfColumns, const Extents& extents );

  void                      _addTileToBeDeleted ( Tile::RefPtr tile );

  void                      _updateTileAlpha ( osg::Group *group );
  
  // Get the number of children.
  virtual unsigned int      getNumChildNodes() const;
  
  // Get the child node.
  virtual Minerva::Core::Data::Feature::RefPtr getChildNode ( unsigned int which );
  
private:

  friend class Tile;

  typedef Usul::Threads::Variable<Usul::Jobs::Manager*> JobManager;

  // No copying or assignment.
  Body ( const Body & );
  Body &operator = ( const Body & );

  void                      _destroy();
  
  void                      _dirtyTiles ( unsigned int flags );
  void                      _dirtyTiles ( unsigned int flags, const Extents& extents );

  MatrixTransformPtr _transform;
  LandModel::RefPtr _landModel;
  Container::RefPtr _container;
  JobManager _manager;
  unsigned int _maxLevel;
  double _splitDistance;
  MeshSize _meshSize;
  bool _useSkirts;
  bool _useBorders;
  SplitCallback::RefPtr _splitCallback;
  Tiles _deleteTiles;
  Tiles _topTiles;
  bool _allowSplit;
  bool _keepDetail;
  Minerva::Core::Utilities::SkyDome::RefPtr _sky;
  bool _needsRedraw;
  LogPtr _log;
  std::string _name;
  ImageSize _imageSize;
  float _alpha;
  float _maxAnisotropy;
  unsigned int _numberOfRows;
  unsigned int _numberOfColumns;
  Extents _extents;

  SERIALIZE_XML_CLASS_NAME ( Body );
  SERIALIZE_XML_ADD_MEMBER_FUNCTION;
  SERIALIZE_XML_DEFINE_MAP;
};


} // namespace TileEngine
} // namespace Core
} // namespace Minerva


#endif // _MINERVA_CORE_TILE_ENGINE_BODY_CLASS_H_
