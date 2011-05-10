
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/Point.h"
#include "Minerva/OsgTools/SortBackToFront.h"
#include "Minerva/OsgTools/Ray.h"
#include "Minerva/OsgTools/StateSet.h"
#include "Minerva/OsgTools/ConvertVector.h"

#include "Minerva/Common/IElevationDatabase.h"
#include "Minerva/Common/IPlanetCoordinates.h"

#include "Usul/Threads/Safe.h"

#include "osg/Geometry"
#include "osg/Point"
#include "osg/Geode"
#include "osg/Material"
#include "osg/AutoTransform"


using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Static Member.
//
///////////////////////////////////////////////////////////////////////////////

OsgTools::ShapeFactory::Ptr Point::_sf ( new OsgTools::ShapeFactory );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Point::Point() : BaseClass(),
  _point()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Point::~Point()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the point.
//
///////////////////////////////////////////////////////////////////////////////

void Point::point ( const Usul::Math::Vec3d& p )
{
  Guard guard ( this->mutex() );
  _point = p;
  
  // Set the extents.
  this->extents ( Extents ( Extents::Vertex ( p[0], p[1] ), Extents::Vertex ( p[0], p[1] ) ) );
  
  // Dirty.
  this->dirty( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the point.
//
///////////////////////////////////////////////////////////////////////////////

const Usul::Math::Vec3d Point::point() const
{
  Guard guard ( this->mutex() );
  return _point;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene branch for the data object.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Point::_buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  // Get the point style to use later.
  PointStyle::RefPtr pointStyle ( new PointStyle );
  if ( style && style->pointstyle() )
  {
    pointStyle = style->pointstyle();
  }
  
  // Return an empty group if the primitive id is none.
  if ( PointStyle::NONE == pointStyle->primitiveId() )
    return new osg::Group;
  
  /// Get the center from our data source.
  Usul::Math::Vec3d location ( this->point() );

  // Set the height.
  location[2] = Minerva::Core::Data::getElevationAtPoint ( location, elevation, this->altitudeMode() );
  
  const double height ( elevation ? elevation->elevationAtLatLong ( location[1], location[0] ) : 0.0 );
  
  // Convert to planet coordinates.
  if( planet )
  {
    planet->convertToPlanet ( Usul::Math::Vec3d ( location ), location );
  }

  // Location on earth in cartesian coordinates.
  osg::Vec3d earthLocation ( location[0], location[1], location[2] );

  osg::ref_ptr < osg::Node > geometry ( this->_buildGeometry ( pointStyle, earthLocation ) );
  
  // Get the state set
  osg::ref_ptr < osg::StateSet > ss ( geometry->getOrCreateStateSet() );
  
  // Set the render bin.
  ss->setRenderBinDetails ( this->renderBin(), "RenderBin" );
  
  // Set the material.
  osg::Vec4f color ( Usul::Convert::Type<Color,osg::Vec4f>::convert ( pointStyle->color() ) );
  OsgTools::State::StateSet::setMaterial ( geometry.get(), color, color, color.w() );
  
  // Make sure lighting is on.
  OsgTools::State::StateSet::setLighting ( ss.get(), true );
  
  // Make the group.
  osg::ref_ptr < osg::Group > group ( new osg::Group );
  
  // Add the geometry to our group.
  group->addChild ( geometry.get () );
  
  if ( this->extrude() )
  {
    Usul::Math::Vec3d p ( location[0], location[1], height );
    if( planet )
    {
      planet->convertToPlanet ( Usul::Math::Vec3d ( p ), p );
    }
    
    OsgTools::Ray ray;
    ray.thickness ( 1 );
    ray.color ( osg::Vec4 ( 1.0, 1.0, 1.0, 1.0 ) );
    ray.start ( osg::Vec3 ( p[0], p[1], p[2] ) );
    ray.end ( earthLocation );
    
    group->addChild ( ray() );
  }
  
  this->dirty ( false );
  
  return group.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the geometry.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Point::_buildGeometry ( PointStyle::RefPtr style, const osg::Vec3d& earthLocation )
{
  // Redirect to proper build function.
  switch ( style->primitiveId() )
  {
  case PointStyle::POINT:  return this->_buildPoint ( style, earthLocation );
  case PointStyle::SPHERE: return this->_buildSphere ( style, earthLocation );
  }
  
  return 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build a point.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Point::_buildPoint ( PointStyle::RefPtr style, const osg::Vec3d& earthLocation )
{
  osg::ref_ptr< osg::Geode > geode ( new osg::Geode );
  
  osg::ref_ptr < osg::Geometry > geometry ( new osg::Geometry );
  
  osg::ref_ptr< osg::Vec3Array > vertices ( new osg::Vec3Array );
  vertices->push_back ( osg::Vec3 ( 0.0, 0.0, 0.0 ) );
  
  geometry->setVertexArray( vertices.get() );
  
  osg::ref_ptr < osg::Vec4Array > colors ( new osg::Vec4Array );
  colors->push_back( Usul::Convert::Type<Color,osg::Vec4f>::convert ( style->color() ) );
  
  geometry->setColorArray ( colors.get() );
  geometry->setColorBinding ( osg::Geometry::BIND_OVERALL );
  
  osg::ref_ptr < osg::Vec3Array > normals ( new osg::Vec3Array );
  normals->push_back( osg::Vec3( 0.0f,-1.0f, 0.0f ) );
  geometry->setNormalArray( normals.get() );
  geometry->setNormalBinding( osg::Geometry::BIND_OVERALL );
  
  osg::ref_ptr < osg::Point > ps ( new osg::Point );
  ps->setSize ( style->size() );
	
  osg::ref_ptr < osg::StateSet > ss ( geometry->getOrCreateStateSet() );
  ss->setAttributeAndModes( ps.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
  
  // Turn off lighting.
  OsgTools::State::StateSet::setLighting ( ss.get(), false );
  
  geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::POINTS, 0, vertices->size() ) );
  
  geode->addDrawable ( geometry.get() );

  osg::ref_ptr<MatrixTransform> mt ( new MatrixTransform );
  mt->setMatrix ( osg::Matrix::translate ( earthLocation ) );
  mt->addChild ( geode.get() );
   
  return mt.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build a sphere.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Point::_buildSphere ( PointStyle::RefPtr style, const osg::Vec3d& earthLocation )
{
  osg::ref_ptr< osg::Geode > geode ( new osg::Geode );

  const unsigned int size ( 20.0 );
  OsgTools::ShapeFactory::MeshSize meshSize ( size, size );
  OsgTools::ShapeFactory::LatitudeRange  latRange  ( 89.9f, -89.9f );
  OsgTools::ShapeFactory::LongitudeRange longRange (  0.0f, 360.0f );

  osg::ref_ptr < osg::Geometry> geometry ( _sf->sphere ( style->size(), meshSize, latRange, longRange ) );
  geometry->setUseDisplayList ( false );
  geometry->setUseVertexBufferObjects ( true );
  geode->addDrawable( geometry.get() );

  osg::ref_ptr< osg::AutoTransform > autoTransform ( new osg::AutoTransform );		
  autoTransform->setPosition ( earthLocation );
  autoTransform->setAutoScaleToScreen ( true );
  autoTransform->addChild ( geode.get() );

  // Set the normalize state to true, so when the sphere size changes it still looks correct.
  OsgTools::State::StateSet::setNormalize ( autoTransform.get(), true );

  return autoTransform.release();
}
