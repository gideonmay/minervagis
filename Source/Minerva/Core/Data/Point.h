
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_POSTGIS_POINT_GEOMETRY_H__
#define __MINERVA_POSTGIS_POINT_GEOMETRY_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Data/Geometry.h"
#include "Minerva/Core/Data/PointStyle.h"

#include "Minerva/OsgTools/ShapeFactory.h"

#include "Usul/Math/Vector3.h"

#include "osg/MatrixTransform"

namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT Point : public Geometry
{
public:
  typedef Geometry BaseClass;
  typedef Usul::Math::Vec3d Vec3d;
  typedef Usul::Math::Vec4f Color;
  typedef osg::MatrixTransform MatrixTransform;

  USUL_DECLARE_QUERY_POINTERS ( Point );
  
  Point();
  
  /// Get the shape factory.
  static OsgTools::ShapeFactory* shapeFactory();

  /// Get/Set the point.
  void                    point ( const Usul::Math::Vec3d & );
  const Vec3d             point() const;

protected:
  
  /// Use reference counting.
  virtual ~Point();

  /// Build the scene branch.
  virtual osg::Node*    _buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation );
  
  osg::Node*            _buildGeometry ( PointStyle::RefPtr style, const osg::Vec3d& earthLocation );
  
  osg::Node*            _buildPoint( PointStyle::RefPtr style, const osg::Vec3d& earthLocation );
  osg::Node*            _buildSphere( PointStyle::RefPtr style, const osg::Vec3d& earthLocation );
  
private:

  Vec3d        _point;

  /// Shape Factory to share across all points.
  static OsgTools::ShapeFactory::Ptr _sf;
};

}
}
}


#endif // __MINERVA_POSTGIS_POINT_GEOMETRY_H__
