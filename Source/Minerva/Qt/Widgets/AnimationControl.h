
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Control panel for animations.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_QT_ANIMATION_CONTROL_PANEL_CLASS_H_
#define _MINERVA_QT_ANIMATION_CONTROL_PANEL_CLASS_H_

#include "Minerva/Qt/Widgets/Export.h"

#include "Minerva/Document/AnimationController.h"
#include "Minerva/Document/MinervaDocument.h"

#include "Usul/Threads/RecursiveMutex.h"
#include "Usul/Threads/Guard.h"

#include "QtCore/QTimer"
#include "QtGui/QWidget"

#include "boost/shared_ptr.hpp"

namespace Ui { class AnimationControl; }

namespace Minerva {
namespace QtWidgets {


class MINERVA_QT_WIDGETS_EXPORT AnimationControl : public QWidget
{
  Q_OBJECT;

public:

  // Useful typedefs.
  typedef QWidget BaseClass;
  typedef Usul::Interfaces::IUnknown Unknown;
  typedef Usul::Threads::RecursiveMutex Mutex;
  typedef Usul::Threads::Guard < Mutex > Guard;
  typedef Minerva::Document::AnimationController AnimationController;
  typedef Minerva::Document::MinervaDocument MinervaDocument;
  typedef boost::shared_ptr<QTimer> QTimerPtr;

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( AnimationControl );

  AnimationControl ( QWidget *parent );
  virtual ~AnimationControl();

  // Possible states.
  enum State
  {
    PLAYING   = 0x00000001,
    FORWARD   = 0x00000002,
    BACKWARD  = 0x00000004
  };

  // Set/get the document.
  void                    document ( MinervaDocument::RefPtr );
  MinervaDocument::RefPtr document() const;

  void start();

  void stop();

  // Get the mutex.
  Mutex &                     mutex() const;

signals:

  void renderRequired();

public slots:
  
  void documentModified();

protected slots:

  void                        _onLoop ( bool );
  void                        _onPlayForward();
  void                        _onPlayBackward();
  void                        _onSliderChanged ( int );
  void                        _onSpeedChanged ( double );
  void                        _onStepForward();
  void                        _onStepBackward();
  void                        _onStopPlaying();

  void                        on_zoomTimeIn_clicked();
  void                        on_zoomTimeOut_clicked();

  // Called when the timer fires.
  void                        _onTimeout();

private:

  void                        _getTimeSpan();

  void                        _destroy();

  void                        _render();

  void                        _loopEvent ( bool );

  void                        _playForwardEvent();
  void                        _playBackwardEvent();

	void                        _setEnabledState();
  void                        _setState();
  void                        _setSliderValues();
  void                        _speedChangedEvent ( double );
  void                        _stepForward();
  void                        _stepForwardEvent();
  void                        _stepBackward();
  void                        _stepBackwardEvent();
  void                        _stopPlaying();
  void                        _stopPlayingEvent();

  void                        _startTimer();
  void                        _stopTimer();

  mutable Mutex *_mutex;
  MinervaDocument::RefPtr _document;
  AnimationController::RefPtr _controller;
  unsigned int _state;
  QTimerPtr _timer;
  double _milliSeconds;
  bool _loop;
  Ui::AnimationControl *_animationControl;
};


}
}


#endif // _MINERVA_QT_ANIMATION_CONTROL_PANEL_CLASS_H_
