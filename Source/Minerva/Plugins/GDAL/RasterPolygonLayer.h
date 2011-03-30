
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_RASTER_POLYGON_LAYER_H__
#define __MINERVA_RASTER_POLYGON_LAYER_H__

#include "Minerva/Plugins/GDAL/Export.h"

#include "Minerva/Core/Layers/RasterLayer.h"

#include "Minerva/Plugins/GDAL/PostGISLayer.h"

#include "Usul/File/Temp.h"

class GDALDataset;
class OGRGeometry;


namespace Minerva {
namespace Layers {
namespace GDAL {


class MINERVA_GDAL_EXPORT RasterPolygonLayer : public Minerva::Core::Layers::RasterLayer
{
public:
  typedef Minerva::Core::Layers::RasterLayer BaseClass;
  typedef Minerva::Layers::GDAL::PostGISLayer Layer;
  
  RasterPolygonLayer();
  RasterPolygonLayer ( Layer* layer );
  
  /// Clone.
  virtual Minerva::Core::Data::Feature* clone() const;
  
  /// Deserialize.
  virtual void          deserialize ( const XmlTree::Node &node );

protected:
  
  virtual ~RasterPolygonLayer();
  
  RasterPolygonLayer ( const RasterPolygonLayer& );

  void                  _init();
  void                  _initGeometries();

  /// Rasterize.
  ImagePtr              _rasterize ( const Extents& extents, unsigned int width, unsigned int height, unsigned int level );
  
  /// Cache functions.
  virtual LayerKey::RefPtr cacheKey() const;
  virtual std::string   _cacheFileExtension() const;

  /// Read/Write image file.
  void                  _writeImageFile ( ImagePtr image, const std::string& filename ) const;
  
  virtual ImagePtr      _textureImplementation ( 
                                                const std::string& filename, 
                                                const TileKey& key,
                                                unsigned int width, 
                                                unsigned int height,  
                                                Usul::Jobs::Job *, 
                                                IUnknown *caller );
  
private:

  RasterPolygonLayer& operator= ( const RasterPolygonLayer& );
  
  typedef std::vector<OGRGeometry*> Geometries;
  typedef std::vector<double> BurnValues;
  
  Layer::RefPtr _layer;
  std::string _dir;
  std::string _projectionText;
  bool _initialized;
  Geometries _geometries;
  BurnValues _burnValues;
  
  SERIALIZE_XML_CLASS_NAME( RasterPolygonLayer ) 
  SERIALIZE_XML_SERIALIZE_FUNCTION 
  SERIALIZE_XML_ADD_MEMBER_FUNCTION
};

  
}
}
}

#endif // __MINERVA_RASTER_POLYGON_LAYER_H__
