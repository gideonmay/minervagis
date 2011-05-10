
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Timer server.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_QT_CORE_TIMER_SERVER_H_
#define _HELIOS_QT_CORE_TIMER_SERVER_H_

#include "Timer.h"

#include "Usul/Base/Object.h"
#include "Minerva/Common/ITimerFactory.h"

#include "boost/tuple/tuple.hpp"

#include <list>
#include <map>


class TimerServer : public Usul::Base::Object,
                    public Minerva::Common::ITimerFactory
{
public:

  // Typedefs.
  typedef Usul::Base::Object BaseClass;
  typedef Minerva::Common::ITimer ITimer;

  // Constructor.
  TimerServer();

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( TimerServer );

  // Usul::Interfaces::IUnknown members.
	USUL_DECLARE_IUNKNOWN_MEMBERS;

  // Usul::Interfaces::ITimerService.
  virtual ITimer::RefPtr createTimer ( unsigned int milliseconds, ITimer::TimerCallback callback, bool singleShot = false );
  
protected:

  // Use reference counting.
	virtual ~TimerServer();

private:

  // No copying or assignment.
  TimerServer ( const TimerServer & );
  TimerServer &operator = ( const TimerServer & );

  void                      _destroy();

};


#endif // _HELIOS_QT_CORE_TIMER_SERVER_H_
