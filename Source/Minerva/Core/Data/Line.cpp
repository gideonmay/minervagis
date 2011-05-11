
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/Line.h"
#include "Minerva/Core/Algorithms/Resample.h"

#include "Minerva/OsgTools/StateSet.h"
#include "Minerva/OsgTools/Font.h"

#include "Minerva/Common/IElevationDatabase.h"
#include "Minerva/Common/IPlanetCoordinates.h"

#include "Usul/Components/Manager.h"

#include "osg/BlendFunc"
#include "osg/Geode"
#include "osg/Geometry"
#include "osg/Depth"
#include "osg/Hint"
#include "osg/Material"
#include "osg/MatrixTransform"
#include "osg/Version"

#include <limits>

using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Line::Line() : BaseClass(),
  _coordinates(),
  _tessellate ( false ),
  _useShader ( false )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Line::~Line()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the point.
//
///////////////////////////////////////////////////////////////////////////////

void Line::coordinates ( Coordinates::RefPtr coordinates )
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

Line::Coordinates::RefPtr Line::coordinates() const
{
  Guard guard ( this->mutex() );
  return _coordinates;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene branch for the data object.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Line::_buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  LineStyle::RefPtr lineStyle;
  if ( style )
  {
    lineStyle = style->linestyle();
  }
  
  if ( !lineStyle )
  {
    lineStyle = new LineStyle;
  }

  return this->_buildScene ( lineStyle->color(), lineStyle->width(), planet, elevation );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene branch for the data object.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Line::_buildScene ( const Color& color, float width, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  //Guard guard ( this ); Was causing deadlock!

  // Get the line data.
  Coordinates::RefPtr data ( this->coordinates() );
  
  // Make sure there are at least 2 points.
  if ( !data || true == data->empty() || !planet )
    return 0x0;

  // TODO: Implement fit to ground.
  
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

  // Create the geometry
  osg::ref_ptr< osg::Geometry > geometry ( new osg::Geometry );

  // Add the primitive set
  geometry->addPrimitiveSet( new osg::DrawArrays ( osg::PrimitiveSet::LINE_STRIP, 0, vertices->size() ) );

  // Set the vertices.
  geometry->setVertexArray ( vertices.get() );

  // Display lists seems to be the fastest.
  geometry->setUseDisplayList ( true );
  geometry->setUseVertexBufferObjects ( false );

  if ( false == this->useShader() )
  {
    // Set the color.
    osg::ref_ptr < osg::Vec4Array > colors ( new osg::Vec4Array );
    colors->push_back ( osg::Vec4 ( color[0], color[1], color[2], color[3] ) );
    geometry->setColorArray ( colors.get() );
    geometry->setColorBinding ( osg::Geometry::BIND_OVERALL );
  }
    
  // Make the geode.
  osg::ref_ptr < osg::Geode > geode ( new osg::Geode );
  geode->addDrawable ( geometry.get() );

  // Get the state set.
  osg::ref_ptr < osg::StateSet > ss ( geode->getOrCreateStateSet() );
  
  // Set needed state.
  this->_setState ( ss.get(), color, width );

  // Make the MatrixTransform.
  osg::ref_ptr<osg::MatrixTransform> mt ( new osg::MatrixTransform );
  mt->setMatrix ( osg::Matrixd::translate ( osg::Vec3d ( offset[0], offset[1], offset[2] ) ) );
  mt->addChild ( geode.get() );
  
  return mt.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Simple shaders to work around problems in per-tile lighting issues.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  static const char* VERTEX_SHADER_SOURCE = 
  { 
    "void main(void)\n"
    "{\n"
    "   gl_Position = ftransform();\n"
    "}\n"
  };

  static const char* FRAGMENT_SHADER_SOURCE = 
  {
    "uniform vec4 Color;\n"
    "void main(void)\n"
    "{\n"
    "   gl_FragColor = vec4( Color );\n"
    "}\n"
  };
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set proper state.
//
///////////////////////////////////////////////////////////////////////////////

void Line::_setState ( osg::StateSet* ss, const Color& color, float width ) const
{
  // Return now if state isn't valid.
  if ( 0x0 == ss )
    return;
  
  // Turn off lighting.
  OsgTools::State::StateSet::setLighting  ( ss, false );
  OsgTools::State::StateSet::setLineWidth ( ss, width );
  
  //const osg::StateAttribute::GLModeValue on ( osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON );
  //const osg::StateAttribute::GLModeValue off ( osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF );

  if ( this->useShader() )
  {
    osg::ref_ptr< osg::Program > program ( new osg::Program ); 
    ss->setAttribute ( program.get() );
    ss->addUniform ( new osg::Uniform ( "Color", osg::Vec4 ( color[0], color[1], color[2], color[3] ) ) );

    program->addShader ( new osg::Shader ( osg::Shader::VERTEX, Detail::VERTEX_SHADER_SOURCE ) );
    program->addShader ( new osg::Shader ( osg::Shader::FRAGMENT, Detail::FRAGMENT_SHADER_SOURCE )  );
  }
  else
  {
#if 0
    // Set the line parameters.
    ss->setMode ( GL_LINE_SMOOTH, on );
    ss->setMode ( GL_BLEND, on );
    
    // Add a blend function.
    osg::ref_ptr<osg::BlendFunc> blend ( new osg::BlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
    ss->setAttributeAndModes ( blend.get(), on );
    
    // Set the hint.
    osg::ref_ptr<osg::Hint> hint ( new osg::Hint ( GL_LINE_SMOOTH_HINT, GL_NICEST ) );
    ss->setAttributeAndModes ( hint.get(), on );
#endif
  }

  // Set the render bin.
  ss->setRenderBinDetails ( this->renderBin(), "RenderBin" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set tessellate flag.
//
///////////////////////////////////////////////////////////////////////////////

void Line::tessellate ( bool b )
{
  Guard guard ( this->mutex() );
  _tessellate = b;
  this->dirty ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get tessellate flag.
//
///////////////////////////////////////////////////////////////////////////////

bool Line::tessellate() const
{
  Guard guard ( this->mutex() );
  return _tessellate;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get flag to use a shader.
//
///////////////////////////////////////////////////////////////////////////////

bool Line::useShader() const
{
  Guard guard ( this );
  return _useShader;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set flag to use a shader.
//
///////////////////////////////////////////////////////////////////////////////

void Line::useShader ( bool b )
{
  Guard guard ( this );
  _useShader = b;
}
