
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Base class for all geometries
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/Geometry.h"
#include "Minerva/Core/Data/LineStyle.h"
#include "Minerva/Core/Data/PolyStyle.h"

#include "Minerva/OsgTools/SortBackToFront.h"
#include "Minerva/OsgTools/ConvertToTriangles.h"

#include "osg/BlendFunc"
#include "osg/Node"
#include "osg/StateSet"

using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Geometry::Geometry() : 
  BaseClass(),
  _altitudeMode ( ALTITUDE_MODE_CLAMP_TO_GROUND ),
  _dirty ( false ),
  _extrude ( false ),
  _renderBin ( osg::StateSet::DEFAULT_BIN ),
  _extents(),
  _elevationChangedListeners()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Geometry::~Geometry()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the altitude mode.
//
///////////////////////////////////////////////////////////////////////////////

void Geometry::altitudeMode ( AltitudeMode mode )
{
  Guard guard ( this );
  _altitudeMode = mode;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the altitude mode.
//
///////////////////////////////////////////////////////////////////////////////

AltitudeMode Geometry::altitudeMode () const
{
  Guard guard ( this );
  return _altitudeMode;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene branch.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Geometry::buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  osg::ref_ptr<osg::Node> node ( this->_buildScene ( style, planet, elevation ) );
  
  if ( node.valid() )
  {
    osg::ref_ptr < osg::StateSet > ss ( node->getOrCreateStateSet () );

    // Set the render bin.
    ss->setRenderBinDetails ( this->renderBin(), "RenderBin" );
    
    const unsigned int on ( osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    const unsigned int off ( osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    
    const unsigned int blendMode ( Geometry::isSemiTransparent ( style ) ? on : off );
    
    ss->setMode ( GL_BLEND, blendMode );
    
    // Set render bin depending on alpha value.
    if( true == Geometry::isSemiTransparent ( style ) )
    {
      // Add a blend function.
      osg::ref_ptr<osg::BlendFunc> blend ( new osg::BlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
      ss->setAttributeAndModes ( blend.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON );

      ss->setRenderingHint ( osg::StateSet::TRANSPARENT_BIN );
      ss->setRenderBinDetails ( osg::StateSet::TRANSPARENT_BIN, "DepthSortedBin" );
      
      // Convert tri-strips to triangles (For sorting).
      Minerva::OsgTools::ConvertToTriangles convert;
      convert ( node.get() );
      
      osg::ref_ptr<osg::NodeVisitor> visitor ( new ::OsgTools::Callbacks::SetSortToFrontCallback );
      node->accept ( *visitor );
    }
  }
  
  this->dirty( false );
  
  return node.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the render bin.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int Geometry::renderBin() const
{
  Guard guard ( this );
  return _renderBin;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Set extrude flag.
//
///////////////////////////////////////////////////////////////////////////////

void Geometry::extrude ( bool b )
{
  Guard guard ( this );
  _extrude = b;
  this->dirty ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get extrude flag.
//
///////////////////////////////////////////////////////////////////////////////

bool Geometry::extrude() const
{
  Guard guard ( this );
  return _extrude;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the extents.
//
///////////////////////////////////////////////////////////////////////////////

void Geometry::extents ( const Extents& e )
{
  Guard guard ( this->mutex() );
  _extents = e;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extents.
//
///////////////////////////////////////////////////////////////////////////////

Geometry::Extents Geometry::extents() const
{
  Guard guard ( this->mutex() );
  return _extents;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the render bin.
//
///////////////////////////////////////////////////////////////////////////////

void Geometry::renderBin ( unsigned int renderBin )
{
  Guard guard ( this );
  
  // Only change it if it's different.
  if ( renderBin != _renderBin )
  {
    _renderBin = renderBin;
    this->dirty( true );
  }
}

///////////////////////////////////////////////////////////////////////////////
//
//  Is this data object transparent?
//
///////////////////////////////////////////////////////////////////////////////

bool Geometry::isSemiTransparent ( Style::RefPtr style )
{
  if ( style )
  {
    LineStyle::RefPtr lineStyle ( style->linestyle() );
    PolyStyle::RefPtr polyStyle ( style->polystyle() );

    const bool lineSemiTransparent ( lineStyle && lineStyle->color()[3] < 1.0 );
    const bool polySemiTransparent ( polyStyle && polyStyle->color()[3] < 1.0 );

    return lineSemiTransparent || polySemiTransparent;
  }
  return false;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the dirty flag.
//
///////////////////////////////////////////////////////////////////////////////

void Geometry::dirty ( bool b )
{
  Guard guard ( this->mutex() );
  _dirty = b;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the dirty flag.
//
///////////////////////////////////////////////////////////////////////////////

bool Geometry::dirty() const
{
  Guard guard ( this->mutex() );
  return _dirty;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Elevation has changed within given extents.
//
///////////////////////////////////////////////////////////////////////////////

void Geometry::elevationChangedNotify ( const Extents& extents, 
                                        unsigned int level, 
                                        ElevationDataPtr elevationData, 
                                        IPlanetCoordinates *planet, 
                                        IElevationDatabase* elevation )
{
  _elevationChangedListeners ( extents, level, elevationData, planet, elevation );
}
