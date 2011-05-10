///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Controller for animations.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Document/AnimationController.h"

const unsigned int HOURS_PER_TIMESTEP ( 8 );

using namespace Minerva::Document;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

AnimationController::AnimationController() : BaseClass(),
  _globalTimeSpan(),
  _visibleTimeSpan(),
  _stepAmount ( boost::posix_time::hours ( HOURS_PER_TIMESTEP ) )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

AnimationController::~AnimationController()
{
}

 
///////////////////////////////////////////////////////////////////////////////
//
//  Set the global time span.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationController::globalTimeSpan ( TimeSpan::RefPtr timeSpan )
{
  _globalTimeSpan = timeSpan;
  this->resetVisibleTimeSpan();
}

///////////////////////////////////////////////////////////////////////////////
//
//  Set the global time span.
//
///////////////////////////////////////////////////////////////////////////////

AnimationController::TimeSpan::RefPtr AnimationController::globalTimeSpan() const
{
  return _globalTimeSpan;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the visible time span.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationController::visibleTimeSpan ( TimeSpan::RefPtr timeSpan )
{
  _visibleTimeSpan = timeSpan;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the visible time span.
//
///////////////////////////////////////////////////////////////////////////////

AnimationController::TimeSpan::RefPtr AnimationController::visibleTimeSpan() const
{
  return _visibleTimeSpan;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Reset the visible time span.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationController::resetVisibleTimeSpan()
{
  TimeSpan::RefPtr global ( this->globalTimeSpan() );
  
  if ( global.valid() )
  {
    TimeSpan::RefPtr visible ( new TimeSpan ( global->begin(), global->begin() ) );
    this->visibleTimeSpan ( visible );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the number of timesteps.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int AnimationController::getNumberOfTimeSteps() const
{
  TimeSpan::RefPtr global ( this->globalTimeSpan() );
  
  if ( global.valid() )
  {
    Minerva::Core::Data::Date begin ( global->begin() );
    Minerva::Core::Data::Date end ( global->end() );

    if ( !begin.date().is_special() && !end.date().is_special() )
    {
      boost::posix_time::time_period period ( begin.date(), end.date() );
      boost::posix_time::time_duration duration ( period.length() );
      const unsigned int numberOfSteps ( duration.hours() / _stepAmount.hours() );
      return numberOfSteps;
    }
  }

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the current time step.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int AnimationController::getCurrentTimeStep() const
{
  TimeSpan::RefPtr global ( this->globalTimeSpan() );
  TimeSpan::RefPtr visible ( this->visibleTimeSpan() );

  if ( !global.valid() || !visible.valid() )
    return 0;

  Minerva::Core::Data::Date begin ( global->begin() );
  Minerva::Core::Data::Date current ( visible->end() );

  if ( !begin.date().is_special() && !current.date().is_special() )
  {
    boost::posix_time::time_period period ( begin.date(), current.date() );
    boost::posix_time::time_duration duration ( period.length() );

    return duration.hours() / _stepAmount.hours();
  }

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the current time step.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationController::setCurrentTimeStep ( unsigned int num )
{
  TimeSpan::RefPtr global ( this->globalTimeSpan() );
  TimeSpan::RefPtr visible ( this->visibleTimeSpan() );

  if ( !global.valid() || !visible.valid() )
    return;

  boost::posix_time::ptime date ( global->begin().date() );
  date = date + ( _stepAmount * num );

  visible->begin ( date );
  visible->end ( date );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Step backward.
//
///////////////////////////////////////////////////////////////////////////////

AnimationController::AnimationResult AnimationController::stepBackward()
{
  TimeSpan::RefPtr global ( this->globalTimeSpan() );
  TimeSpan::RefPtr visible ( this->visibleTimeSpan() );

  if ( !global.valid() || !visible.valid() )
    return ANIMATION_RESULT_ERROR;

  boost::posix_time::ptime date ( _visibleTimeSpan->end().date() );
  date = date - _stepAmount;

  const bool first ( date <= _globalTimeSpan->begin().date() );
  if ( first )
  {
    this->_setVisibleTimeSpan ( global->begin(), global->begin() );
    return ANIMATION_RESULT_AT_BEGIN;
  }

  this->_setVisibleTimeSpan ( date, date );

  return ANIMATION_RESULT_CONTINUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Step forward.
//
///////////////////////////////////////////////////////////////////////////////

AnimationController::AnimationResult AnimationController::stepForward()
{
  TimeSpan::RefPtr global ( this->globalTimeSpan() );
  TimeSpan::RefPtr visible ( this->visibleTimeSpan() );

  if ( !global.valid() || !visible.valid() )
    return ANIMATION_RESULT_ERROR;

  boost::posix_time::ptime date ( _visibleTimeSpan->end().date() );
  date = date + _stepAmount;

  const bool last ( date >= _globalTimeSpan->end().date() );
  if ( last )
  {
    this->_setVisibleTimeSpan ( global->end(), global->end() );
    return ANIMATION_RESULT_AT_END;
  }

  this->_setVisibleTimeSpan ( date, date );

  return ANIMATION_RESULT_CONTINUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Increase the step size.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationController::increaseStepSize()
{
  _stepAmount = _stepAmount * 2;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Decrease the step size.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationController::decreaseStepSize()
{
  if ( _stepAmount.hours() > 1 )
  {
    _stepAmount = _stepAmount / 2;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the visible time span.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationController::_setVisibleTimeSpan ( const Minerva::Core::Data::Date& begin, const Minerva::Core::Data::Date& end )
{
  _visibleTimeSpan->begin ( begin );
  _visibleTimeSpan->end ( end );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the step size.
//
///////////////////////////////////////////////////////////////////////////////

void AnimationController::setStepSize ( unsigned int hours )
{
  _stepAmount = boost::posix_time::hours ( hours );
}
