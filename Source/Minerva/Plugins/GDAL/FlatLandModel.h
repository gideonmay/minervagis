
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  A flat land model.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_PLUGINS_GDAL_FLAT_LAND_MODEL_H__
#define __MINERVA_PLUGINS_GDAL_FLAT_LAND_MODEL_H__

#include "Minerva/Plugins/GDAL/Export.h"

#include "Minerva/Core/TileEngine/LandModel.h"

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"

class OGRSpatialReference;
class OGRCoordinateTransformation;

namespace Minerva {
namespace Layers {
namespace GDAL {

class MINERVA_GDAL_EXPORT FlatLandModel : public Minerva::Core::TileEngine::LandModel
{
public:

  typedef Minerva::Core::TileEngine::LandModel BaseClass;

  FlatLandModel ( const std::string& projection );

  // Convert lat, lon, height to x,y,z.
  virtual void        latLonHeightToXYZ ( double lat, double lon, double elevation, double& x, double& y, double& z ) const;
  virtual void        xyzToLatLonHeight ( double x, double y, double z, double& lat, double& lon, double& elevation ) const;

  // Matrix to place items on the planet (i.e. local coordinates to world coordinates).
  virtual Matrix      planetRotationMatrix ( double lat, double lon, double elevation, double heading ) const;

protected:

  virtual ~FlatLandModel();

private:
  
  typedef boost::scoped_ptr<OGRSpatialReference> OGRSpatialReferencePtr;
  typedef boost::shared_ptr<OGRCoordinateTransformation> OGRCoordinateTransformationPtr;

  OGRSpatialReferencePtr _spatialReference;
  OGRCoordinateTransformationPtr _toSRStransform;
  OGRCoordinateTransformationPtr _fromSRStransform;
};

}
}
}

#endif // __MINERVA_PLUGINS_GDAL_FLAT_LAND_MODEL_H__
