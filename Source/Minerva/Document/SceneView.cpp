
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  Copyright (c) 2005, Perry L. Miller IV and Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  View class that wraps osgUtil::SceneView.
//
///////////////////////////////////////////////////////////////////////////////

// Disable deprecated warning in Visual Studio 8 
#if defined ( _MSC_VER ) && _MSC_VER == 1400
#pragma warning ( disable : 4996 )
#endif

#include "Minerva/Document/SceneView.h"
#include "Minerva/OsgTools/Defaults.h"
#include "Minerva/OsgTools/FBOScreenCapture.h"
#include "Minerva/OsgTools/TiledScreenCapture.h"

#include "Usul/Bits/Bits.h"
#include "Usul/Cast/Cast.h"
#include "Usul/Components/Manager.h"
#include "Usul/Convert/Convert.h"
#include "Usul/Convert/Vector2.h"
#include "Usul/Errors/Checker.h"
#include "Usul/Math/Absolute.h"
#include "Usul/Math/Constants.h"
#include "Usul/Math/Functions.h"
#include "Usul/Math/NaN.h"
#include "Usul/Registry/Constants.h"
#include "Usul/Registry/Database.h"
#include "Usul/Threads/Named.h"

#include "osgUtil/CullVisitor"
#include "osgUtil/UpdateVisitor"

#include "osgDB/WriteFile"
#include "osgDB/ReadFile"

#include "osg/Material"
#include "osg/LightModel"
#include "osg/Notify"
#include "osg/Version"
#include "osg/GL"
#include "osg/GLObjects"

#include "boost/bind.hpp"

// The OpenSceneGraph trunk no longer includes glu header.
#ifdef __APPLE__
#include "OpenGL/glu.h"
#else
#include "GL/glu.h"
#endif

#include <limits>

using namespace Minerva::Document;

namespace Sections = Usul::Registry::Sections;
namespace Keys = Usul::Registry::Keys;
typedef Usul::Registry::Database Reg;


///////////////////////////////////////////////////////////////////////////////
//
//  NaN convienence function.
//
///////////////////////////////////////////////////////////////////////////////

USUL_DECLARE_NAN_VEC3 ( osg::Vec3f );
USUL_DECLARE_NAN_VEC3 ( osg::Vec3d );


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to check for errors.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  void checkForErrors ( unsigned int id )
  {
    GLenum error = ::glGetError();
    if ( GL_NO_ERROR != error )
    {
      std::cout << "Error " << id << ": OpenGL error detected." << std::endl
        << "Message: " << ::gluErrorString ( error ) << std::endl;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

SceneView::SceneView ( Document::RefPtr document ) :
  BaseClass            ( document ),
  _sceneView           ( new osgUtil::SceneView ),
  _framestamp          ( new osg::FrameStamp ),
  _timer               (),
  _start_tick          ( 0 ),
  _frameDump           (),
  _contextId           ( 0 )
{
  // Unique context id to identify this SceneView in OSG.
  static unsigned int count ( 0 );
  _contextId = ++count;

  this->_initSceneView();

  _sceneView->setSceneData ( this->scene() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

SceneView::~SceneView()
{
  // Better be zero
  USUL_ASSERT ( 0 == this->refCount() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear the SceneView.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::clear()
{ 
  // Clear the scene view.
  _sceneView->releaseAllGLObjects();
  _sceneView->flushAllDeletedGLObjects();
  _sceneView->setSceneData ( 0x0 );

  // Flush all delete objects for this context.
  osg::flushAllDeletedGLObjects ( _contextId );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Render the scene.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::render ( Minerva::Core::Data::Camera::RefPtr camera )
{
  // Handle no SceneView or scene.
  if ( !this->viewer() || !this->viewer()->getSceneData() )
    return;

  this->_updateHUD ( camera );

  // Initialize the error.
  ::glGetError();

  // Check for errors.
  Detail::checkForErrors ( 1491085606 );

  // Update.
  {
    // Handle particles and osg-animations.
    _framestamp->setFrameNumber ( _framestamp->getFrameNumber() + 1 );
    _framestamp->setReferenceTime ( _timer.delta_s( _start_tick, _timer.tick() ) );
    _framestamp->setSimulationTime ( _framestamp->getReferenceTime () );

    _sceneView->getUpdateVisitor()->setTraversalNumber ( _framestamp->getFrameNumber() );

    _sceneView->update();
  }
  
  // Cull and draw.
  _sceneView->cull();
  _sceneView->draw();

  // Check for errors.
  Detail::checkForErrors ( 840649560 );

  // Flush deleted GL objects.
  this->viewer()->flushAllDeletedGLObjects();
  
  // Check for errors.
  Detail::checkForErrors ( 896118310 );

  // Dump the current frame.
  this->_dumpFrame();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the SceneView.
//
///////////////////////////////////////////////////////////////////////////////

osgUtil::SceneView *SceneView::viewer()
{
  return _sceneView.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the SceneView.
//
///////////////////////////////////////////////////////////////////////////////

const osgUtil::SceneView *SceneView::viewer() const
{
  return _sceneView.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Resize the window.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::resize ( unsigned int w, unsigned int h )
{
  BaseClass::resize ( w, h );

  USUL_ERROR_CHECKER ( w < (unsigned int) std::numeric_limits<int>::max() );
  USUL_ERROR_CHECKER ( h < (unsigned int) std::numeric_limits<int>::max() );

  // Ignore initial values sent; they make a poor viewport.
  if ( w <= 1 || h <= 1 )
    return;

  // Set the viewport.
  _sceneView->setViewport ( 0, 0, (int) w, (int) h );

  // Set the SceneView's projection matrix.
  const double fovy  ( Minerva::OsgTools::Defaults::CAMERA_FOV_Y );
  double zNear ( Minerva::OsgTools::Defaults::CAMERA_Z_NEAR );
  double zFar  ( Minerva::OsgTools::Defaults::CAMERA_Z_FAR );
  double width ( w ), height ( h );
  double aspect ( width / height );
  if ( this->viewer() )
    this->viewer()->setProjectionMatrixAsPerspective ( fovy, aspect, zNear, zFar );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle the polygon mode.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::togglePolygonMode ( osg::PolygonMode::Mode mode )
{
  if ( this->hasPolygonMode ( mode ) )
    this->removePolygonMode();
  else
    this->setPolygonMode ( mode );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the polygon mode.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::setPolygonMode ( osg::PolygonMode::Mode mode )
{
  // Handle no SceneView.
  if ( !this->viewer() )
    return;

  // Get the state set, or make one.
  osg::ref_ptr<osg::StateSet> ss ( this->viewer()->getGlobalStateSet() );
  if ( !ss.valid() )
    return;

  // Make a polygon-mode.
  osg::ref_ptr<osg::PolygonMode> pm ( new osg::PolygonMode );
  pm->setMode ( osg::PolygonMode::FRONT_AND_BACK, mode );

  // Set the state. Make it override any other similar states.
  typedef osg::StateAttribute Attribute;
  ss->setAttributeAndModes ( pm.get(), Attribute::OVERRIDE | Attribute::ON );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query the polygon mode.
//
///////////////////////////////////////////////////////////////////////////////

bool SceneView::hasPolygonMode ( osg::PolygonMode::Mode mode ) const
{
  // Handle no scene.
  if ( !this->scene() )
    return false;

  // Get the state set.
  const osg::StateSet *ss = this->viewer()->getGlobalStateSet();
  if ( !ss )
    return false;

  // Get the polygon-mode attribute, if any.
  const osg::StateAttribute *sa = ss->getAttribute ( osg::StateAttribute::POLYGONMODE );
  if ( !sa )
    return false;

  // Should be true.
  const osg::PolygonMode *pm = dynamic_cast < const osg::PolygonMode * > ( sa );
  if ( !pm )
    return false;

  // Is the mode the same?
  return ( pm->getMode ( osg::PolygonMode::FRONT_AND_BACK ) == mode );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove the polygon mode.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::removePolygonMode()
{
  // Get the state set.
  osg::StateSet *ss = _sceneView->getGlobalStateSet();

  // Have the state-set inherit the attribute. This will delete any 
  // existing attribute in the state-set.
  ss->removeAttribute ( osg::StateAttribute::POLYGONMODE );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set two sided lighting
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::twoSidedLightingSet ( bool twoSided )
{
  // Get the state set, or make one.
  osg::ref_ptr<osg::StateSet> ss ( _sceneView->getGlobalStateSet() );

  // Make a light-model.
  osg::ref_ptr<osg::LightModel> lm ( new osg::LightModel );
  lm->setTwoSided( twoSided );

  // Set the state. Make it override any other similar states.
  typedef osg::StateAttribute Attribute;
  ss->setAttributeAndModes ( lm.get(), Attribute::OVERRIDE | Attribute::ON );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get two sided lighting
//
///////////////////////////////////////////////////////////////////////////////

bool SceneView::twoSidedLightingGet() const
{
  // Get the state set.
  const osg::StateSet *ss = _sceneView->getGlobalStateSet();

  // Get the attribute, if any.
  const osg::StateAttribute *sa = ss->getAttribute ( osg::StateAttribute::LIGHTMODEL );
  if ( !sa )
    return false;

  // Should be true.
  const osg::LightModel *lm = dynamic_cast < const osg::LightModel * > ( sa );
  if ( !lm )
    return false;

  //Is two sided light on?
  return ( lm->getTwoSided() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Dump the current frame to file.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::_dumpFrame()
{
  if ( FrameDump::NEVER_DUMP == _frameDump.dump() )
  {
    return;
  }

  this->writeImageFile ( _frameDump.file() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Write the current frame to an image file.
//
///////////////////////////////////////////////////////////////////////////////

bool SceneView::writeImageFile ( const std::string &filename ) const
{
  // Make the image
  osg::ref_ptr<osg::Image> image ( new osg::Image );

  // Read the screen buffer.
  image->readPixels ( 0, 0, static_cast<int> ( this->_width() ), static_cast<int> ( this->_height() ), GL_RGB, GL_UNSIGNED_BYTE );

  // Write the image.
  return osgDB::writeImageFile ( *image, filename );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the view matrix.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::setViewMatrix ( const osg::Matrixd& matrix )
{
  this->viewer()->setViewMatrix ( matrix );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the view matrix.
//
///////////////////////////////////////////////////////////////////////////////

osg::Matrixd SceneView::getViewMatrix() const
{
  return this->viewer()->getViewMatrix();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set properties for dumping frames.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::frameDumpProperties ( const std::string &dir,
                                   const std::string &base, 
                                   const std::string &ext, 
                                   unsigned int start, 
                                   unsigned int digits )
{
  const FrameDump::DumpState dump ( this->getFrameDumpState() );
  FrameDump fd ( dump, dir, base, ext, start, digits );
  this->frameDump ( fd );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Turn on/off frame-dumping.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::setFrameDumpState ( FrameDump::DumpState state )
{
  _frameDump.dump ( state );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Are we dumping frames?
//
///////////////////////////////////////////////////////////////////////////////

Minerva::OsgTools::FrameDump::DumpState SceneView::getFrameDumpState() const
{
  return _frameDump.dump();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the line segment into the scene that goes through the 2D coordinate.
//
///////////////////////////////////////////////////////////////////////////////

bool SceneView::_lineSegment ( float x, float y, osg::Vec3d &pt0, osg::Vec3d &pt1 )
{
  // Handle no SceneView.
  if ( !this->viewer() )
    return false;

  // Project into the scene.
  {
    // Get the matrix.
    osg::Matrix M ( this->viewer()->getViewMatrix() );
    osg::Matrix P ( this->viewer()->getProjectionMatrix() );
    osg::Matrixd W ( this->viewer()->getViewport()->computeWindowMatrix() );
    osg::Matrix MPW ( M * P * W );
    osg::Matrix IMPW ( osg::Matrix::inverse ( MPW ) );

    // Calculate the two points for our line-segment.
    pt0 = osg::Vec3d ( x, y, -1 ) * IMPW;
    pt1 = osg::Vec3d ( x, y,  1 ) * IMPW;
  }

  // Are the numbers valid?
  const bool valid ( ( false == Usul::Math::nan ( pt0 ) ) && ( false == Usul::Math::nan ( pt1 ) ) );

  // It worked if the values are valid.
  return valid;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Intersect the scene.
//
///////////////////////////////////////////////////////////////////////////////

bool SceneView::_intersect ( float x, float y, osg::Node *scene, osgUtil::LineSegmentIntersector::Intersections &intersections )
{
  // Handle no scene or SceneView.
  if ( !scene || !this->viewer() )
    return false;
  
  // Calculate the two points for our line-segment.
  osg::Vec3d pt0, pt1;
  if ( !this->_lineSegment ( x, y, pt0, pt1 ) )
    return false;
  
  // Make the intersector.
  osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector ( new osgUtil::LineSegmentIntersector ( pt0, pt1 ) );
  
  // Declare the pick-visitor.
  typedef osgUtil::IntersectionVisitor Visitor;
  osg::ref_ptr<Visitor> visitor ( new Visitor );
  visitor->setIntersector ( intersector.get() );
  
  // Intersect the scene.
  scene->accept ( *visitor );
  
  // Get the hit-list for our line-segment.
  intersections = intersector->getIntersections();
  if ( intersections.empty() )
    return false;
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//  See if event intersects the current scene
//
///////////////////////////////////////////////////////////////////////////////

bool SceneView::intersect ( float x, float y, osgUtil::LineSegmentIntersector::Intersections &intersections )
{
  return this->_intersect ( x, y, this->scene(), intersections );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the back face culling state.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::showBackFaces ( bool state )
{
  // Handle no SceneView.
  if ( !this->viewer() )
    return;
  
  // Set the state.
  osg::ref_ptr<osg::StateSet> ss ( this->viewer()->getGlobalStateSet() );
  ss->setMode ( GL_CULL_FACE, ( ( state ) ? osg::StateAttribute::ON : osg::StateAttribute::OFF ) | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the back face culling state.
//
///////////////////////////////////////////////////////////////////////////////

bool SceneView::isBackFacesShowing() const
{
  // Return the state.
  const osg::StateSet * ss ( this->viewer()->getGlobalStateSet() );
  return ( ( 0x0 == this->viewer() ) ? false : !Usul::Bits::has ( ss->getMode ( GL_CULL_FACE ), osg::StateAttribute::ON ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the projection matrix.
//
///////////////////////////////////////////////////////////////////////////////

osg::Matrixd SceneView::getProjectionMatrix() const
{
  const osgUtil::SceneView * viewer ( this->viewer() );
  return ( 0x0 != viewer ? viewer->getProjectionMatrix() : osg::Matrixd() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set near far ratio.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::nearFarRatio ( double value )
{
  osgUtil::SceneView * viewer ( this->viewer() );
  if ( viewer != 0x0 )
  {
    viewer->setNearFarRatio ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get near far ratio.
//
///////////////////////////////////////////////////////////////////////////////

double SceneView::nearFarRatio() const
{
  const osgUtil::SceneView * viewer ( this->viewer() );
  return ( 0x0 != viewer ? viewer->getNearFarRatio() : 1.0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize the scene view.
//
///////////////////////////////////////////////////////////////////////////////

void SceneView::_initSceneView()
{
  // Set the start tick.
  _start_tick = _timer.tick();

  _sceneView->setFrameStamp ( _framestamp.get() );

  // Set the update-visitor.
  _sceneView->setUpdateVisitor ( new osgUtil::UpdateVisitor );

  // Set the display settings.
  _sceneView->setDisplaySettings ( new osg::DisplaySettings );

  // Set the SceneView's defaults.
  _sceneView->setDefaults();
  
  _sceneView->init();
  
  // Set the background to black.
  _sceneView->setClearColor ( osg::Vec4 (  0.0, 0.0, 0.0, 1.0 ) );

  _sceneView->setNearFarRatio ( 0.00001 );

  osgUtil::CullVisitor *cv ( _sceneView->getCullVisitor() );  
  cv->setFrameStamp ( _framestamp.get () );

  cv->setCullingMode ( Usul::Bits::remove ( cv->getCullingMode(), osg::CullingSet::SMALL_FEATURE_CULLING ) );
  _sceneView->setCullingMode ( cv->getCullingMode() );

  // Counter for display-list id. OSG will handle using the correct display 
  // list for this context.
  _sceneView->getState()->setContextID ( _contextId );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Capture the screen using super-sampled, tiled rendering.
//
///////////////////////////////////////////////////////////////////////////////

osg::Image* SceneView::screenCapture ( float frameSizeScale, unsigned int numSamples )
{
  // Get our current width and height.
  unsigned int width  ( static_cast < unsigned int > ( this->_width()  * frameSizeScale ) );
  unsigned int height ( static_cast < unsigned int > ( this->_height() * frameSizeScale ) );

  Minerva::OsgTools::TiledScreenCapture tiled;
  tiled.size ( width, height );
  tiled.clearColor ( this->viewer()->getClearColor() );
  tiled.viewMatrix ( this->getViewMatrix() );
  tiled.numSamples ( numSamples );
  tiled.scale ( frameSizeScale );

  return tiled ( this->scene(), _sceneView->getProjectionMatrix() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the width.
//
///////////////////////////////////////////////////////////////////////////////

double SceneView::_width() const
{
  return this->viewer()->getViewport()->width();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the height.
//
///////////////////////////////////////////////////////////////////////////////

double SceneView::_height() const
{
  return this->viewer()->getViewport()->height();
}
