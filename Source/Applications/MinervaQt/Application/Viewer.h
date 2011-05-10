
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_QT_VIEWER_H__
#define __MINERVA_QT_VIEWER_H__

#include "Minerva/Document/MinervaDocument.h"
#include "Minerva/Document/CameraPath.h"
#include "Minerva/Document/SceneView.h"

#include "Minerva/Core/Navigator.h"
#include "Minerva/Core/Data/CameraState.h"

#include "Usul/Math/Matrix44.h"
#include "Usul/Threads/RecursiveMutex.h"
#include "Usul/Threads/Guard.h"

#include "QtOpenGL/QGLWidget"

#include "osgUtil/LineSegmentIntersector"

#include "boost/shared_ptr.hpp"

#include <map>

class MatrixAnimationComponent;
class QTimer;


class Viewer : public QGLWidget
{
  Q_OBJECT

public:

  /// Typedefs.
  typedef QGLWidget BaseClass;
  typedef Minerva::Document::MinervaDocument MinervaDocument;
  typedef std::map<int,bool> KeyMap;
  typedef Usul::Threads::RecursiveMutex Mutex;
  typedef Usul::Threads::Guard<Mutex> Guard;
  typedef Usul::Interfaces::IUnknown IUnknown;
  typedef Minerva::OsgTools::FrameDump FrameDump;
  typedef Minerva::Core::Data::CameraState CameraState;

  /// Constructor.
  Viewer ( MinervaDocument::RefPtr doc, const QGLFormat& format, QWidget* parent );
  virtual ~Viewer();
  
  /// Get the camera state.
  CameraState::RefPtr camera() const;

  /// Edit the near/far ratio.
  void editNearFarRatio();
  
  void flyToFeature ( Minerva::Core::Data::Feature* );
  void flyToLocation ( double lon, double lat );

  // Reset the view.
  void resetCamera();

  // Get the mutex.
  Mutex& mutex() const;

  // Set frame dump state.
  void setFrameDump ( bool value );
  bool getFrameDump() const;

  void goToCameraPostion ( Minerva::Document::CameraPath::RefPtr, unsigned int cameraIndex );
  void playPathForward ( Minerva::Document::CameraPath::RefPtr );
  void playPathBackward ( Minerva::Document::CameraPath::RefPtr );
  void stopAnimation();

  void setShowPosition ( bool value );
  void setShowEyeAltitude ( bool value );
  void setShowJobFeedback ( bool value );
  void setShowDateFeedback ( bool value );
  void setShowBorders ( bool value );
  void setAllowSplitting ( bool value );
  void setKeepDetail ( bool value );

  void takePicture ( QString filename, double frameScale, unsigned int numSamples );
  
public slots:
  
  void setViewMatrix ( Usul::Math::Matrix44d );

protected:

  /// Get the document.
  MinervaDocument *                       document();
  const MinervaDocument *                 document() const;

  /// Update the cursor.
  void                                    updateCursor ( bool left = false, bool middle = false, bool right = false );

  void                                    _frameDumpStateSet ( FrameDump::DumpState b );
  bool                                    _frameDumpProperties();
  FrameDump::DumpState                    _frameDumpStateGet() const;
  bool                                    _isFrameDumpState ( FrameDump::DumpState ) const;

  void                                    _closeEvent ( QCloseEvent* event );
  void                                    _close();

  // Override these events.
  virtual void                            paintEvent  ( QPaintEvent * );
  virtual void                            resizeEvent ( QResizeEvent * );
  virtual void                            focusInEvent ( QFocusEvent * );
  virtual void                            focusOutEvent ( QFocusEvent * );
  virtual void                            mouseMoveEvent ( QMouseEvent * );
  virtual void                            mousePressEvent ( QMouseEvent * );
  virtual void                            mouseReleaseEvent ( QMouseEvent * );
  virtual void                            mouseDoubleClickEvent ( QMouseEvent * );
  virtual void                            keyPressEvent ( QKeyEvent * );
  virtual void                            keyReleaseEvent ( QKeyEvent * );
  virtual void                            dragEnterEvent ( QDragEnterEvent * );
  virtual void                            dropEvent ( QDropEvent * );
  virtual void                            closeEvent ( QCloseEvent * );
  virtual void                            wheelEvent ( QWheelEvent * );

private:

  struct MouseEventData
  {
    bool left;
    bool middle;
    bool right;

    float x;
    float y;

    float normalizedX;
    float normalizedY;
  };

  typedef boost::shared_ptr<MouseEventData> MouseEventDataPtr;

  // Push the event onto our mini-event stack.
  MouseEventDataPtr _pushMouseEventData ( QMouseEvent * );

  void _handleNavigation();
  void _handleSeek ( float x, float y );
  
  void _stopAnimation();

  MinervaDocument::RefPtr _document;
  Minerva::Document::SceneView::RefPtr _viewer;
  QTimer *_timer;
  KeyMap _keys;
  unsigned long _threadId;
  mutable Mutex *_mutex;
  int _mouseWheelPosition;
  float _mouseWheelSensitivity;
  Minerva::Core::Navigator::RefPtr _navigator;
  MouseEventDataPtr _event0;
  MouseEventDataPtr _event1;
  
  MatrixAnimationComponent* _animationController;

  boost::shared_ptr<osgUtil::LineSegmentIntersector::Intersection> _lastIntersection;
};


#endif // __MINERVA_QT_VIEWER_H__
