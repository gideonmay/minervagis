
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "VRV/Common/Libraries.h"
#include "VRV/Core/Application.h"
#include "VRV/Core/JugglerFunctors.h"
#include "VRV/Core/Exceptions.h"
#include "VRV/Common/Constants.h"

#include "Usul/App/Application.h"
#include "Usul/Components/Manager.h"
#include "Usul/Bits/Bits.h"
#include "Usul/CommandLine/Arguments.h"
#include "Usul/Convert/Convert.h"
#include "Usul/Convert/Matrix44.h"
#include "Usul/Convert/Vector4.h"
#include "Usul/Errors/Assert.h"
#include "Usul/File/Path.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Math/Constants.h"
#include "Usul/Math/Matrix44.h"
#include "Usul/Math/MinMax.h"
#include "Usul/Print/Matrix.h"
#include "Usul/Registry/Database.h"
#include "Usul/Registry/Convert.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Strings/Format.h"
#include "Usul/System/Host.h"
#include "Usul/Threads/Safe.h"

#include "Minerva/OsgTools/StateSet.h"
#include "Minerva/OsgTools/ConvertMatrix.h"
#include "Minerva/OsgTools/ConvertVector.h"
#include "Minerva/OsgTools/Group.h"
#include "Minerva/OsgTools/Font.h"

#include "osg/CoordinateSystemNode"
#include "osg/MatrixTransform"
#include "osg/Quat"
#include "osg/Version"
#include "osg/Material"

#include "vrj/Kernel/Kernel.h"
#include "vrj/Draw/OpenGL/Window.h"

#include "boost/bind.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/program_options.hpp"

#include <algorithm>
#include <stdexcept>
#include <fstream>

USUL_IO_TEXT_DEFINE_WRITER_TYPE_VECTOR_4 ( osg::Vec4 );
USUL_IO_TEXT_DEFINE_READER_TYPE_VECTOR_4 ( osg::Vec4 );
SERIALIZE_XML_DECLARE_VECTOR_4_WRAPPER ( osg::Vec4 );

using namespace VRV::Core;
namespace Sections = VRV::Constants::Sections;
namespace Keys = VRV::Constants::Keys;
typedef Usul::Registry::Database Reg;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Application::Application() : 
  BaseClass(),
  _root              ( new osg::Group ),
  _navBranch         ( new osg::MatrixTransform ),
  _models            ( new osg::MatrixTransform ),
  _timer             (),
  _initialTime       ( static_cast < osg::Timer_t > ( 0.0 ) ),
  _frameStart        ( static_cast < osg::Timer_t > ( 0.0 ) ),
  _sharedFrameTime   (),
  _sharedReferenceTime  (),
  _sharedMatrix      (),
  _frameTime         ( 1 ),
  _home              ( osg::Matrixd::identity() ),
  _document ( new Minerva::Document::MinervaDocument ),
  _animationController ( new Minerva::Document::AnimationController ),
  _wingMan ( new VRV::Devices::Wingman ),
  _navigator ( new VRV::Core::Navigator ),
  _isAnimating ( false ),
  _lastTimeAnimationStep ( 0.0 ),
  _hudCamera ( new osg::Camera )
{
  // We want thread safe ref and unrefing.
  osg::Referenced::setThreadSafeReferenceCounting ( true );

   // Hook up the branches
  _root->addChild      ( _navBranch.get()   );
  _navBranch->addChild ( _models.get()      );

  _hudCamera->setRenderOrder ( osg::Camera::POST_RENDER );
  _hudCamera->setReferenceFrame ( osg::Camera::ABSOLUTE_RF );
  _hudCamera->setClearMask( GL_DEPTH_BUFFER_BIT );
  _hudCamera->setViewMatrix( osg::Matrix::identity() );
  _hudCamera->setComputeNearFarMode ( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
  _hudCamera->setCullingMode ( osg::CullSettings::NO_CULLING );
  if ( "c14" == Usul::System::Host::name() )
  {
    _hudCamera->setViewport ( 0, 0, 2560, 1600 );
    _hudCamera->setProjectionMatrixAsOrtho ( 0, 2560, 0, 1600, -1000.0, 1000.0 );

    _root->addChild ( _hudCamera.get() );
  }

  // Parse the command-line arguments.
  this->_parseCommandLine();

  // Crank up the max number of contexts. When you figure out a good way to
  // get the real number, then use it here instead of this hard-coded one.
  // Note: this has to be done early. Waiting for init() or contextInit() is
  // too late.
  osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts ( 20 );

  _root->getOrCreateStateSet()->setMode ( GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
  _models->getOrCreateStateSet()->setMode ( GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );

  _wingMan->addButtonPressCallback ( "VJButton8", boost::bind ( &Application::quit, this ) );
  _wingMan->addButtonPressCallback ( "VJButton9", boost::bind ( &Application::reinitialize, this ) );
  _wingMan->addButtonPressCallback ( "VJButton0", boost::bind ( &Application::resetView, this ) );
  _wingMan->addButtonPressCallback ( "VJButton1", boost::bind ( &Application::viewScene, this ) );
  _wingMan->addButtonPressCallback ( "VJButton2", boost::bind ( &VRV::Devices::Wingman::analogsCalibrate, _wingMan.get() ) );
  _wingMan->addButtonPressCallback ( "VJButton3", boost::bind ( &Application::toggleAnimation, this ) );

  _wingMan->addButtonPressCallback ( "VJButton4", boost::bind ( &Application::previousTimestep, this ) );
  _wingMan->addButtonPressCallback ( "VJButton5", boost::bind ( &Application::nextTimestep, this ) );

  _wingMan->addButtonPressCallback ( "VJButton10", boost::bind ( &Application::_setHome, this ) );

  //_animationController->decreaseStepSize();
  //_animationController->decreaseStepSize();
  _animationController->setStepSize ( 3 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Application::~Application()
{
  // Make sure.
  this->cleanup();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear.
//
///////////////////////////////////////////////////////////////////////////////

void Application::cleanup()
{
  // Clean up the scene.
  OsgTools::Group::removeAllChildren ( _root.get() );
  OsgTools::Group::removeAllChildren ( _navBranch.get() );
  OsgTools::Group::removeAllChildren ( _models.get() );

  _root = 0x0;
  _navBranch = 0x0;
  _models = 0x0;

  // Wait for all jobs to finish
  Usul::Jobs::Manager::instance().wait();

  // Clear the active view and document.
  _document = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Called once for each display ( or draw thread ) to initialize the OpenGL context.
//
///////////////////////////////////////////////////////////////////////////////

void Application::configSceneView ( osgUtil::SceneView* newSceneViewer )
{
  BaseClass::configSceneView ( newSceneViewer );

  // Turn on back face culling.
  newSceneViewer->getGlobalStateSet()->setMode( GL_CULL_FACE, osg::StateAttribute::ON );
  //newSceneViewer->getGlobalStateSet()->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

  newSceneViewer->setClearColor ( osg::Vec4 ( 0.0, 0.0, 0.0, 1.0 ) );

  osgUtil::CullVisitor *cv ( newSceneViewer->getCullVisitor() );
  cv->setCullingMode ( Usul::Bits::remove ( cv->getCullingMode(), osg::CullingSet::SMALL_FEATURE_CULLING ) );
  newSceneViewer->setCullingMode ( cv->getCullingMode() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Draw.
//
///////////////////////////////////////////////////////////////////////////////

void Application::draw()
{
  // Do the drawing.
  //BaseClass::draw();

  glClear(GL_DEPTH_BUFFER_BIT);

  // Users have reported problems with OpenGL reporting stack underflow
  // problems when the texture attribute bit is pushed here, so we push all
  // attributes *except* GL_TEXTURE_BIT.
  glPushAttrib(GL_ALL_ATTRIB_BITS & ~GL_TEXTURE_BIT);
  glPushAttrib(GL_TRANSFORM_BIT);
  glPushAttrib(GL_VIEWPORT_BIT);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();

  ::osg::ref_ptr<osgUtil::SceneView> sv;
  sv = (*sceneViewer);    // Get context specific scene viewer
  vprASSERT(sv.get() != NULL);

  // The OpenGL Draw Manager that we are rendering for.
  //Get the view matrix and the frustrum form the draw manager
  vrj::opengl::DrawManager* gl_manager =
     dynamic_cast<vrj::opengl::DrawManager*>(this->getDrawManager());
  vprASSERT(gl_manager != NULL);

  vrj::opengl::UserData* user_data = gl_manager->currentUserData();

  // Set the up the viewport (since OSG clears it out)
  float vp_ox, vp_oy, vp_sx, vp_sy;   // The float vrj sizes of the view ports
  int w_ox, w_oy, w_width, w_height;  // Origin and size of the window
  user_data->getViewport()->getOriginAndSize(vp_ox, vp_oy, vp_sx, vp_sy);
  user_data->getGlWindow()->getOriginSize(w_ox, w_oy, w_width, w_height);

  // compute unsigned versions of the viewport info (for passing to glViewport)
  const unsigned int ll_x =
     static_cast<unsigned int>(vp_ox * static_cast<float>(w_width));
  const unsigned int ll_y =
     static_cast<unsigned int>(vp_oy * static_cast<float>(w_height));
  const unsigned int x_size =
     static_cast<unsigned int>(vp_sx * static_cast<float>(w_width));
  const unsigned int y_size =
     static_cast<unsigned int>(vp_sy * static_cast<float>(w_height));

  const double eyeAltitude ( _navigator->camera()->altitude() );

  // Sometimes the eye is invalid if there hasn't been any navigation.
  // Put in a check for this.  Need to revist why the eye altitude is invalid.
  if ( eyeAltitude > 10000 || eyeAltitude < 0.0 )
  {
    sv->setComputeNearFarMode(osgUtil::CullVisitor::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
  }
  else
  {
    sv->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
    vrj::Projection::setNearFar ( 10, osg::WGS_84_RADIUS_EQUATOR * 3 );
  }

  //sv->setCalcNearFar(false);
  //sv->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
  //sv->setComputeNearFarMode(osgUtil::CullVisitor::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
  sv->setViewport(ll_x, ll_y, x_size, y_size);

  //Get the frustrum
  vrj::ProjectionPtr project = user_data->getProjection();
  vrj::Frustum frustum = project->getFrustum();
  sv->setProjectionMatrixAsFrustum(frustum[vrj::Frustum::VJ_LEFT],
                                   frustum[vrj::Frustum::VJ_RIGHT],
                                   frustum[vrj::Frustum::VJ_BOTTOM],
                                   frustum[vrj::Frustum::VJ_TOP],
                                   frustum[vrj::Frustum::VJ_NEAR],
                                   frustum[vrj::Frustum::VJ_FAR]);

  // Copy the view matrix
  sv->setViewMatrix(::osg::Matrix(project->getViewMatrix().mData));

  //Draw the scene
  // NOTE: It is not safe to call osgUtil::SceneView::update() here; it
  // should only be called by a single thread. The equivalent of calling
  // osgUtil::SceneView::update() is in vrj::osg::App::update().
  sv->cull();
  sv->draw();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib();
  glPopAttrib();
  glPopAttrib();

  this->_postRenderNotify();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize the application.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_init()
{
  // Call the base class first.
  BaseClass::_init();

  // Set the initial time.
  _initialTime = _timer.tick();

  // Initialize the shared frame time data.
  {
    vpr::GUID guid ( "8297080d-c22c-41a6-91c1-188a331fabe5" );
    _sharedFrameTime.init ( guid );
  }

  // Initialize the shared frame start data.
  {
    vpr::GUID guid ( "2E3E374B-B232-476f-A870-F854E717F61A" );
    _sharedReferenceTime.init ( guid );
  }

  // Initialize the shared navigation matrix.
  {
    vpr::GUID guid ( "FEFB5D44-9EC3-4fe3-B2C7-43C394A49848" );
    _sharedMatrix.init ( guid );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Called before the frame.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_preFrame()
{
  // Call the base class first.
  BaseClass::_preFrame();

  // Now guard.
  Guard guard ( this->mutex() );

  // Mark the start of the frame.
  _frameStart = _timer.tick();

  // Write the frame time if we've suppose to.
  if( _sharedFrameTime.isLocal() )
  {
    // Capture the frame time.
    _sharedFrameTime->data ( _frameTime );
  }

  // Write out the start of the frame.
  if ( _sharedReferenceTime.isLocal() )
  {
    _sharedReferenceTime->data ( osg::Timer::instance()->delta_m ( _initialTime, osg::Timer::instance()->tick() ) );
  }

  // Update these input devices.
  _wingMan->update();
  
  // Navigate if we are supposed to.
  this->_navigate();

  // Write out the navigation matrix.
  if ( _sharedMatrix.isLocal() )
  {
    _sharedMatrix->data ( _navBranch->getMatrix() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Called after preFrame, but before the frame.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_latePreFrame()
{
  // Get the frame time.
  _frameTime = _sharedFrameTime->data();

  const double currentTime ( this->_getCurrentTimeInMilliSeconds() );

  const double ANIMATION_STEP_TIME ( 300.0 );
  const double elapsedTime ( currentTime - _lastTimeAnimationStep );
  if ( _isAnimating && elapsedTime > ANIMATION_STEP_TIME )
  {
    std::cout << "Elapsed time:" << elapsedTime << std::endl;
    this->nextTimestep();
    _lastTimeAnimationStep = currentTime;
  }

  // Set the navigation matrix.
  _navBranch->setMatrix ( _sharedMatrix->data() );

  // Notify that it's ok to update.
  this->_updateNotify();

  std::string dateFeedback ( "" );
  if ( _document )
  {
    Minerva::Core::Data::TimeSpan::RefPtr visibleTimeSpan ( _document->visibleTimeSpan() );
    if ( visibleTimeSpan )
    {
      Minerva::Core::Data::Date firstDate ( visibleTimeSpan->begin() );
      Minerva::Core::Data::Date lastDate ( visibleTimeSpan->end() );

      if ( !firstDate.date().is_special() && !lastDate.date().is_special() )
      {
        dateFeedback = lastDate.toString();
      }
    }
  }

  osg::ref_ptr<osgText::Text> text ( new osgText::Text );
  text->setText ( dateFeedback );
  text->setCharacterSize ( 50 );
  text->setCharacterSizeMode ( osgText::Text::OBJECT_COORDS );
  text->setColor ( osg::Vec4 ( 1.0, 1.0, 1.0, 1.0 ) );
  text->setBackdropColor ( osg::Vec4 ( 0.0, 0.0, 0.0, 1.0 ) );
  text->setBackdropType ( osgText::Text::DROP_SHADOW_BOTTOM_LEFT );

  text->setAutoRotateToScreen ( true );

  osg::ref_ptr<osgText::Font> font ( OsgTools::Font::defaultFont() );
  text->setFont ( font.get() );

  osg::BoundingBox bb ( text->computeBound() );
  const float h ( bb.yMax() - bb.yMin() );
  const float w ( bb.xMax() - bb.xMin() );
  //text->setPosition ( osg::Vec3 ( 2560 - w, 1600 - h / 2.0, 0.0 ) );
  text->setPosition ( osg::Vec3 ( 2050, 1525, 0.0 ) );

  osg::ref_ptr<osg::Geode> geode ( new osg::Geode );
  geode->addDrawable ( text.get() );

  _hudCamera->removeChildren ( 0, _hudCamera->getNumChildren() );
  _hudCamera->addChild ( geode.get() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Called when the frame is done.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_postFrame()
{
  // Capture the frame time.
  _frameTime = _timer.delta_s ( _frameStart, _timer.tick() );

  // Make sure.
  this->_setNearAndFarClippingPlanes();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the duration of the last frame in seconds.
//
///////////////////////////////////////////////////////////////////////////////

double Application::frameTime() const
{
  return _frameTime;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Load the models.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_loadModelFiles ( const Filenames& filenames )
{
  for ( Filenames::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter )
  {
    const std::string filename ( *iter );
    if ( ".minerva" == boost::filesystem::extension ( filename ) )
    {
      OsgTools::Group::removeAllChildren ( _models.get() );

      std::cout << "Loading filename " << filename << std::endl;

      Minerva::Document::MinervaDocument::RefPtr document ( new Minerva::Document::MinervaDocument );
      document->read ( filename );

      osg::ref_ptr < osg::Node > model ( document->buildScene() );
      
      // Hook things up.
      _models->addChild ( model );

      // Based on the scene size, set the near and far clipping plane distances.
      this->_setNearAndFarClippingPlanes();

      // Set the document.
      _document = document;

      // Set the global timespan.
      _animationController->globalTimeSpan ( document->timeSpanOfData() );

      // Set the navigator data.
      _navigator->body ( document->body() );

      Usul::Math::Matrix44d m ( Usul::Registry::Database::instance()[this->_documentSection()][VRV::Constants::Keys::HOME_POSITION].get<Usul::Math::Matrix44d> ( Usul::Math::Matrix44d(), true ) );
      _home.set ( &m[0] );

      // Push it back so we can see it.
      this->viewScene();
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the models node.
//
///////////////////////////////////////////////////////////////////////////////

osg::MatrixTransform* Application::models()
{
  Guard guard ( this->mutex() );
  return _models.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the models node.
//
///////////////////////////////////////////////////////////////////////////////

const osg::MatrixTransform* Application::models() const
{
  Guard guard ( this->mutex() );
  return _models.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the near and far clipping planes based on the scene.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_setNearAndFarClippingPlanes()
{
  // Set both distances.
  const double zNear ( 100 );
  const double zFar  ( 4 * osg::WGS_84_RADIUS_EQUATOR );

  vrj::Projection::setNearFar ( zNear, zFar );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the navigation scene.
//
///////////////////////////////////////////////////////////////////////////////

const osg::Group *Application::navigationScene() const
{
  Guard guard ( this->mutex() );
  return _navBranch.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the navigation scene.
//
///////////////////////////////////////////////////////////////////////////////

osg::Group *Application::navigationScene()
{
  Guard guard ( this->mutex() );
  return _navBranch.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the models scene.
//
///////////////////////////////////////////////////////////////////////////////

const osg::Group *Application::modelsScene() const
{
  Guard guard ( this->mutex() );
  return _models.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the models scene.
//
///////////////////////////////////////////////////////////////////////////////

osg::Group *Application::modelsScene()
{
  Guard guard ( this->mutex() );
  return _models.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the navigation matrix.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_navigationMatrix ( const osg::Matrixd& m )
{
  Guard guard ( this->mutex() );
  _navBranch->setMatrix ( m );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the navigation matrix.
//
///////////////////////////////////////////////////////////////////////////////

const osg::Matrixd& Application::_navigationMatrix() const
{
  Guard guard ( this->mutex() );
  return _navBranch->getMatrix();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Print the usage string.
//
///////////////////////////////////////////////////////////////////////////////

void Application::usage ( const std::string &exe, std::ostream &out )
{
  out << "usage: " << exe << ' ';
  out << "<juggler1.config> [juggler2.config ... jugglerN.config] ";
  out << "[document] ";
  out << '\n';
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update notify.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_updateNotify()
{
  Guard guard ( this->mutex() );

  if ( _document.valid() )
  {
    USUL_ASSERT ( _navigator );
    _document->updateNotify ( _navigator->camera() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Navigate if we are supposed to.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_navigate()
{
  this->setViewMatrix ( osg::Matrixd::inverse ( _navigator->viewMatrix ( _wingMan, this->frameTime() ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the view matrix ( Usul::Interfaces::IViewMatrix ).
//  Note: In this implementation, the navigation matrix is set.
//
///////////////////////////////////////////////////////////////////////////////

void Application::setViewMatrix ( const osg::Matrixf& m )
{
  Guard guard ( this->mutex() );
  _navBranch->setMatrix ( m );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the view matrix ( Usul::Interfaces::IViewMatrix ).
//  Note: In this implementation, the navigation matrix is set.
//
///////////////////////////////////////////////////////////////////////////////

void Application::setViewMatrix ( const osg::Matrixd& m )
{
  Guard guard ( this->mutex() );
  _navBranch->setMatrix ( m );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the view matrix ( Usul::Interfaces::IViewMatrix ).
//  Note: In this implementation, the navigation matrix is set.
//
///////////////////////////////////////////////////////////////////////////////

osg::Matrixd Application::getViewMatrix() const
{
  Guard guard ( this->mutex() );
  return _navBranch->getMatrix ( );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Parse the command-line arguments.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_parseCommandLine()
{
  typedef boost::filesystem::path Path;
  typedef std::vector<std::string> Paths;

  boost::program_options::options_description baseOptions ( "Options" );
  baseOptions.add_options()
      ( "cache-directory", boost::program_options::value<std::string>(), "Specify the cache directory file." );

  boost::program_options::options_description hidden ( "Hidden options" );
  hidden.add_options()
    ( "files", boost::program_options::value<Paths>(), "files");

  boost::program_options::options_description allOptions;
  allOptions.add ( baseOptions ).add ( hidden );

  boost::program_options::positional_options_description p;
  p.add("files", -1);

  // Get the command line arguments.
  using Usul::CommandLine::Arguments;

  boost::program_options::variables_map vm;
  boost::program_options::store (
    boost::program_options::command_line_parser ( Arguments::instance().argc(), Arguments::instance().argv() ).
    options ( allOptions ).positional(p).allow_unregistered().run(), vm );

  // Check for the cache directory.
  if ( vm.count ( "cache-directory" ) )
  {
    const std::string cacheDirectory ( vm["cache-directory"].as<std::string>() );
    std::cout << "Setting cache directory to " << cacheDirectory << std::endl;
    Minerva::Core::DiskCache::instance().cacheDirectory ( cacheDirectory );
  }

  Paths filenames;

  if ( vm.count ( "files" ) )
  {
    filenames = vm["files"].as<Paths>();
  }

  // Have to load the config files now. Remove them from the arguments.
  Paths configs;
  
  Path ext ( ".jconf" );
  std::remove_copy_if ( filenames.begin(), filenames.end(), std::back_inserter ( configs ),
		        boost::lambda::bind ( static_cast<Path::string_type (*) ( const Path& )> ( &boost::filesystem::extension ), boost::lambda::_1 ) != ext );

  this->_loadConfigFiles ( configs );

  // Load the model files.
  this->_loadModelFiles ( filenames );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Goto camera position.
//
///////////////////////////////////////////////////////////////////////////////

void Application::resetView()
{
  Guard guard ( this );
  this->_setNearAndFarClippingPlanes();

  std::cout << "Going home." << std::endl;

  _navigator->viewMatrix ( osg::Matrixd::inverse ( _home ) );
  this->setViewMatrix ( _home );
}


///////////////////////////////////////////////////////////////////////////////
//
// Set the current camera position as the home view.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_setHome()
{
  std::cout << "Setting home" << std::endl;

  Guard guard ( this->mutex() );
  _home = this->_navigationMatrix();

  Usul::Math::Matrix44d m;
  OsgTools::Convert::matrix ( _home, m );
  Usul::Registry::Database::instance()[ this->_documentSection () ][ VRV::Constants::Keys::HOME_POSITION ] = m;
}


///////////////////////////////////////////////////////////////////////////////
//
// Move the camera such that the whole world is visible.
//
///////////////////////////////////////////////////////////////////////////////

void Application::viewScene()
{
  _navigator->home();
  this->setViewMatrix ( _navigator->viewMatrix ( _wingMan, 0.0 ) );

  this->_setNearAndFarClippingPlanes();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the document.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Document::MinervaDocument* Application::document()
{
  Guard guard ( this );
  return _document.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get section for current document.
//
///////////////////////////////////////////////////////////////////////////////

std::string Application::_documentSection() const
{
  // Get the active document.
  Guard guard ( this );
  return ( _document.valid() ? _document->registryTagName() : "Document" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Reinitialize.
//
///////////////////////////////////////////////////////////////////////////////

void Application::reinitialize()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Rendering is about to end.
//
///////////////////////////////////////////////////////////////////////////////

void Application::_postRenderNotify()
{
  Guard guard ( this->mutex() );
  if ( _document.valid() )
  {
    _document->postRenderNotify();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Go to the next timestep.
//
///////////////////////////////////////////////////////////////////////////////

void Application::nextTimestep()
{
  using Minerva::Document::AnimationController;
  if ( AnimationController::ANIMATION_RESULT_AT_END == _animationController->stepForward() )
  {
    _animationController->setCurrentTimeStep ( 0 );
  }

  if ( _document )
  {
    _document->visibleTimeSpan ( _animationController->visibleTimeSpan() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Go to the previous timestep.
//
///////////////////////////////////////////////////////////////////////////////

void Application::previousTimestep()
{
  _animationController->stepBackward();

  if ( _document )
  {
    _document->visibleTimeSpan ( _animationController->visibleTimeSpan() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle the animation state.
//
///////////////////////////////////////////////////////////////////////////////

void Application::toggleAnimation()
{
  if ( _isAnimating )
  {
    _isAnimating = false;
    _lastTimeAnimationStep = 0.0;
  }
  else
  {
    _isAnimating = true;
    const double currentTime ( this->_getCurrentTimeInMilliSeconds() );
    _lastTimeAnimationStep = currentTime;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the current time in milliseconds.
//
///////////////////////////////////////////////////////////////////////////////

double Application::_getCurrentTimeInMilliSeconds() const
{
  return vrj::Kernel::instance()->getUsers()[0]->getHeadPosProxy()->getTimeStamp().msecd();
}
