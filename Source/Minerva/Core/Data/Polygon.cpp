
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/Polygon.h"

#include "Minerva/OsgTools/ConvertVector.h"
#include "Minerva/OsgTools/StateSet.h"

#include "Minerva/Common/IPlanetCoordinates.h"

#include "Usul/Threads/Safe.h"

#include "osg/Material"
#include "osg/MatrixTransform"
#include "osg/PolygonOffset"
#include "osg/Geode"
#include "osg/Geometry"

#include "osgUtil/Tessellator"
#include "osgUtil/SmoothingVisitor"

using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Polygon::Polygon ( ) :
  BaseClass(),
  _outerBoundary(),
  _boundaries()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Polygon::~Polygon()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the outer boundary.
//
///////////////////////////////////////////////////////////////////////////////

void Polygon::outerBoundary ( Line::RefPtr line )
{
  Guard guard ( this );
  _outerBoundary = line;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the outer boundary.
//
///////////////////////////////////////////////////////////////////////////////

Line::RefPtr Polygon::outerBoundary() const
{
  Guard guard ( this );
  return _outerBoundary;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add an inner boundary.
//
///////////////////////////////////////////////////////////////////////////////

void Polygon::addInnerBoundary ( Line::RefPtr line )
{
  Guard guard ( this->mutex() );
  _boundaries.push_back ( line );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the inner boundaries.
//
///////////////////////////////////////////////////////////////////////////////

const Polygon::Boundaries& Polygon::innerBoundaries() const
{
  Guard guard ( this->mutex() );
  return _boundaries;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Predicate for comparing vertices.  Should use CloseFloat for robustness?
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  struct VectorCompare
  {
    template < class T >
    bool operator() ( const T& lhs, const T& rhs ) const
    {
      return lhs.equal( rhs );
    }
  };
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build geometry from vertices.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Polygon::_buildGeometry ( const PolyStyle& polyStyle, Minerva::Common::Coordinates::RefPtr inVertices, Extents& e, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  // Make sure we have vertices.
  if ( !inVertices || inVertices->size() < 3 )
    return 0x0;
  
  // Vertices and normals.
  osg::ref_ptr<osg::Vec3Array> vertices ( new osg::Vec3Array );
  osg::ref_ptr<osg::Vec3Array> normals  ( new osg::Vec3Array );
  osg::ref_ptr<osg::Vec4Array> colors  ( new osg::Vec4Array ( inVertices->size() ) );
  osg::Vec4f color ( Usul::Convert::Type<Color,osg::Vec4f>::convert ( polyStyle.color() ) );
  std::fill ( colors->begin(), colors->end(), color );
  
  // Reserve enough rooms.
  vertices->reserve ( inVertices->size() );
  normals->reserve ( inVertices->size() );
  
  Vertices convertedPoints;
  convertedPoints.reserve ( inVertices->size() );
  
  // Expand the vertex.
  e.expand ( inVertices->extents() );
  
  for ( Coordinates::const_iterator iter = inVertices->begin(); iter != inVertices->end(); ++iter )
  {
    Coordinates::value_type v0 ( *iter ), p0;
    
    if ( elevation )
      v0[2] = Minerva::Core::Data::getElevationAtPoint ( v0, elevation, this->altitudeMode() );
    
    if ( planet )
      planet->convertToPlanet ( v0, p0 );
    
    convertedPoints.push_back ( p0 );
  }

  // Subtract the first point from all vertices.
  const Vertices::value_type offset ( convertedPoints.front() );
  for ( Vertices::const_iterator iter = convertedPoints.begin(); iter != convertedPoints.end(); ++iter )
  {
    const Vertices::value_type value ( *iter );

    // Add the normal.
    Vertices::value_type n0 ( value );
    n0.normalize();
    normals->push_back ( osg::Vec3 ( n0[0], n0[1], n0[2] ) );

    // Add the vertex.
    const Vertices::value_type vertex ( value - offset );
    vertices->push_back ( osg::Vec3f ( vertex[0], vertex[1], vertex[2] ) );    
  }
  
  osg::ref_ptr < osg::Geometry > geom ( new osg::Geometry );
  
  geom->setVertexArray ( vertices.get() );
  geom->setNormalArray ( normals.get() );
  geom->setNormalBinding ( osg::Geometry::BIND_PER_VERTEX );
  geom->setColorArray ( colors.get() );
  geom->setColorBinding ( osg::Geometry::BIND_PER_VERTEX );
  
  geom->addPrimitiveSet ( new osg::DrawArrays ( GL_POLYGON, 0, vertices->size() ) );
  
  // Make into triangles.
  osg::ref_ptr<osgUtil::Tessellator> tessellator ( new osgUtil::Tessellator );
  tessellator->retessellatePolygons ( *geom );
  
  // Make normals.
  osgUtil::SmoothingVisitor::smooth ( *geom );
  
  osg::ref_ptr<osg::Geode> geode ( new osg::Geode );
  geode->addDrawable ( geom.get() );

  osg::ref_ptr<osg::MatrixTransform> mt ( new osg::MatrixTransform );
  mt->setMatrix ( osg::Matrixd::translate ( osg::Vec3d ( offset[0], offset[1], offset[2] ) ) );
  mt->addChild ( geode.get() );

  return mt.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert to planet coordinates.
//
///////////////////////////////////////////////////////////////////////////////

Polygon::Vertex Polygon::_convertToPlanetCoordinates ( const Polygon::Vertex& v, Minerva::Common::IPlanetCoordinates* planet, Minerva::Common::IElevationDatabase* elevation ) const
{
  Polygon::Vertex v0 ( v ), p0;
  
  if ( 0x0 != elevation )
    v0[2] = Minerva::Core::Data::getElevationAtPoint ( v0, elevation, this->altitudeMode() );
  
  if ( 0x0 != planet )
    planet->convertToPlanet ( v0, p0 );
  
  return p0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make a tri-strip from vertex array to the ground.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Polygon::_extrudeToGround ( const PolyStyle& polyStyle, Minerva::Common::Coordinates::RefPtr inVertices, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  if ( !inVertices )
  {
    return 0x0;
  }
  
  // The number of vertices in the tri-strip.
  const unsigned int numVertices ( inVertices->size() * 2 );

  // Vertices and normals.
  osg::ref_ptr<osg::Vec3Array> vertices ( new osg::Vec3Array );
  osg::ref_ptr<osg::Vec3Array> normals  ( new osg::Vec3Array );
  osg::ref_ptr<osg::Vec4Array> colors  ( new osg::Vec4Array ( numVertices ) );
  osg::Vec4f color ( Usul::Convert::Type<Color,osg::Vec4f>::convert ( polyStyle.color() ) );
  std::fill ( colors->begin(), colors->end(), color );

  // Reserve enough rooms.
  vertices->reserve( numVertices );
  normals->reserve( numVertices );

  Vertices convertedPoints;
  convertedPoints.reserve ( numVertices );

  // Loop over the vertices.
  for ( Coordinates::const_iterator iter = inVertices->begin(); iter != inVertices->end(); ++iter )
  {
    Vertex top ( *iter );
    Vertex bottom ( top ); bottom[2] = 0.0;

    Vertex p0 ( this->_convertToPlanetCoordinates ( top, planet, elevation ) );
    Vertex p1 ( this->_convertToPlanetCoordinates ( bottom, planet, elevation ) );

    convertedPoints.push_back ( p0 );
    convertedPoints.push_back ( p1 );

    p0.normalize();
    p1.normalize();

    normals->push_back ( osg::Vec3 ( p1[0], p1[1], p1[2] ) );
    normals->push_back ( osg::Vec3 ( p0[0], p0[1], p0[2] ) );
  }

  // Subtract the first point from all vertices.
  const Vertices::value_type offset ( convertedPoints.front() );
  for ( Vertices::const_iterator iter = convertedPoints.begin(); iter != convertedPoints.end(); ++iter )
  {
    const Vertices::value_type value ( *iter );
    const Vertices::value_type vertex ( value - offset );
    vertices->push_back ( osg::Vec3f ( vertex[0], vertex[1], vertex[2] ) );
  }
  
  // Make the geometry.
  osg::ref_ptr < osg::Geometry > geom ( new osg::Geometry );
  
  geom->setVertexArray ( vertices.get() );
  geom->setNormalArray ( normals.get() );
  geom->setNormalBinding ( osg::Geometry::BIND_PER_VERTEX );
  geom->setColorArray ( colors.get() );
  geom->setColorBinding ( osg::Geometry::BIND_PER_VERTEX );
  
  geom->addPrimitiveSet ( new osg::DrawArrays ( osg::PrimitiveSet::TRIANGLE_STRIP, 0, vertices->size() ) );

  // Make normals.
  osgUtil::SmoothingVisitor::smooth ( *geom );

  osg::ref_ptr<osg::Geode> geode ( new osg::Geode );
  geode->addDrawable ( geom.get() );

  osg::ref_ptr<osg::MatrixTransform> mt ( new osg::MatrixTransform );
  mt->setMatrix ( osg::Matrixd::translate ( osg::Vec3d ( offset[0], offset[1], offset[2] ) ) );
  mt->addChild ( geode.get() );

  return mt.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build a mesh.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Polygon::_buildPolygons ( const PolyStyle& polyStyle, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{  
  // Make new extents.
  Extents e;
  
  // Get the outer boundary.
  Line::RefPtr outerBoundary ( this->outerBoundary() );

  if ( !outerBoundary )
    return 0x0;
  
  // Need at least 3 points to make a polygon.

  // TODO: Handle inner boundaries.
  //Boundaries innerBoundaries ( polygon->innerBoundaries() );

  osg::ref_ptr<osg::Group> group ( new osg::Group );
  
  group->addChild ( this->_buildGeometry ( polyStyle, outerBoundary->coordinates(), e, planet, elevation ) );

  // Extrude if we are suppose to.
  if ( true == this->extrude() )
  {
    group->addChild ( this->_extrudeToGround ( polyStyle, outerBoundary->coordinates(), planet, elevation ) );
  }
  
  this->extents ( e );

  // Set the state.
  osg::ref_ptr < osg::StateSet > ss ( group->getOrCreateStateSet () );
  
  // Make a polygon offset.
  osg::ref_ptr< osg::PolygonOffset > po ( new osg::PolygonOffset( 1.0f, 4.0f ) );
  ss->setMode ( GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
  ss->setAttribute( po.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
  
  // Turn off lighting.
  OsgTools::State::StateSet::setLighting  ( ss.get(), false );
  
  return group.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Pre build the scene.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Polygon::_buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  // Clear what we have by making a new one.
  osg::ref_ptr < osg::Group > group ( new osg::Group );
  
  // Get the state set.
  osg::ref_ptr < osg::StateSet > ss ( group->getOrCreateStateSet () );
  
  PolyStyle::RefPtr polyStyle;
  if ( style )
  {
    polyStyle = style->polystyle();
  }

  if ( !polyStyle )
  {
    polyStyle = new PolyStyle;
  }

  if ( polyStyle->fill() )
  {
    group->addChild ( this->_buildPolygons ( *polyStyle, planet, elevation ) );
    
    // Turn off depth testing if we are clamping to ground.
    // This could probably be done better.
    if ( ALTITUDE_MODE_CLAMP_TO_GROUND == this->altitudeMode() )
    {
      ss->setMode ( GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
    }
  }
  
  if ( polyStyle->outline() )
  {
    Line::RefPtr line ( this->outerBoundary() );
    
    if ( line )
    {
      group->addChild ( line->buildScene ( style, planet, elevation ) );
    }
  }
  
  // Set state set modes depending on alpha value.
  if ( false == Geometry::isSemiTransparent ( style ) )
  {
    // Set the render bin.
    ss->setRenderBinDetails ( this->renderBin(), "RenderBin" );
  }
  
  this->dirty ( false );
  return group.release();
}
