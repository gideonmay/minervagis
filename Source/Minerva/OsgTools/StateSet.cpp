///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV and John K. Grant
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class for working with the state.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/OsgTools/StateSet.h"

#include "Usul/Adaptors/Random.h"
#include "Usul/Bits/Bits.h"

#include "osg/Node"
#include "osg/StateSet"
#include "osg/PolygonMode"
#include "osg/PolygonOffset"
#include "osg/ShadeModel"
#include "osg/LightModel"
#include "osg/LineWidth"
#include "osg/Point"
#include "osg/Material"

#include <stdexcept>
#include <iostream>

using namespace OsgTools;
using namespace OsgTools::State;


///////////////////////////////////////////////////////////////////////////////
//
//  Set the lighting state.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setLighting ( osg::StateSet *ss, bool state )
{
  // Handle bad input
  if ( 0x0 == ss )
    return;

  // Apply the mode settings
  ss->setMode ( GL_LIGHTING, ( ( state ) ? osg::StateAttribute::ON : osg::StateAttribute::OFF ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the lighting state.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setLighting ( osg::Node *node, bool state )
{
  StateSet::setLighting ( ( node ) ? node->getOrCreateStateSet() : 0x0, state );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the normalization state.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setNormalize ( osg::Node *node, bool on )
{
  // Handle bad input.
  if ( 0x0 == node )
    return;

  // Get the state set, or make one.
  osg::ref_ptr<osg::StateSet> ss ( node->getOrCreateStateSet() );

  // Set the mode.
  ss->setMode ( GL_NORMALIZE, ( ( on ) ? osg::StateAttribute::ON : osg::StateAttribute::OFF ) | osg::StateAttribute::PROTECTED );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the normalization state.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setNormalize ( osg::StateSet *ss, bool on )
{
  // Handle bad input.
  if ( 0x0 == ss )
    return;

  // Set the mode.
  ss->setMode ( GL_NORMALIZE, ( ( on ) ? osg::StateAttribute::ON : osg::StateAttribute::OFF ) | osg::StateAttribute::PROTECTED );
}



///////////////////////////////////////////////////////////////////////////////
//
//  Set the Texture state
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setPolygonsTextures ( osg::StateSet* ss, bool on )
{
  if ( 0x0 == ss )
    return;

  if ( on ) // Turn on textures.
  {
    ss->setTextureMode ( 0, GL_TEXTURE_2D, osg::StateAttribute::INHERIT | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON );
  }

  else
  {
    ss->setTextureMode( 0, GL_TEXTURE_2D, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the two sided lighting state.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setTwoSidedLighting ( osg::StateSet *ss, bool state )
{
  if ( 0x0 == ss )
    return;

  // Initialize.
  osg::ref_ptr<osg::LightModel> lm ( 0x0 );

  // Get the light-model attribute, if any.
  osg::StateAttribute *sa = ss->getAttribute ( osg::StateAttribute::LIGHTMODEL );
  if ( 0x0 == sa )
    lm = new osg::LightModel;
  else
    lm = dynamic_cast < osg::LightModel * > ( sa );

  lm->setTwoSided( state );

  // Set the state. Make it override any other similar states.
  typedef osg::StateAttribute Attribute;
  ss->setAttributeAndModes ( lm.get(), Attribute::OVERRIDE | Attribute::ON | Attribute::PROTECTED );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the two sided lighting state.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setTwoSidedLighting ( osg::Node *node, bool state )
{
  StateSet::setTwoSidedLighting ( ( node ) ? node->getOrCreateStateSet() : 0x0, state );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to set the line width.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  void setLineWidth ( osg::StateSet* ss, float width )
  {
    osg::ref_ptr<osg::LineWidth> lw ( new osg::LineWidth );
    lw->setWidth( width );

    // Set the state. Make it override any other similar states.
    typedef osg::StateAttribute Attribute;
    ss->setAttributeAndModes ( lw.get(), Attribute::OVERRIDE | Attribute::ON );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set line width.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setLineWidth ( osg::Node *node, float width )
{
  StateSet::setLineWidth ( ( ( node ) ? node->getOrCreateStateSet() : 0x0 ), width );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set line width.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setLineWidth ( osg::StateSet *ss, float width )
{
  Helper::setLineWidth ( ss, width );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to set the point size.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  void setPointSize ( osg::StateSet* ss, float size )
  {
    osg::ref_ptr<osg::Point> pt ( new osg::Point );
    pt->setSize ( size );

    // Set the state. Make it override any other similar states.
    typedef osg::StateAttribute Attribute;
    ss->setAttributeAndModes ( pt.get(), Attribute::OVERRIDE | Attribute::ON );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set point size.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setPointSize ( osg::Node *node, float size )
{
  StateSet::setPointSize ( ( ( node ) ? node->getOrCreateStateSet() : 0x0 ), size );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set point size.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setPointSize ( osg::StateSet *ss, float size )
{
  Helper::setPointSize ( ss, size );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set a random material.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setMaterialRandom ( osg::Node *node )
{
  // Handle bad input.
  if ( 0x0 == node )
    return;

  osg::Vec4 emissive ( 0.0f, 0.0f, 0.0f, 1.0f );
  osg::Vec4 specular ( 0.2f, 0.2f, 0.2f, 1.0f );

  Usul::Adaptors::Random < float > rd ( 0.2f, 1.0f );

  osg::Vec4 diffuse ( rd(), rd(), rd(), 1.0f );
  osg::Vec4 ambient ( diffuse );

  osg::ref_ptr<osg::Material> mat ( new osg::Material );
  mat->setAmbient   ( osg::Material::FRONT_AND_BACK, ambient  );
  mat->setDiffuse   ( osg::Material::FRONT_AND_BACK, diffuse  );
  mat->setEmission  ( osg::Material::FRONT_AND_BACK, emissive );
  mat->setSpecular  ( osg::Material::FRONT_AND_BACK, specular );
  mat->setShininess ( osg::Material::FRONT_AND_BACK, 100      );

  StateSet::setMaterial ( node, mat.get() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get a default material.
//
///////////////////////////////////////////////////////////////////////////////

osg::Material *StateSet::getMaterialDefault()
{
  osg::ref_ptr<osg::Material> mat ( new osg::Material );
  mat->setAmbient   ( osg::Material::FRONT_AND_BACK, osg::Vec4 ( 1.0f, 1.0f, 1.0f, 1.0f ) );
  mat->setDiffuse   ( osg::Material::FRONT_AND_BACK, osg::Vec4 ( 1.0f, 1.0f, 1.0f, 1.0f ) );
  mat->setEmission  ( osg::Material::FRONT_AND_BACK, osg::Vec4 ( 0.0f, 0.0f, 0.0f, 1.0f ) );
  mat->setSpecular  ( osg::Material::FRONT_AND_BACK, osg::Vec4 ( 0.2f, 0.2f, 0.2f, 1.0f ) );
  mat->setShininess ( osg::Material::FRONT_AND_BACK, 100 );
  return mat.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set a default material.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setMaterialDefault ( osg::Node *node )
{
  // Handle bad input.
  if ( 0x0 == node )
    return;

  // Pass through for backward compatability.
  StateSet::setMaterial ( node, OsgTools::State::StateSet::getMaterialDefault() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the material.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setMaterial ( osg::Node *node, osg::Material *mat )
{
  // Handle bad input.
  if ( 0x0 == node || 0x0 == mat )
    return;

  OsgTools::State::StateSet::setMaterial ( node->getOrCreateStateSet(), mat );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the material.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setMaterial ( osg::StateSet *ss, osg::Material *mat )
{
  // Handle bad input.
  if ( 0x0 == ss || 0x0 == mat )
    return;

  ss->setAttributeAndModes ( mat, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the material.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setMaterial ( osg::Node *node, const osg::Vec4f &ambient, const osg::Vec4f &diffuse, float alpha )
{
  // Handle bad input.
  if ( 0x0 == node )
    return;

  osg::ref_ptr<osg::Material> mat ( OsgTools::State::StateSet::getMaterialDefault() );
  if ( false == mat.valid() )
    return;

  mat->setAmbient ( osg::Material::FRONT_AND_BACK, ambient );
  mat->setDiffuse ( osg::Material::FRONT_AND_BACK, diffuse );
  //mat->setSpecular ( osg::Material::FRONT_AND_BACK, osg::Vec4 ( 0.2, 0.2, 0.2, 1.0 ) );
  //mat->setShininess ( osg::Material::FRONT_AND_BACK, 20 );

  if ( alpha < 1 )
  {
    node->getOrCreateStateSet()->setMode ( GL_BLEND, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON );
    node->getOrCreateStateSet()->setRenderingHint ( osg::StateSet::TRANSPARENT_BIN );
    mat->setAlpha ( osg::Material::FRONT_AND_BACK, alpha );
  }

  OsgTools::State::StateSet::setMaterial ( node, mat.get() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the alpha. Add default material if needed.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setAlpha ( osg::Node *node, float alpha )
{
  // Handle bad input.
  if ( 0x0 == node )
    return;

  osg::ref_ptr<osg::StateSet> ss ( node->getOrCreateStateSet() );
  OsgTools::State::StateSet::setAlpha ( ss.get(), alpha );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the alpha. Add default material if needed.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::setAlpha ( osg::StateSet *ss, float alpha )
{
  // Handle bad input.
  if ( 0x0 == ss )
    return;

  // Get the material attribute.
  osg::ref_ptr<osg::Material> mat ( dynamic_cast < osg::Material * > ( ss->getAttribute ( osg::StateAttribute::MATERIAL ) ) );

  // Set a default material if there isn't one.
  if ( false == mat.valid() )
  {
    OsgTools::State::StateSet::setMaterial ( ss, OsgTools::State::StateSet::getMaterialDefault() );
    mat = dynamic_cast < osg::Material * > ( ss->getAttribute ( osg::StateAttribute::MATERIAL ) );
  }

  // Check.
  if ( false == mat.valid() )
    return;

  // Set these other properties if it's not completely opaque.
  if ( alpha < 1 )
  {
    ss->setMode ( GL_BLEND, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON );
    ss->setRenderingHint ( osg::StateSet::TRANSPARENT_BIN );
  }

  // Set the alpha.
  mat->setAlpha ( osg::Material::FRONT_AND_BACK, alpha );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove the material, if there is any.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::removeMaterial ( osg::Node *node )
{
  if ( 0x0 != node )
  {
    OsgTools::State::StateSet::removeMaterial ( node->getOrCreateStateSet() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove the material, if there is any.
//
///////////////////////////////////////////////////////////////////////////////

void StateSet::removeMaterial ( osg::StateSet *ss )
{
  // Handle bad input
  if ( 0x0 == ss )
    return;

  // Remove the material.
  ss->removeAttribute ( osg::StateAttribute::MATERIAL );
}
