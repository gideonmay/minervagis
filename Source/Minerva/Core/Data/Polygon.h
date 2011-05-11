
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_POSTGIS_POLYGON_GEOMETRY_H__
#define __MINERVA_POSTGIS_POLYGON_GEOMETRY_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Data/Line.h"
#include "Minerva/Core/Data/PolyStyle.h"

#include "Usul/Math/Vector3.h"

#include <vector>

namespace osg { class Geometry; }
namespace Minerva { namespace Interfaces { struct IPlanetCoordinates; struct IElevationDatabase; } }


namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT Polygon : public Geometry
{
public:
  typedef Geometry                         BaseClass;
  typedef ColorStyle::Color                Color;
  typedef Minerva::Common::IPlanetCoordinates IPlanetCoordinates;
  typedef Minerva::Common::IElevationDatabase IElevationDatabase;
  typedef std::vector<Line::RefPtr> Boundaries;

  USUL_DECLARE_QUERY_POINTERS ( Polygon );

  Polygon();

  void                  outerBoundary ( Line::RefPtr );
  Line::RefPtr          outerBoundary() const;

  void                  addInnerBoundary ( Line::RefPtr );
  const Boundaries&     innerBoundaries() const;

protected:
  
  typedef Usul::Math::Vec3d            Vertex;
  typedef std::vector < Vertex >       Vertices;
  typedef Minerva::Common::Coordinates Coordinates;
  
  virtual ~Polygon();
  
  /// Build the scene branch for the data object.
  virtual osg::Node*    _buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation );
  
  osg::Node*            _buildPolygons ( const PolyStyle& polyStyle, IPlanetCoordinates *planet, IElevationDatabase* elevation );
  
  osg::Node*            _buildGeometry ( const PolyStyle& polyStyle, Coordinates::RefPtr inVertices, Extents& e, IPlanetCoordinates *planet, IElevationDatabase* elevation );
  osg::Node*            _extrudeToGround ( const PolyStyle& polyStyle, Coordinates::RefPtr, IPlanetCoordinates *planet, IElevationDatabase* elevation );

  Vertex                _convertToPlanetCoordinates ( const Polygon::Vertex& v, IPlanetCoordinates* planet, IElevationDatabase* elevation ) const;

private:
  
  Line::RefPtr _outerBoundary;
  Boundaries _boundaries;
};

}
}
}


#endif // __MINERVA_POSTGIS_POLYGON_GEOMETRY_H__

