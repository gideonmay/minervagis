
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/MultiPoint.h"
#include "Minerva/Core/Data/PointStyle.h"
#include "Minerva/OsgTools/StateSet.h"
#include "Minerva/OsgTools/ConvertVector.h"

#include "Minerva/Common/IElevationDatabase.h"
#include "Minerva/Common/IPlanetCoordinates.h"

#include "Usul/Threads/Safe.h"

#include "osg/Geometry"
#include "osg/Point"
#include "osg/Geode"
#include "osg/MatrixTransform"

using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

MultiPoint::MultiPoint() : BaseClass(),
  _coordinates()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

MultiPoint::~MultiPoint()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the point.
//
///////////////////////////////////////////////////////////////////////////////

void MultiPoint::coordinates ( Coordinates::RefPtr coordinates )
{
  Guard guard ( this->mutex() );
  _coordinates = coordinates;

  if ( _coordinates )
  {
    this->extents ( coordinates->extents() );
  }
  
  // Dirty.
  this->dirty( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the point.
//
///////////////////////////////////////////////////////////////////////////////

MultiPoint::Coordinates::RefPtr MultiPoint::coordinates() const
{
  Guard guard ( this->mutex() );
  return _coordinates;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene branch for the data object.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* MultiPoint::_buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  // Get the point style to use later.
  PointStyle::RefPtr pointStyle ( new PointStyle );
  if ( style && style->pointstyle() )
  {
    pointStyle = style->pointstyle();
  }

  if ( PointStyle::NONE == pointStyle->primitiveId() )
    return 0x0;

  // Get the point data.
  Coordinates::RefPtr data ( this->coordinates() );

  if ( !data || true == data->empty() || !planet )
    return 0x0;
  
  // Make the osg::Vec3Array.
  osg::ref_ptr< osg::Vec3Array > vertices ( new osg::Vec3Array );
  vertices->reserve ( data->size() );

  Coordinates::value_type offset ( data->at ( 0 ) );
  offset[2] = Minerva::Core::Data::getElevationAtPoint ( offset, elevation, this->altitudeMode() );
  planet->convertToPlanet ( Usul::Math::Vec3d ( offset ), offset );
  
  // Convert and move all the points so that the first point starts at (0,0,0).
  for ( Coordinates::const_iterator iter = data->begin(); iter != data->end(); ++iter )
  {
    Coordinates::value_type v ( *iter );

    v[2] = Minerva::Core::Data::getElevationAtPoint ( v, elevation, this->altitudeMode() );
    planet->convertToPlanet ( Usul::Math::Vec3d ( v ), v );

    Coordinates::value_type point ( v - offset );
    vertices->push_back ( osg::Vec3f ( point[0], point[1], point[2] ) );
  }

  // Make the geomtry.
  osg::ref_ptr < osg::Geometry > geometry ( new osg::Geometry );

  geometry->setVertexArray ( vertices.get() );

  osg::ref_ptr < osg::Vec4Array > colors ( new osg::Vec4Array ( data->size() ) );
  osg::Vec4f color ( Usul::Convert::Type<Color,osg::Vec4f>::convert ( pointStyle->color() ) );
  std::fill ( colors->begin(), colors->end(), color );
  geometry->setColorArray ( colors.get() );
  geometry->setColorBinding ( osg::Geometry::BIND_PER_VERTEX );
  
  osg::ref_ptr < osg::Vec3Array > normals ( new osg::Vec3Array ( data->size() ) );
  std::fill ( normals->begin(), normals->end(), osg::Vec3 ( 0.0f, -1.0f, 0.0f ) );
  geometry->setNormalArray ( normals.get() );
  geometry->setNormalBinding ( osg::Geometry::BIND_PER_VERTEX );
  
  osg::ref_ptr < osg::Point > ps ( new osg::Point );
  ps->setSize ( pointStyle->size() );
	
  osg::ref_ptr < osg::StateSet > ss ( geometry->getOrCreateStateSet() );
  ss->setAttributeAndModes( ps.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
  
  // Turn off lighting.
  OsgTools::State::StateSet::setLighting ( ss.get(), false );
  
  geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::POINTS, 0, vertices->size() ) );
  
  // Make the geode.
  osg::ref_ptr< osg::Geode > geode ( new osg::Geode );
  geode->addDrawable ( geometry.get() );
  
  osg::ref_ptr<osg::MatrixTransform> mt ( new osg::MatrixTransform );
  mt->setMatrix ( osg::Matrixd::translate ( offset[0], offset[1], offset[2] ) );
  mt->addChild ( geode.get() );
  
  // Set the render bin.
  ss->setRenderBinDetails ( this->renderBin(), "RenderBin" );
  
  // Return the MatixTransform.
  return mt.release();
}
