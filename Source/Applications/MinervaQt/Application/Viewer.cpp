
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Viewer.h"
#include "MatrixAnimationComponent.h"

#include "Minerva/Document/PathBuilder.h"
#include "Minerva/Document/CurvePlayer.h"

#include "Usul/Bits/Bits.h"
#include "Usul/Cast/Cast.h"
#include "Usul/Components/Manager.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Interfaces/IAnimateMatrices.h"
#include "Usul/Jobs/Manager.h"
#include "Usul/Registry/Constants.h"
#include "Usul/Registry/Convert.h"
#include "Usul/Registry/Database.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Strings/Format.h"
#include "Usul/Threads/Named.h"
#include "Usul/Threads/ThreadId.h"
#include "Usul/User/Directory.h"

#include "Minerva/OsgTools/Defaults.h"

#include "QtCore/QUrl"
#include "QtCore/QTimer"
#include "QtGui/QResizeEvent"
#include "QtGui/QDialog"
#include "QtGui/QHBoxLayout"
#include "QtGui/QVBoxLayout"
#include "QtGui/QPushButton"
#include "QtGui/QSpinBox"
#include "QtGui/QLabel"
#include "QtGui/QFileDialog"
#include "QtGui/QMessageBox"

#include "osgDB/WriteFile"

#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"

#include <ctime>
#include <limits>

namespace Sections = Usul::Registry::Sections;
namespace Keys = Usul::Registry::Keys;
typedef Usul::Registry::Database Reg;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Viewer::Viewer ( MinervaDocument::RefPtr doc, const QGLFormat& format, QWidget* parent ) :
  BaseClass ( format, parent ),
  _document ( doc ),
  _viewer ( 0x0 ),
  _timer ( 0x0 ),
  _keys(),
  _threadId ( Usul::Threads::currentThreadId() ),
  _mutex ( new Viewer::Mutex ),
  _mouseWheelPosition ( 0 ),
  _mouseWheelSensitivity ( Reg::instance()[Sections::VIEWER_SETTINGS]["mouse_wheel_sensitivity"].get<float> ( 5.0f, true ) ),
  _navigator ( 0x0 ),
  _event0(),
  _event1(),
  _animationController ( new MatrixAnimationComponent ),
  _lastIntersection()
{
  if ( !_document )
  {
    throw std::runtime_error ( "Error: A valid document is required for Qt view." );
  }

  // Initialize openGL.
  this->initializeGL();

  // Create the viewer.
  this->makeCurrent();
  _viewer = new Minerva::Document::SceneView ( doc );

  // Set the focus policy.
  this->setFocusPolicy ( Qt::ClickFocus );

  // Delete on close.
  this->setAttribute ( Qt::WA_DeleteOnClose );

  // Initialize the timer.
  _timer = new QTimer ( this );

  // Enable drag 'n drop.
  this->setAcceptDrops ( true );

  // Update the cursor.
  this->updateCursor();
  
  // We want mouse-move events even when there are no mouse buttons pressed.
  this->setMouseTracking ( true );
  
  _navigator = new Minerva::Core::Navigator ( _document->body() );
  
  QObject::connect ( _animationController, SIGNAL ( setViewMatrix ( Usul::Math::Matrix44d ) ), this, SLOT ( setViewMatrix ( Usul::Math::Matrix44d ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Viewer::~Viewer()
{
  // Stop and delete the timer.
  if ( 0x0 != _timer )
  {
    _timer->stop();
    delete _timer;
    _timer = 0x0;
  }

  // Clear the keys.
  _keys.clear();

  _viewer = 0x0;
  _document = 0x0;

  delete _animationController;
  
  // Now delete the mutex.
  delete _mutex; _mutex = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the document.
//
///////////////////////////////////////////////////////////////////////////////

Viewer::MinervaDocument * Viewer::document()
{
  Guard guard ( this );
  return _document.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the document.
//
///////////////////////////////////////////////////////////////////////////////

const Viewer::MinervaDocument * Viewer::document() const
{
  Guard guard ( this );
  return _document.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Draw.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::paintEvent ( QPaintEvent * event )
{
  USUL_ASSERT ( _navigator.valid() );
  USUL_ASSERT ( _viewer.valid() );
  USUL_ASSERT ( _document.valid() );

  _viewer->setViewMatrix ( osg::Matrixd::inverse ( _navigator->viewMatrix() ) );

  this->makeCurrent();

  Minerva::Core::Data::Camera::RefPtr camera ( _navigator->copyCameraState() );

  _document->updateNotify ( camera );

  _viewer->render ( camera );

  _document->postRenderNotify();

  Minerva::Core::TileEngine::Body::RefPtr body ( _document->body() );
  const bool needsRedraw ( body.valid() && body->needsRedraw() );
  const bool isBusy ( _document->busyStateGet() );
  if ( needsRedraw || isBusy )
  {
    // Request redraw so textures are updated.
    this->update();

    // Request has been made.  Reset state.
    body->needsRedraw ( false );
  }

  this->swapBuffers();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Resize.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::resizeEvent ( QResizeEvent * event )
{
  BaseClass::resizeEvent ( event );
  
  const QSize& size ( event->size() );
  const unsigned int width ( static_cast < unsigned int > ( size.width() ) );
  const unsigned int height ( static_cast < unsigned int > ( size.height() ) );

  _viewer->resize ( width, height );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Window now has focus.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::focusInEvent ( QFocusEvent * event )
{
  // Call the base class first.
  BaseClass::focusInEvent ( event );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Window has lost the focus.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::focusOutEvent ( QFocusEvent * event )
{
  // Call the base class first.
  BaseClass::focusInEvent ( event );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The mouse has moved.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::mouseMoveEvent ( QMouseEvent * event )
{
  // Add event to our stack.
  MouseEventDataPtr data ( this->_pushMouseEventData ( event ) );

  this->_handleNavigation();

  // Intersect.
  osgUtil::LineSegmentIntersector::Intersections hits;
  _viewer->intersect ( data->x, data->y, hits );
  _lastIntersection.reset();

  if ( false == hits.empty() )
  {
    _lastIntersection.reset ( new osgUtil::LineSegmentIntersector::Intersection ( *hits.begin() ) );
  }

  if ( _lastIntersection )
  {
    _viewer->intersectNotify ( *_lastIntersection );
  }

  // Resetting this every mouse-move makes it not very effective.
  // Keeping this here as a reminder.
  //_mouseWheelPosition = 0;

  this->update();
}


///////////////////////////////////////////////////////////////////////////////
//
//  A mouse button has been pressed.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::mousePressEvent ( QMouseEvent * event )
{
  this->_stopAnimation();
  
  MouseEventDataPtr data ( this->_pushMouseEventData ( event ) );

  this->_handleNavigation();

  if ( data->left )
  {
    _viewer->clearBalloon();

    if ( _lastIntersection )
    {
      _viewer->displayBalloon ( *_lastIntersection );
    }
  }
  
  // Reset this.
  _mouseWheelPosition = 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  A mouse button has been released.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::mouseReleaseEvent ( QMouseEvent * event )
{
  this->_pushMouseEventData ( event );

  this->_handleNavigation();

  _event0.reset();
  _event1.reset();

  // Reset this.
  _mouseWheelPosition = 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  A mouse button has been double-clicked.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::mouseDoubleClickEvent ( QMouseEvent * event )
{
  const float x ( event->x() );
  const float y ( this->height() - event->y() );

  this->_handleSeek ( x, y );

  // Reset this.
  _mouseWheelPosition = 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Handle a seek event.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::_handleSeek ( float x, float y )
{
  USUL_ASSERT ( _viewer.valid() );
  USUL_ASSERT ( _document.valid() );

  Minerva::Core::TileEngine::Body::RefPtr body ( _document->body() );

  if ( body.valid() )
  {
    // Intersect.
    osgUtil::LineSegmentIntersector::Intersections hits;
    _viewer->intersect ( x, y, hits );

    if ( hits.empty() )
    {
      return;
    }

    osgUtil::LineSegmentIntersector::Intersection hit ( *hits.begin() );
    osg::Vec3d point ( hit.getWorldIntersectPoint() );

    Usul::Math::Vec3d lonLatPoint;
    body->convertFromPlanet ( Usul::Math::Vec3d ( point[0], point[1], point[2] ), lonLatPoint );

    Minerva::Core::Data::Camera::RefPtr start ( _navigator->copyCameraState() );

    Minerva::Core::Data::Camera::RefPtr destination ( new Minerva::Core::Data::Camera );
    destination->longitude ( lonLatPoint[0] );
    destination->latitude ( lonLatPoint[1] );
    destination->altitude ( Usul::Math::maximum ( start->altitude() * 0.50, 1000.0 ) );
    destination->heading ( 0.0 );
    destination->tilt ( 0.0 );
    destination->roll ( 0.0 );

    typedef Usul::Interfaces::IAnimateMatrices::Matrices Matrices;
    Matrices matrices;
    Minerva::Document::PathBuilder::generateAnimatePath ( start, destination, 50, body->landModel(), matrices );
    
    const unsigned int milliSeconds ( Usul::Registry::Database::instance()[Usul::Registry::Sections::PATH_ANIMATION]["curve"]["milliseconds"].get<unsigned int> ( 15, true ) );
    _animationController->animateMatrices ( matrices, milliSeconds );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  The mouse wheel has been moved.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::wheelEvent ( QWheelEvent * event )
{
  // Adjust the mouse wheel state.
  _mouseWheelPosition += event->delta();

  const float delta ( static_cast < float > ( _mouseWheelPosition ) / _mouseWheelSensitivity );
  
  osg::Matrixd matrix ( _navigator->viewMatrix() );
  Usul::Math::Vec3d eye ( matrix ( 3, 0 ), matrix ( 3, 1 ), matrix ( 3, 2 ) );
  eye.normalize();

  _navigator->zoomAlongDirection ( eye, delta );

  this->update();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Key pressed.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::keyPressEvent ( QKeyEvent *event )
{
  if ( 0x0 == event )
    return;

  _keys[event->key()] = true;

  {
    // Process the key.
    switch ( event->key() )
    {
    // See if it was the space-bar...
    case Qt::Key_Space:
      {
        this->resetCamera();
        this->update();
      }
      break;
      
    // See if it was the w key...
    case Qt::Key_W:

      _viewer->togglePolygonMode ( osg::PolygonMode::LINE );
      _viewer->render ( _navigator->copyCameraState() );
      break;

    // See if it was the p key...
    case Qt::Key_P:

      _viewer->togglePolygonMode ( osg::PolygonMode::POINT );
      _viewer->render ( _navigator->copyCameraState() );
      break;

    case Qt::Key_F11:
      {
        if ( true == Usul::Bits::has ( this->windowState(), Qt::WindowFullScreen ) )
        {
          this->showNormal();
        }
        else
        {
          this->showFullScreen();
        }
      }
      break;
    }
  }
  // Update the cursor.
  this->updateCursor();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Key released.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::keyReleaseEvent ( QKeyEvent * event )
{
  if ( 0x0 == event )
    return;

  _keys[event->key()] = false;

  // Update the cursor.
  this->updateCursor();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the mutex.
//
///////////////////////////////////////////////////////////////////////////////

Viewer::Mutex &Viewer::mutex() const
{
  return *_mutex;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Dragging has entering window.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::dragEnterEvent ( QDragEnterEvent *event )
{
  MinervaDocument::RefPtr document ( this->document() );
  
  if ( false == document.valid() || 0x0 == event )
    return;

  typedef QList < QUrl > Urls;
  typedef Urls::const_iterator ConstIterator;

  Urls urls ( event->mimeData()->urls() );

  for ( ConstIterator i = urls.begin(); i != urls.end(); ++ i )
  {
    std::string file ( i->toLocalFile().toStdString() );

    if ( document->canInsert ( file ) )
    {
      event->acceptProposedAction();
      return;
    }
  }

  event->accept();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Files have been dropped.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::dropEvent ( QDropEvent *event )
{
  MinervaDocument::RefPtr document ( this->document() );

  if ( false == document.valid() || 0x0 == event )
    return;

  typedef QList < QUrl > Urls;
  typedef Urls::const_iterator ConstIterator;

  Urls urls ( event->mimeData()->urls() );

  for ( ConstIterator i = urls.begin(); i != urls.end(); ++ i )
  {
    std::string file ( i->toLocalFile().toStdString() );

    document->read ( file );
  }

  event->acceptProposedAction();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the frame dump state.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::_frameDumpStateSet ( FrameDump::DumpState state )
{
  // Always turn it off first.
  _viewer->setFrameDumpState ( FrameDump::NEVER_DUMP );

  // Are we turning it off?
  if ( FrameDump::NEVER_DUMP == state )
    return;
  
  // Set the properties.
  if ( false == this->_frameDumpProperties() )
    return;

  // Set the frame dump state.
  _viewer->setFrameDumpState ( state );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the frame dump state.
//
///////////////////////////////////////////////////////////////////////////////

Viewer::FrameDump::DumpState Viewer::_frameDumpStateGet() const
{
  Guard guard ( this );
  return _viewer->getFrameDumpState();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is this the frame-dump state?
//
///////////////////////////////////////////////////////////////////////////////

bool Viewer::_isFrameDumpState ( FrameDump::DumpState state ) const
{
  Guard guard ( this );
  return state == _viewer->getFrameDumpState();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show a dialog for frame dump properties.
//
///////////////////////////////////////////////////////////////////////////////

bool Viewer::_frameDumpProperties()
{
  // Get the directory from the registry.
  const std::string defaultFrameDumpDir ( Usul::User::Directory::documents ( true, false ) );
  std::string directory ( Reg::instance()[Sections::VIEWER_SETTINGS][Keys::FRAME_DUMP_DIRECTORY].get ( defaultFrameDumpDir ) );

  // Confirm that this is the correct directory.
  QString dir ( QFileDialog::getExistingDirectory ( this, "Select a directory", directory.c_str() ) );
  if ( 0 == dir.size() )
    return false;

  // Set the directory in the registry.
  directory = dir.toStdString();
  Reg::instance()[Sections::VIEWER_SETTINGS][Keys::FRAME_DUMP_DIRECTORY] = directory;

  // Get the current time.
  ::tm time ( boost::posix_time::to_tm ( boost::posix_time::ptime ( boost::posix_time::second_clock::local_time() ) ) );

  // Convert it to a string.
  const unsigned int size ( 1024 );
  char buffer[size];
  ::memset ( buffer, '\0', size );
  ::strftime ( buffer, size - 1, "%Y_%m_%d_%H_%M_%S", &time );

  directory += std::string ( "/" ) + std::string ( buffer ) + std::string ( "/" );

  // Make the directory.
  boost::filesystem::create_directories ( directory );

  // Set the frame dump properties. (9 works better with ffmpeg.)
  _viewer->frameDumpProperties ( directory, "", ".jpg", 0, 9 );

  return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Close event.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::closeEvent ( QCloseEvent *event )
{
  Usul::Functions::safeCall ( boost::bind ( &Viewer::_closeEvent, this, event ), "2663774419" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make sure we can close.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::_closeEvent ( QCloseEvent* event )
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  // TODO: Ask the document if we can close.
  
  // Close.  Removing this from the document's windows.
  Usul::Functions::safeCall ( boost::bind ( &Viewer::_close, this ), "1749695082" );

  // Pass along to the base class.
  BaseClass::closeEvent ( event );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Tell the document that we are closing.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::_close()
{
  // Clean up the viewer first.
  {
    Guard guard ( this->mutex() );
  
    {
      // Clear the viewer.
      this->makeCurrent();
      _viewer->clear();
    }

    _viewer = 0x0;
  }

  // Clear the document.
  {
    Guard guard ( this );
    _document = 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the cursor.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::updateCursor ( bool left, bool middle, bool right )
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  const bool buttons ( left || middle || right );

  if ( false == buttons )
  {
    this->setCursor ( Qt::OpenHandCursor );
  }
  else
  {
    this->setCursor ( Qt::ClosedHandCursor );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Edit the clipping distances.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::editNearFarRatio()
{
  USUL_ASSERT ( _viewer.valid() );

  QDialog dialog;
  dialog.setWindowTitle ( "Near/Far Ratio" );
  
  QVBoxLayout *topLayout ( new QVBoxLayout );
  dialog.setLayout ( topLayout );
  
  QDoubleSpinBox *zNear ( new QDoubleSpinBox );
  
  zNear->setRange ( std::numeric_limits<double>::min(), std::numeric_limits<double>::max() );
  
  const double nearFarRatio ( _viewer->nearFarRatio() );
  
  zNear->setDecimals ( 7 );
  zNear->setSingleStep ( 0.0001 );
  zNear->setValue ( nearFarRatio );
  
  {
    QHBoxLayout *layout ( new QHBoxLayout );
    layout->addWidget ( new QLabel ( "Near/Far Ratio:" ) );
    layout->addWidget ( zNear );
    topLayout->addLayout ( layout );
  }
  
  {
    QPushButton *ok ( new QPushButton ( "Ok" ) );
    QPushButton *cancel ( new QPushButton ( "Cancel" ) );
    
    QObject::connect ( ok, SIGNAL ( clicked() ), &dialog, SLOT ( accept() ) );
    QObject::connect ( cancel, SIGNAL ( clicked() ), &dialog, SLOT ( reject() ) );
    
    QHBoxLayout *layout ( new QHBoxLayout );
    layout->addStretch();
    layout->addWidget( ok );
    layout->addWidget( cancel );
    topLayout->addLayout ( layout );
  }
  
  if ( QDialog::Accepted == dialog.exec() )
  {
    _viewer->nearFarRatio ( zNear->value() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the view matrix.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setViewMatrix ( Usul::Math::Matrix44d matrix )
{
  osg::Matrix m ( &matrix[0] );
  _navigator->viewMatrix ( osg::Matrix::inverse ( m ) );
  this->update();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Reset the view.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::resetCamera()
{
  _navigator->home();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Push the event onto our mini-event stack.
//
///////////////////////////////////////////////////////////////////////////////

Viewer::MouseEventDataPtr Viewer::_pushMouseEventData ( QMouseEvent * event )
{
  // See if the mouse buttons are down. If the corresponding keys are 
  // down then we simulate a mouse-down state.
  const bool left   ( ( true == event->buttons().testFlag ( Qt::LeftButton  ) ) || ( true == _keys[Qt::Key_R] ) );
  const bool middle ( ( true == event->buttons().testFlag ( Qt::MidButton   ) ) || ( true == _keys[Qt::Key_T] ) );
  const bool right  ( ( true == event->buttons().testFlag ( Qt::RightButton ) ) || ( true == _keys[Qt::Key_Z] ) );

  const float x ( event->x() );
  const float y ( this->height() - event->y() );

  MouseEventDataPtr mouseData ( new MouseEventData );
  mouseData->left = left;
  mouseData->middle = middle;
  mouseData->right = right;
  mouseData->x = x;
  mouseData->y = y;

  // Normalize mouse position between -1.0 and 1.0.
  mouseData->normalizedX = ( 2.0 * ( ( x ) / this->width() ) ) - 1.0;
  mouseData->normalizedY = ( 2.0 * ( ( y ) / this->height() ) ) - 1.0;

  _event1 = _event0;
  _event0 = mouseData;

  this->updateCursor ( left, middle, right );

  return mouseData;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Do the navigating.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::_handleNavigation()
{
  USUL_ASSERT ( _navigator.valid() );

  // Return if less then two events have been added.
  if ( !_event0 || !_event1 )
    return;

  const float dx ( _event0->normalizedX - _event1->normalizedX );
  const float dy ( _event0->normalizedY - _event1->normalizedY );

  // return if there is no movement.
  if ( dx == 0.0f && dy == 0.0f )
    return;

  Minerva::Core::Navigator::MousePosition p0 ( _event0->normalizedX, _event0->normalizedY );
  Minerva::Core::Navigator::MousePosition p1 ( _event1->normalizedX, _event1->normalizedY );

  if ( _event1->left )
  {
    _navigator->rotate ( p0, p1, _viewer->getProjectionMatrix() );
  }
  else if ( _event1->middle || ( _event1->left && _event1->right ) )
  {
    _navigator->zoomLOS ( p0, p1, _viewer->getProjectionMatrix() );
  }
  else if ( _event1->right )
  {
    _navigator->look ( p0, p1 );
  }
  else if ( _event1->left && _event1->middle )
  {
    _navigator->elevation ( p0, p1 );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Take a picture.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::takePicture ( QString filename, double frameScale, unsigned int numSamples )
{
  USUL_ASSERT ( _viewer.valid() );

  this->makeCurrent();
  osg::ref_ptr<osg::Image> image ( _viewer->screenCapture ( frameScale, numSamples ) );
  osgDB::writeImageFile ( *image, filename.toStdString() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the frame dump state.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setFrameDump ( bool value )
{
  FrameDump::DumpState state ( value ? FrameDump::ALWAYS_DUMP : FrameDump::NEVER_DUMP );
  this->_frameDumpStateSet ( state );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the frame dump state.
//
///////////////////////////////////////////////////////////////////////////////

bool Viewer::getFrameDump() const
{
  return FrameDump::ALWAYS_DUMP == this->_frameDumpStateGet();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the show mouse position state.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setShowPosition ( bool value )
{
  USUL_ASSERT ( _viewer.valid() );
  _viewer->showLatLonText ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the eye altitude state.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setShowEyeAltitude ( bool value )
{
  USUL_ASSERT ( _viewer.valid() );
  _viewer->showEyeAltitude ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle visibility of job feedback.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setShowJobFeedback ( bool value )
{
  USUL_ASSERT ( _viewer.valid() );
  _viewer->showJobFeedback ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle visibility of date feedback.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setShowDateFeedback ( bool value )
{
  USUL_ASSERT ( _viewer.valid() );
  _viewer->showDateFeedback ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  show borders.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setShowBorders ( bool value )
{
  USUL_ASSERT ( _document.valid() );
  _document->showBorders ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle allowing of more tile splitting.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setAllowSplitting ( bool value )
{
  USUL_ASSERT ( _document.valid() );
  _document->allowSplit ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle keeping of detail.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::setKeepDetail ( bool value )
{
  USUL_ASSERT ( _document.valid() );
  _document->keepDetail ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop any animation.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::_stopAnimation()
{
  typedef Usul::Interfaces::IAnimateMatrices::Matrices Matrices;
  Matrices matrices;
  _animationController->animateMatrices ( matrices, 0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Fly to the feature.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::flyToFeature ( Minerva::Core::Data::Feature* feature )
{
  if ( !_navigator || !_document )
  {
    return;
  }
  
  typedef Usul::Interfaces::IAnimateMatrices::Matrices Matrices;
  Matrices matrices;
  
  Minerva::Document::PathBuilder::lookAtLayer ( _navigator->copyCameraState(), feature, _document->body(), matrices );
  
  const unsigned int milliSeconds ( Usul::Registry::Database::instance()[Usul::Registry::Sections::PATH_ANIMATION]["curve"]["milliseconds"].get<unsigned int> ( 15, true ) );
  _animationController->animateMatrices ( matrices, milliSeconds );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Fly to the location.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::flyToLocation ( double lon, double lat )
{
  if ( !_navigator || !_document )
  {
    return;
  }
  
  typedef Usul::Interfaces::IAnimateMatrices::Matrices Matrices;
  Matrices matrices;
  
  Minerva::Document::PathBuilder::lookAtPoint ( _navigator->copyCameraState(), Usul::Math::Vec2d ( lon, lat ), _document->body(), matrices );
  
  const unsigned int milliSeconds ( Usul::Registry::Database::instance()[Usul::Registry::Sections::PATH_ANIMATION]["curve"]["milliseconds"].get<unsigned int> ( 15, true ) );
  _animationController->animateMatrices ( matrices, milliSeconds );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the camera.
//
///////////////////////////////////////////////////////////////////////////////

Viewer::CameraState::RefPtr Viewer::camera() const
{  
  Minerva::Core::Data::Camera::RefPtr camera ( _navigator.valid() ? _navigator->copyCameraState() : new Minerva::Core::Data::Camera );
  Usul::Math::Matrix44d matrix ( _document.valid() ? camera->viewMatrix ( _document->body()->landModel() ) : Usul::Math::Matrix44d() );

  CameraState::RefPtr state ( new CameraState ( 
                                               camera->longitude(), 
                                               camera->latitude(), 
                                               camera->altitude(),
                                               camera->heading(),
                                               camera->tilt(),
                                               camera->roll(),
                                               matrix ) );
  return state;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Go to the camera position.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::goToCameraPostion ( Minerva::Document::CameraPath::RefPtr path, unsigned int cameraIndex )
{
  if ( path )
  {
    Minerva::Document::CameraPath::RefPtr temp ( new Minerva::Document::CameraPath );
    Minerva::Document::callCameraFunction ( boost::bind ( &Minerva::Document::CameraPath::append, temp.get(), _1, _2, _3 ), this->camera() );

    Usul::Math::Vec3d eye, center, up;
    path->camera ( eye, center, up, cameraIndex );
    temp->append ( eye, center, up );
    this->playPathForward ( temp );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play the camera path forward.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::playPathForward ( Minerva::Document::CameraPath::RefPtr path )
{
  if ( false == path.valid() )
  {
    return;
  }

  Minerva::Document::CurvePlayer::RefPtr player ( new Minerva::Document::CurvePlayer );
  player->playForward ( path.get(), 4 );
  player->playing ( true );

  MatrixAnimationComponent::Matrices matrices;
  while ( player->playing() )
  {
    matrices.push_back ( player->update().ptr() );
  }

  const unsigned int milliSeconds ( Usul::Registry::Database::instance()[Usul::Registry::Sections::PATH_ANIMATION]["curve"]["milliseconds"].get<unsigned int> ( 15, true ) );
  _animationController->animateMatrices ( matrices, milliSeconds );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play the camera path backward.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::playPathBackward ( Minerva::Document::CameraPath::RefPtr path )
{
  if ( false == path.valid() )
  {
    return;
  }

  Minerva::Document::CurvePlayer::RefPtr player ( new Minerva::Document::CurvePlayer );
  player->playBackward ( path.get(), 4 );
  player->playing ( true );

  MatrixAnimationComponent::Matrices matrices;
  while ( player->playing() )
  {
    matrices.push_back ( player->update().ptr() );
  }

  const unsigned int milliSeconds ( Usul::Registry::Database::instance()[Usul::Registry::Sections::PATH_ANIMATION]["curve"]["milliseconds"].get<unsigned int> ( 15, true ) );
  _animationController->animateMatrices ( matrices, milliSeconds );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop any animation.
//
///////////////////////////////////////////////////////////////////////////////

void Viewer::stopAnimation()
{
  this->_stopAnimation();
}
