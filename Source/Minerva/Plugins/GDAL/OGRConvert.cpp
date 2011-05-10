
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

#include "Minerva/Plugins/GDAL/OGRConvert.h"
#include "Minerva/Core/Data/Point.h"
#include "Minerva/Core/Data/Line.h"
#include "Minerva/Core/Data/Polygon.h"
#include "Minerva/Core/Data/MultiGeometry.h"

#include "ogr_api.h"
#include "ogrsf_frmts.h"

using namespace Minerva::Layers::GDAL;


///////////////////////////////////////////////////////////////////////////////
//
//  Create a geometry.
//
///////////////////////////////////////////////////////////////////////////////

OGRConvert::Geometry* OGRConvert::geometry ( OGRGeometry* geometry, OGRCoordinateTransformation *transform, double verticalOffset )
{
  if ( 0x0 != geometry )
  {
    switch ( geometry->getGeometryType() )
    {
    case wkbPoint:
    case wkbPoint25D:
    case wkbLineString:
    case wkbLineString25D:
    case wkbLinearRing:
    case wkbPolygon:
    case wkbPolygon25D:
      return OGRConvert::_geometry ( geometry, transform, verticalOffset );
      break;
    case wkbMultiPoint:
    case wkbMultiPoint25D:
    case wkbMultiLineString:
    case wkbMultiLineString25D:
    case wkbMultiPolygon:
    case wkbMultiPolygon25D:
    case wkbGeometryCollection:
    case wkbGeometryCollection25D:
      {
        OGRGeometryCollection* collection ( static_cast<OGRGeometryCollection*> ( geometry ) );
        Minerva::Core::Data::MultiGeometry::RefPtr multiGeometry ( new Minerva::Core::Data::MultiGeometry );
        for ( int i = 0; i < collection->getNumGeometries(); ++i )
        {
          multiGeometry->addGeometry ( OGRConvert::_geometry ( collection->getGeometryRef ( i ), transform, verticalOffset ) );
        }
        return multiGeometry.release();
      }
      break;
    }
  }

  return 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert and transform a point.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  inline Usul::Math::Vec3d convertAndTransform ( const OGRPoint& point, OGRCoordinateTransformation* transform, double verticalOffset )
  {
    // Declare the point.
    Usul::Math::Vec3d p ( point.getX(), point.getY(), point.getZ() + verticalOffset );

    // Transform the point.
    if ( 0x0 != transform )
    {
      transform->Transform ( 1, &p[0], &p[1], &p[2] );
    }

    return p;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert and transform a line string.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  template <class Vertices>
  inline void convertAndTransform ( Vertices &vertices, const OGRLineString& line, OGRCoordinateTransformation* transform, double verticalOffset )
  {
    OGRPoint point;

    const int numPoints ( line.getNumPoints() );
    vertices.reserve ( numPoints );
    for ( int i = 0; i < numPoints; ++i )
    {
      line.getPoint ( i, &point );
      vertices.push_back ( Helper::convertAndTransform ( point, transform, verticalOffset ) );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create a point.
//
///////////////////////////////////////////////////////////////////////////////

OGRConvert::Geometry* OGRConvert::point ( OGRPoint* geometry, OGRCoordinateTransformation *transform, double verticalOffset )
{
  Minerva::Core::Data::Point::RefPtr point ( new Minerva::Core::Data::Point );

  if ( 0x0 != geometry )
  {
    // Set the point.
    point->point ( Helper::convertAndTransform ( *geometry, transform, verticalOffset ) );
  }

  return point.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create a line.
//
///////////////////////////////////////////////////////////////////////////////

OGRConvert::Geometry* OGRConvert::line ( OGRLineString* geometry, OGRCoordinateTransformation *transform, double verticalOffset )
{
  typedef Minerva::Core::Data::Line Line;
  typedef Line::Vertices Vertices;

  Line::RefPtr line ( new Line );

  if ( 0x0 != geometry )
  {
    Vertices vertices;
    Helper::convertAndTransform ( vertices, *geometry, transform, verticalOffset );

    line->line ( vertices );
  }

  return line.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create a polygon.
//
///////////////////////////////////////////////////////////////////////////////

OGRConvert::Geometry* OGRConvert::polygon ( OGRPolygon* geometry, OGRCoordinateTransformation *transform, double verticalOffset )
{
  typedef Minerva::Core::Data::Polygon Polygon;
  typedef Polygon::Vertices Vertices;

  Polygon::RefPtr polygon ( new Polygon );

  if ( 0x0 != geometry )
  {
    // Add the outer rings.
    OGRLinearRing *outer ( geometry->getExteriorRing() );
    if ( 0x0 != outer )
    {
      Vertices vertices;
      Helper::convertAndTransform ( vertices, *outer, transform, verticalOffset );
      polygon->outerBoundary ( vertices );
    }

    // Add the inner rings.
    const int numRings ( geometry->getNumInteriorRings() );
    for ( int i = 0; i < numRings; ++i )
    {
      OGRLinearRing* ring ( geometry->getInteriorRing ( i ) );
      if ( 0x0 != ring )
      {
        Vertices vertices;
        Helper::convertAndTransform ( vertices, *outer, transform, verticalOffset );
        polygon->addInnerBoundary ( vertices );
      }
    }
  }

  return polygon.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create a geometry.
//
///////////////////////////////////////////////////////////////////////////////

OGRConvert::Geometry* OGRConvert::_geometry ( OGRGeometry* geometry, OGRCoordinateTransformation *transform, double verticalOffset )
{
  if ( 0x0 == geometry )
      return 0x0;

    switch ( geometry->getGeometryType() )
    {
    case wkbPoint:
    case wkbPoint25D:
      return OGRConvert::point ( static_cast<OGRPoint*> ( geometry ), transform, verticalOffset );
    case wkbLineString:
    case wkbLineString25D:
    case wkbLinearRing:
      return OGRConvert::line ( static_cast<OGRLineString*> ( geometry ), transform, verticalOffset );
    case wkbPolygon:
    case wkbPolygon25D:
      return OGRConvert::polygon ( static_cast<OGRPolygon*> ( geometry ), transform, verticalOffset );
    }

    return 0x0;
}
