
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Internal timer callback.
//
///////////////////////////////////////////////////////////////////////////////

#include "TimerImpl.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Threads/Named.h"
#include "Usul/Threads/Safe.h"

#include "QtCore/QCoreApplication"

#include "boost/bind.hpp"


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

TimerImpl::TimerImpl ( unsigned int milliseconds, Callback callback, bool singleShot ) : BaseClass(),
  _mutex(),
  _milliseconds ( milliseconds ),
  _timer(),
  _callback ( callback )
{
  // Make the timer.
  _timer.reset ( new QTimer );
  
  // Move to the main thread.
  this->moveToThread ( QCoreApplication::instance()->thread() );
  _timer->moveToThread ( QCoreApplication::instance()->thread() );
  
  // Set the parent. Do this after moving to the main thread.
  _timer->setParent ( this );

  // Set the connection.
  this->connect ( _timer.get(), SIGNAL ( timeout() ), this, SLOT ( _onTimeout() ) );

  // Is this a single-shot timer?
  _timer->setSingleShot ( singleShot );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

TimerImpl::~TimerImpl()
{
  Usul::Functions::safeCall ( boost::bind ( &TimerImpl::_destroy, this ), "4218934207" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor contents.
//
///////////////////////////////////////////////////////////////////////////////

void TimerImpl::_destroy()
{
  // Stop the timer if it's running.
  this->stop();

  // Disconnect the slot.
  this->disconnect();

  // This should delete the timer.
  _timer.reset();

  // Clear the callback.
  _callback = Callback();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Start the timer.
//
///////////////////////////////////////////////////////////////////////////////

void TimerImpl::start()
{
  if ( false == Usul::Threads::Named::instance().is ( Usul::Threads::Names::GUI ) )
  {
    if ( false == QMetaObject::invokeMethod ( this, "_start", Qt::QueuedConnection ) )
    {
      std::cout << "Warning 7474543440: could not start the timer." << std::endl;
    }
  }
  else
  {
    this->_start();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop the timer.
//
///////////////////////////////////////////////////////////////////////////////

void TimerImpl::stop()
{
  if ( false == Usul::Threads::Named::instance().is ( Usul::Threads::Names::GUI ) )
    QMetaObject::invokeMethod ( this, "_stop", Qt::QueuedConnection );
  else
    this->_stop();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Timer slot.
//
///////////////////////////////////////////////////////////////////////////////

void TimerImpl::_onTimeout()
{
  Callback callback ( _callback );

  if ( callback )
  {
    Usul::Functions::safeCall ( callback, "4014661170" );
  }

  {
    Guard guard ( _mutex );
    {
      if ( ( 0x0 != _timer ) && ( true == _timer->isSingleShot() ) )
      {
        _timer->start();
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Start the timer.
//
///////////////////////////////////////////////////////////////////////////////

void TimerImpl::_start()
{
  Guard guard ( _mutex );
  
  // Make sure the timer is valid.
  if ( 0x0 == _timer.get() )
    return;
  
  // Is the timer already running?
  if ( true == _timer->isActive() )
    return;
  
  // Start the timer.
  _timer->setInterval ( _milliseconds );
  _timer->start();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop the timer.
//
///////////////////////////////////////////////////////////////////////////////

void TimerImpl::_stop()
{
  Guard guard ( _mutex );

  // Make sure the timer is valid.
  if ( 0x0 == _timer.get() )
    return;

  // Is the timer running?
  if ( false == _timer->isActive() )
    return;

  // Stop the timer.
  _timer->stop();
}
