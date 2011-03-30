
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_INTERFACES_TIMER_FACTORY_H_
#define _MINERVA_INTERFACES_TIMER_FACTORY_H_

#include "Usul/Interfaces/IUnknown.h"

#include "Minerva/Common/ITimer.h"

namespace Minerva {
namespace Common {


struct ITimerFactory : public Usul::Interfaces::IUnknown
{
  // Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( ITimerFactory );

  // Id for this interface.
  enum { IID = 2221836210u };

  // Add a timer.
  virtual ITimer::RefPtr createTimer ( unsigned int milliseconds, ITimer::TimerCallback callback, bool singleShot = false ) = 0;
};


} // namespace Minerva
} // namespace Interfaces


#endif // _MINERVA_INTERFACES_TIMER_FACTORY_H_

