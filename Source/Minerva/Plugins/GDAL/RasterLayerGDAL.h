
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_GDAL_LAYER_H__
#define __MINERVA_GDAL_LAYER_H__

#include "Minerva/Plugins/GDAL/Export.h"
#include "Minerva/Plugins/GDAL/Dataset.h"

#include "Minerva/Core/Layers/RasterLayer.h"

#include "gdalwarper.h"

class GDALDataset;
class GDALRasterBand;

namespace Minerva {
namespace Layers {
namespace GDAL {

class MINERVA_GDAL_EXPORT RasterLayerGDAL : public Minerva::Core::Layers::RasterLayer
{
public:
  typedef Minerva::Core::Layers::RasterLayer BaseClass;
  typedef Usul::Interfaces::IUnknown IUnknown;
  typedef std::vector<double> GeoTransform;
  
  USUL_DECLARE_REF_POINTERS ( RasterLayerGDAL );
  
  RasterLayerGDAL();
  
  /// Clone.
  virtual Minerva::Core::Data::Feature* clone() const;
  
  /// Read.
  virtual void          read ( const std::string& filename );
  
  /// Deserialize.
  virtual void          deserialize ( const XmlTree::Node &node );
  
  /// Get the raster data as elevation data.
  virtual IElevationData::RefPtr elevationData ( 
                                                const TileKey& key,
                                                unsigned int width,
                                                unsigned int height,
                                                Usul::Jobs::Job* job,
                                                Usul::Interfaces::IUnknown* caller );

  std::string projection() const;

  // Get the size of underlying image.
  Usul::Math::Vec2ui size() const;

protected:
  
  virtual ~RasterLayerGDAL();
  
  RasterLayerGDAL ( const RasterLayerGDAL& );

  virtual LayerKey::RefPtr cacheKey() const;
  virtual std::string   _cacheFileExtension() const;

  virtual ImagePtr      _textureImplementation ( 
    const std::string& filename, 
    const TileKey& key,
    unsigned int width, 
    unsigned int height,  
    Usul::Jobs::Job *, 
    IUnknown *caller );

  Dataset::RefPtr _createTile ( 
    const std::string& filename, 
    const Extents& extents, 
    unsigned int requestedWidth, 
    unsigned int requestedHeight );

  float _getElevationData ( GDALRasterBand* band, double longitude, double latitude, float noDataValue );

private:
  
  // No assignment.
  RasterLayerGDAL& operator= ( const RasterLayerGDAL& );
  
  GDALDataset *_data;
  GDALDataset *_warpedData;
  std::vector<double> _geoTransform;
  std::vector<double> _invGeoTransform;
  std::string _filename;
  
  SERIALIZE_XML_CLASS_NAME( RasterLayerGDAL ) 
};

}
}
}

#endif // __MINERVA_GDAL_LAYER_H__
