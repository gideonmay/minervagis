
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Convert OGRGeometry to corresponding Minerva geometry.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_PLUGINS_GDAL_OGR_CONVERT_H__
#define __MINERVA_PLUGINS_GDAL_OGR_CONVERT_H__

#include "Minerva/Plugins/GDAL/Export.h"

#include "Minerva/Core/Data/Geometry.h"

class OGRCoordinateTransformation;
class OGRGeometry;
class OGRPoint;
class OGRLineString;
class OGRPolygon;

namespace Minerva {
namespace Layers {
namespace GDAL {

struct MINERVA_GDAL_EXPORT OGRConvert
{
  typedef Minerva::Core::Data::Geometry Geometry;

  static Geometry* geometry ( OGRGeometry* geometry,   OGRCoordinateTransformation *transform, double verticalOffset = 0.0 );
  static Geometry* point    ( OGRPoint* geometry,      OGRCoordinateTransformation *transform, double verticalOffset = 0.0 );
  static Geometry* line     ( OGRLineString* geometry, OGRCoordinateTransformation *transform, double verticalOffset = 0.0 );
  static Geometry* polygon  ( OGRPolygon* geometry,    OGRCoordinateTransformation *transform, double verticalOffset = 0.0 );

private:
  static Geometry* _geometry ( OGRGeometry* geometry,   OGRCoordinateTransformation *transform, double verticalOffset );
};

}
}
}

#endif // __MINERVA_PLUGINS_GDAL_OGR_CONVERT_H__
