
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  ArcGIS layer class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_RASTER_LAYER_ARC_GIS_H__
#define __MINERVA_CORE_RASTER_LAYER_ARC_GIS_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Layers/RasterLayerNetwork.h"
#include "Minerva/Core/Layers/LayerInfo.h"

#include "Usul/Math/Vector3.h"

#include <string>


namespace Minerva {
namespace Core {
namespace Layers {
      
      
class MINERVA_EXPORT RasterLayerArcGIS : public RasterLayerNetwork
{
public:
  
  typedef RasterLayerNetwork BaseClass;
  
  USUL_DECLARE_REF_POINTERS ( RasterLayerArcGIS );
  
  RasterLayerArcGIS();
  
  /// Clone.
  virtual Feature*      clone() const;
  
  /// Get the full url.
  virtual std::string   urlFull ( const TileKey& key, unsigned int width, unsigned int height ) const;
  
protected:
  
  virtual ~RasterLayerArcGIS();
  
  RasterLayerArcGIS ( const RasterLayerArcGIS& );
  
  virtual void          _download ( const std::string& file, const TileKey& key, unsigned int width, unsigned int height, Usul::Jobs::Job *, IUnknown *caller );
  
private:
  
  // Do not use.
  RasterLayerArcGIS& operator = ( const RasterLayerArcGIS& );
  
  virtual std::string   _cacheFileExtension() const;
  
  SERIALIZE_XML_CLASS_NAME ( RasterLayerArcGIS );
};


} // namespace Layers
} // namespace Core
} // namespace Minerva


#endif // __MINERVA_CORE_RASTER_LAYER_ARC_GIS_H__
