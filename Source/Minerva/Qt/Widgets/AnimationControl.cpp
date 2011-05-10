
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

#include "ui_AnimationControl.h" // Cannot have path here.
#include "Minerva/Qt/Widgets/AnimationControl.h"

#include "Minerva/Qt/Tools/ScopedSignals.h"

#include "Usul/Bits/Bits.h"
#include "Usul/Components/Manager.h"
#include "Usul/Errors/Assert.h"
#include "Usul/Exceptions/Thrower.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Threads/Named.h"

#include "QtCore/QTimer"

#include "boost/bind.hpp"

using namespace Minerva::QtWidgets;

const unsigned int HOURS_PER_TIMESTEP ( 8 );

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

AnimationControl::AnimationControl ( QWidget *parent  ) : BaseClass ( parent ),
  _mutex        ( new AnimationControl::Mutex ),
  _document     (),
  _controller   ( new Minerva::Document::AnimationController ),
  _state        ( 0 ),
  _timer        (),
  _milliSeconds ( 1000 ),
  _loop         ( false ),
  _animationControl ( new Ui::AnimationControl )
{
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "1692127533" );

  // Initialize code from Designer.
  _animationControl->setupUi ( this );

  // Set gui members before attaching slots.
  _animationControl->_loopCheckBox->setChecked ( _loop );
  _animationControl->_speedSpinBox->setValue ( _milliSeconds / 1000.0 );

	// Set the enabled state.
	this->_setEnabledState();

  // Connect slots.
  QObject::connect ( _animationControl->_playForwardButton,  SIGNAL ( clicked() ), this, SLOT ( _onPlayForward()  ) );
  QObject::connect ( _animationControl->_playBackwardButton, SIGNAL ( clicked() ), this, SLOT ( _onPlayBackward() ) );
  QObject::connect ( _animationControl->_stepForwardButton,  SIGNAL ( clicked() ), this, SLOT ( _onStepForward()  ) );
  QObject::connect ( _animationControl->_stepBackwardButton, SIGNAL ( clicked() ), this, SLOT ( _onStepBackward() ) );
  QObject::connect ( _animationControl->_stopButton,         SIGNAL ( clicked() ), this, SLOT ( _onStopPlaying()  ) );
  QObject::connect ( _animationControl->_loopCheckBox, SIGNAL ( toggled ( bool ) ), this, SLOT ( _onLoop ( bool ) ) );
  QObject::connect ( _animationControl->_speedSpinBox, SIGNAL ( valueChanged ( double ) ), this, SLOT ( _onSpeedChanged ( double ) ) );
  QObject::connect ( _animationControl->_sliderSlider, SIGNAL ( valueChanged ( int ) ), this, SLOT ( _onSliderChanged( int ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

AnimationControl::~AnimationControl()
{
  Usul::Functions::safeCall ( boost::bind ( &AnimationControl::_destroy, this ), "3603850681" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destroy.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_destroy()
{
  // Need local scope.
  {
    Guard guard ( this );

    // Remove the timer.
    this->_stopTimer();

    // Done with these.
    _document = 0x0;
  }
  
  delete _animationControl; _animationControl = 0x0;

  // Do this now.
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "2023346938" );

  // Done with mutex.
  delete _mutex; _mutex = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the mutex.
//
///////////////////////////////////////////////////////////////////////////////

AnimationControl::Mutex &AnimationControl::mutex() const
{
  return *_mutex;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the state.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_setState()
{
  Guard guard ( this );
  
  // Disconnect the slots and make sure they are re-connected.
  QtTools::ScopedSignals scopedSignals ( *this );
  
  this->_getTimeSpan();
  
  // Set the enabled state.
	this->_setEnabledState();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Macros to make slot.
//
///////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_SLOT_0(slot_name,function_name,error_id) \
void AnimationControl::slot_name() \
{ \
  USUL_THREADS_ENSURE_GUI_THREAD ( return ); \
  Usul::Functions::safeCall ( boost::bind ( &AnimationControl::function_name, this ), error_id ); \
}

#define IMPLEMENT_SLOT_1(slot_name,function_name,error_id,argument_type) \
void AnimationControl::slot_name ( argument_type the_value ) \
{ \
  USUL_THREADS_ENSURE_GUI_THREAD ( return ); \
  Usul::Functions::safeCall ( boost::bind ( &AnimationControl::function_name, this, the_value ), error_id ); \
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make slots.
//
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SLOT_0 ( _onPlayForward,  _playForwardEvent,  "2991793146" );
IMPLEMENT_SLOT_0 ( _onPlayBackward, _playBackwardEvent, "3231596619" );
IMPLEMENT_SLOT_0 ( _onStepForward,  _stepForwardEvent,  "1110757894" );
IMPLEMENT_SLOT_0 ( _onStepBackward, _stepBackwardEvent, "1598944220" );
IMPLEMENT_SLOT_0 ( _onStopPlaying,  _stopPlayingEvent,  "7441062000" );
IMPLEMENT_SLOT_1 ( _onLoop,         _loopEvent,         "1080906911", bool );
IMPLEMENT_SLOT_1 ( _onSpeedChanged, _speedChangedEvent, "1431849635", double );


///////////////////////////////////////////////////////////////////////////////
//
//  Start the timer.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_startTimer()
{
  Guard guard ( this );

  this->_stopTimer();

  // Handle repeated calls.
  if ( _timer )
    return;

  _timer.reset ( new QTimer );
  
  _timer->setInterval ( _milliSeconds );
  _timer->start();

  // Set the connection.
  this->connect ( _timer.get(), SIGNAL ( timeout() ), this, SLOT ( _onTimeout() ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop the timer.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_stopTimer()
{
  Guard guard ( this );
  
  // Handle repeated calls.
  if ( 0x0 == _timer )
    return;
  
  // Clear the timer.  This is important!
  _timer->stop();
  _timer.reset();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Callback for the timer.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_onTimeout()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  Guard guard ( this );

  if ( false == Usul::Bits::has ( _state, AnimationControl::PLAYING ) )
  {
    return;
  }

  if ( true == Usul::Bits::has ( _state, AnimationControl::FORWARD ) )
  {
    this->_stepForward();
    return;
  }

  if ( true == Usul::Bits::has ( _state, AnimationControl::BACKWARD ) )
  {
    this->_stepBackward();
    return;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play the animation forward.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_playForwardEvent()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  Guard guard ( this );

  // Set the state.
  _state = AnimationControl::PLAYING | AnimationControl::FORWARD;

  // Start the timer.
  this->_startTimer();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play the animation backward.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_playBackwardEvent()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  Guard guard ( this );

  // Set the state.
  _state = AnimationControl::PLAYING | AnimationControl::BACKWARD;

  // Start the timer.
  this->_startTimer();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Move forward one step.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_stepForwardEvent()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  Guard guard ( this );

  // Set the state.
  _state = 0;

  // Make sure the timer is stopped.
  this->_stopTimer();

  // Step forward.
  this->_stepForward();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Move backward one step.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_stepBackwardEvent()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  Guard guard ( this );

  // Set the state.
  _state = 0;

  // Make sure the timer is stopped.
  this->_stopTimer();

  // Step backward.
  this->_stepBackward();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop playing the animation.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_stopPlayingEvent()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  this->_stopPlaying();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop playing the animation.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_stopPlaying()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  Guard guard ( this );

  // Set the state.
  _state = 0;

  // Make sure the timer is stopped.
  this->_stopTimer();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Move forward one step.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_stepForward()
{
  Guard guard ( this );

  AnimationController::AnimationResult result ( _controller->stepForward() );

  // Set the new visible timespan.
  if ( _document.valid() )
  {
    _document->visibleTimeSpan ( _controller->visibleTimeSpan() );
  }

  // Set the slider value.
  {
    QtTools::ScopedSignals scoped ( *_animationControl->_sliderSlider );
    _animationControl->_sliderSlider->setValue ( _controller->getCurrentTimeStep() );
  }

  // If we're on the last one, and we're not supposed to loop, then we're done.
  if ( AnimationController::ANIMATION_RESULT_AT_END == result )
  {
    if ( false == _loop )
    {
      this->_stopPlaying();
    }
    else
    {
      _controller->resetVisibleTimeSpan();
    }
  }

  // Render the views now.
  this->_render();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Move backward one step.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_stepBackward()
{
  AnimationController::AnimationResult result ( _controller->stepBackward() );

  // Set the new visible timespan.
  if ( _document.valid() )
  {
    _document->visibleTimeSpan ( _controller->visibleTimeSpan() );
  }

  // Set the slider value.
  {
    QtTools::ScopedSignals scoped ( *_animationControl->_sliderSlider );
    _animationControl->_sliderSlider->setValue ( _controller->getCurrentTimeStep() );
  }

  // If we're on the first one, and we're not supposed to loop, then we're done.
  if ( AnimationController::ANIMATION_RESULT_AT_BEGIN == result )
  {
    if ( false == _loop )
    {
      this->_stopPlaying();
    }
    else
    {
      _controller->resetVisibleTimeSpan();
    }
  }

  // Render the views now.
  this->_render();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the document's modified flag.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_render()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  emit renderRequired();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Called when the button is pressed.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_loopEvent ( bool state )
{
  Guard guard ( this );
  _loop = state;
}


///////////////////////////////////////////////////////////////////////////////
//
//  The property changed.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_speedChangedEvent ( double speed )
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );
  Guard guard ( this );

  _milliSeconds = speed * 1000.0;

  if ( 0 != _timer )
  {
    this->_stopTimer();
    this->_startTimer();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the enabled state.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_setEnabledState()
{
	// Set the values.
  const bool enabled ( true == _document.valid() );
  _animationControl->_playForwardButton->setEnabled ( enabled );
  _animationControl->_playBackwardButton->setEnabled ( enabled );
  _animationControl->_stepForwardButton->setEnabled ( enabled );
  _animationControl->_stepBackwardButton->setEnabled ( enabled );
  _animationControl->_stopButton->setEnabled ( enabled );
  _animationControl->_loopCheckBox->setEnabled ( enabled );
  _animationControl->_sliderSlider->setEnabled ( enabled );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The slider has changed.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_onSliderChanged ( int value )
{
  if ( value > 0 )
  {
    _controller->setCurrentTimeStep ( value );

    if ( _document )
    {
      _document->visibleTimeSpan ( _controller->visibleTimeSpan() );
    }

    this->_render();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the document.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::document ( MinervaDocument::RefPtr doc )
{
  Guard guard ( this );
  _document = doc;
  this->_setState();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the document.
//
///////////////////////////////////////////////////////////////////////////////

AnimationControl::MinervaDocument::RefPtr AnimationControl::document() const
{
  Guard guard ( this );
  return _document;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Start the animation.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::start()
{
  this->_onPlayForward();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop the animation.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::stop()
{
  this->_onStopPlaying();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Document has been modified, update new timespan.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::documentModified()
{
  this->_getTimeSpan();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the time span from the document.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_getTimeSpan()
{
  this->_stopPlaying();

  if ( _document.valid() )
  {
    Minerva::Core::Data::TimeSpan::RefPtr timeSpan ( _document->timeSpanOfData() );
    _controller->globalTimeSpan ( timeSpan );

    _document->visibleTimeSpan ( _controller->visibleTimeSpan() );

    const unsigned int numberOfSteps ( _controller->getNumberOfTimeSteps() );
    _animationControl->_sliderSlider->setRange ( 0, ( 0 != numberOfSteps ? numberOfSteps - 1 : 0 ) );
    _animationControl->_sliderSlider->setValue ( 0 );

    if ( timeSpan.valid() )
    {
      _animationControl->minDateLabel->setText ( timeSpan->begin().toString().c_str() );
      _animationControl->maxDateLabel->setText ( timeSpan->end().toString().c_str() );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Decrease the time step size.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::on_zoomTimeIn_clicked()
{
  _controller->decreaseStepSize();

  this->_setSliderValues();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Increase the time step size.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::on_zoomTimeOut_clicked()
{
  _controller->increaseStepSize();

  this->_setSliderValues();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the slider values.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationControl::_setSliderValues()
{
  const unsigned int numberOfSteps ( _controller->getNumberOfTimeSteps() );
  _animationControl->_sliderSlider->setRange ( 0, ( 0 != numberOfSteps ? numberOfSteps - 1 : 0 ) );
  _animationControl->_sliderSlider->setValue ( _controller->getCurrentTimeStep() );
}
