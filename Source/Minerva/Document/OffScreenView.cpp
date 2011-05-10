
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  View.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Document/OffScreenView.h"
#include "Minerva/OsgTools/OffScreenRendererPBuffer.h"
#include "Usul/Errors/Assert.h"
#include "Usul/System/Sleep.h"

#include "osgDB/WriteFile"

using namespace Minerva::Document;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

OffScreenView::OffScreenView ( Document::RefPtr document, unsigned int width, unsigned int height ) : BaseClass ( document ),
  _renderer ( new Minerva::OsgTools::OffScreenRendererPBuffer ( width, height ) ),
  _frameDump ( new FrameDump )
{
  if ( !document )
  {
    throw std::runtime_error ( "View requires valid document" );
  }

  _renderer->scene ( document->buildScene() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

OffScreenView::OffScreenView ( Document::RefPtr document, Minerva::OsgTools::OffScreenRenderer::RefPtr renderer ) : BaseClass ( document ),
  _renderer ( renderer ),
  _frameDump ( new FrameDump )
{
  if ( !document )
  {
    throw std::runtime_error ( "View requires valid document" );
  }

  if ( !_renderer )
  {
    throw std::runtime_error ( "View requires a valid renderer." );
  }

  _renderer->scene ( document->buildScene() );
}



///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

OffScreenView::~OffScreenView()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set frame dump properties.
//
///////////////////////////////////////////////////////////////////////////////

void OffScreenView::frameDumpProperties ( const std::string &dir,
                                 const std::string &base,
                                 const std::string &ext,
                                 unsigned int start )
{
  _frameDump->dir ( dir );
  _frameDump->base ( base );
  _frameDump->ext ( ext );
  _frameDump->start ( start );
  _frameDump->digits ( 9 );
  _frameDump->reset();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Render.
//
///////////////////////////////////////////////////////////////////////////////

void OffScreenView::render ( Minerva::Core::Data::Camera::RefPtr camera )
{
  this->_render ( camera );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Render.
//
///////////////////////////////////////////////////////////////////////////////

void OffScreenView::_render ( Minerva::Core::Data::Camera::RefPtr camera, bool dumpFrame )
{
  if ( camera )
  {
    Document::RefPtr document ( this->document() );

    Usul::Math::Matrix44d matrix ( camera->viewMatrix ( document->body()->landModel() ) );
    matrix.invert();
    _renderer->viewMatrix ( matrix );

    document->body()->needsRedraw ( false );

    document->updateNotify ( camera );

    typedef Minerva::OsgTools::OffScreenRenderer::ImagePtr ImagePtr;
    ImagePtr image ( _renderer->render() );

    if ( image && dumpFrame )
    {
      USUL_ASSERT ( _frameDump );
      osgDB::writeImageFile ( *image, _frameDump->file() );
    }

    document->postRenderNotify();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Render until all jobs are finished.
//
///////////////////////////////////////////////////////////////////////////////

void OffScreenView::waitForDetail ( Minerva::Core::Data::Camera::RefPtr camera )
{
  Document::RefPtr document ( this->document() );
  do
  {
    this->_render ( camera, false );
    Usul::System::Sleep::seconds ( 1 );
  } while ( document->busyStateGet() || document->body()->needsRedraw() );
}
