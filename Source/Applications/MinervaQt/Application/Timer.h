
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Internal timer callback.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_QT_CORE_TIMER_CALLBACK_H_
#define _HELIOS_QT_CORE_TIMER_CALLBACK_H_

#include "Minerva/Common/ITimer.h"

#include "Usul/Base/Object.h"

#include "QtCore/QTimer"

class TimerImpl;

class Timer : 
  public Usul::Base::Object,
  public Minerva::Common::ITimer
{
public:

  // Typedefs.
  typedef Usul::Base::Object BaseClass;
  typedef Minerva::Common::ITimer::TimerCallback Callback;

  USUL_DECLARE_QUERY_POINTERS ( Timer );
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  Timer ( unsigned int milliseconds, Callback callback, bool singleShot );

  // Start the timer.
  void start();
  
  // Stop the timer.
  void stop();

protected:

  virtual ~Timer();
  
private:

  // No copying or assignment.
  Timer ( const Timer & );
  Timer &operator = ( const Timer & );

  void                      _destroy();

  TimerImpl *_impl;
};


#endif // _HELIOS_QT_CORE_TIMER_CALLBACK_H_
