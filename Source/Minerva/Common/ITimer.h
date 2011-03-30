
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_INTERFACES_TIMER_H_
#define _MINERVA_INTERFACES_TIMER_H_

#include "Usul/Interfaces/IUnknown.h"

#include "boost/function.hpp"

namespace Minerva {
namespace Common {


struct ITimer : public Usul::Interfaces::IUnknown
{
  // Typedefs.
  typedef boost::function<void ()> TimerCallback;

  // Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( ITimer );

  // Id for this interface.
  enum { IID = 2615516348u };

  // Start the timer.
  virtual void start() = 0;

  // Stop.
  virtual void stop() = 0;
};


} // namespace Usul
} // namespace Interfaces


#endif // _MINERVA_INTERFACES_TIMER_H_
