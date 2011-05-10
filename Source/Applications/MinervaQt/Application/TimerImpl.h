
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Timer implementation.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_QT_TIMER_IMPL_H__
#define __MINERVA_QT_TIMER_IMPL_H__

#include "Minerva/Common/ITimer.h"

#include "Usul/Threads/Guard.h"
#include "Usul/Threads/RecursiveMutex.h"

#include "QtCore/QTimer"

#include "boost/shared_ptr.hpp"

class TimerImpl : public QObject
{
  Q_OBJECT;

public:

  // Typedefs.
  typedef QObject BaseClass;
  typedef Usul::Threads::RecursiveMutex Mutex;
  typedef Usul::Threads::Guard<Mutex> Guard;
  typedef Minerva::Common::ITimer::TimerCallback Callback;
  typedef boost::shared_ptr < QTimer > QTimerPtr;

  TimerImpl ( unsigned int milliseconds, Callback callback, bool singleShot );
  virtual ~TimerImpl();
  
  void start();
  void stop();

private slots:

  void _start();
  void _stop();
  void _onTimeout();

private:

  // No copying or assignment.
  TimerImpl ( const TimerImpl & );
  TimerImpl &operator = ( const TimerImpl & );

  void                      _destroy();

  mutable Mutex _mutex;
  unsigned int _milliseconds;
  QTimerPtr _timer;
  Callback _callback;
};

#endif // __MINERVA_QT_TIMER_IMPL_H__
