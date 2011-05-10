
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class to render to off screen target, without needed a window.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/OsgTools/OffScreenRendererPBuffer.h"

#include "Usul/Bits/Bits.h"
#include "Usul/Errors/Assert.h"

#include "osg/GraphicsContext"
#include "osg/Image"
#include "osg/Texture2D"
#include "osg/Multisample"

#include <iostream>

using namespace Minerva::OsgTools;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

OffScreenRendererPBuffer::OffScreenRendererPBuffer ( unsigned int width, unsigned int height ) : BaseClass(),
  _viewer ( new osgViewer::Viewer ),
  _image ( 0x0 )
{
  _viewer->setThreadingModel ( osgViewer::Viewer::SingleThreaded );
  _viewer->setCameraManipulator ( 0x0 );

  this->_init ( width, height );

  // Make sure everything is set up.
  _viewer->realize();

  // Set a black background.
  _viewer->getCamera()->setClearColor ( osg::Vec4 ( 0.0, 0.0, 0.0, 1.0 ) );

  // Turn off small feature culling.
  _viewer->getCamera()->setCullingMode ( Usul::Bits::remove ( _viewer->getCamera()->getCullingMode(), osg::CullingSet::SMALL_FEATURE_CULLING ) );

  // Turn on multi-sampling.
  osg::ref_ptr<osg::StateSet> ss ( _viewer->getCamera()->getOrCreateStateSet() );
  osg::ref_ptr<osg::Multisample> multiSample ( new osg::Multisample );

  ss->setAttribute ( multiSample.get(), osg::StateAttribute::ON );
  ss->setMode( GL_MULTISAMPLE_ARB, osg::StateAttribute::ON );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

OffScreenRendererPBuffer::~OffScreenRendererPBuffer()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Callback to read the pixels from the buffer.
//
///////////////////////////////////////////////////////////////////////////////

class ReadPixelsCallback : public osg::Camera::DrawCallback
{
public:
  ReadPixelsCallback ( osg::Image* image ) : osg::Camera::DrawCallback(),
    _image ( image )
  {
  }

  virtual void operator () ( osg::RenderInfo& /*renderInfo*/ ) const
  {
    ::glReadBuffer ( GL_BACK );
    _image->readPixels ( 0, 0, _image->s(), _image->t(), GL_RGB, GL_UNSIGNED_BYTE );
  }

private:

  osg::ref_ptr<osg::Image> _image;
};


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize.
//
///////////////////////////////////////////////////////////////////////////////

void OffScreenRendererPBuffer::_init ( unsigned int width, unsigned int height )
{
  // Make the traits for the graphics context we want.
  osg::ref_ptr<osg::GraphicsContext::Traits> traits ( new osg::GraphicsContext::Traits );
  traits->x = 0;
  traits->y = 0;
  traits->width = width;
  traits->height = height;
  traits->red = 8;
  traits->green = 8;
  traits->blue = 8;
  traits->alpha = 8;
  traits->depth = 24;
  traits->windowDecoration = false;
  traits->pbuffer = true; // We want a pixel buffer.
  traits->doubleBuffer = true;
  traits->sharedContext = 0x0;
  traits->samples = 4;

  // Make the graphics context.
  osg::ref_ptr<osg::GraphicsContext> gc ( osg::GraphicsContext::createGraphicsContext ( traits.get() ) );

  osg::ref_ptr<osg::Camera> camera ( new osg::Camera );
  camera->setDrawBuffer ( GL_BACK );
  camera->setReadBuffer ( GL_BACK );
  camera->setClearMask ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  camera->setCullingMode ( Usul::Bits::remove ( camera->getCullingMode(), osg::CullingSet::SMALL_FEATURE_CULLING ) );

  osg::ref_ptr<osg::StateSet> ss ( camera->getOrCreateStateSet() );
  osg::ref_ptr<osg::Multisample> multiSample ( new osg::Multisample );

  ss->setAttribute ( multiSample.get(), osg::StateAttribute::ON );
  ss->setMode( GL_MULTISAMPLE_ARB, osg::StateAttribute::ON );

  _viewer->addSlave ( camera.get() );

  // Set the graphic convert.
  camera->setGraphicsContext ( gc.get() );

  // Set the size.
  const double fovy   ( 45.0 );
  const double zNear  ( 0.01f );
  const double zFar   ( 100.0 );
  const double w      ( width ), h ( height );
  const double aspect ( w / h );

  _viewer->getCamera()->setViewport ( 0, 0, w, h );
  _viewer->getCamera()->setProjectionMatrixAsPerspective ( fovy, aspect, zNear, zFar );

  camera->setViewport ( new osg::Viewport  ( 0 ,0, w, h ) );
  camera->setProjectionMatrixAsPerspective ( fovy, aspect, zNear, zFar );

  _image = new osg::Image;
  _image->allocateImage ( width, height, 1, GL_RGB, GL_UNSIGNED_BYTE );

  camera->setFinalDrawCallback ( new ReadPixelsCallback ( _image.get() ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the model.
//
///////////////////////////////////////////////////////////////////////////////

void OffScreenRendererPBuffer::scene ( osg::Node* node )
{
  USUL_ASSERT ( _viewer.valid() );
  _viewer->setSceneData ( node );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Render.
//
///////////////////////////////////////////////////////////////////////////////

OffScreenRendererPBuffer::ImagePtr OffScreenRendererPBuffer::render() const
{
  USUL_ASSERT ( _viewer.valid() );
  USUL_ASSERT ( _image );

  // Render the frame.
  _viewer->frame();

  return ImagePtr ( reinterpret_cast<osg::Image*> ( _image->clone ( osg::CopyOp::DEEP_COPY_ALL ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set view matrix.
//
///////////////////////////////////////////////////////////////////////////////

void OffScreenRendererPBuffer::viewMatrix ( const Usul::Math::Matrix44d& matrix )
{
  if ( _viewer.valid() )
  {
    osg::Matrixd m ( &matrix[0] );
    _viewer->getCamera()->setViewMatrix ( m );
  }
}
