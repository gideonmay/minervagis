
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Policy class that does nothing.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_POINTERS_DO_NOTHING_POLICY_H_
#define _USUL_POINTERS_DO_NOTHING_POLICY_H_

#include "boost/concept_check.hpp"

namespace Usul {
namespace Pointers {


struct DoNothingPolicy
{
  template < class T > static void check ( const T *t )
  {
    boost::ignore_unused_variable_warning ( t );
  }
};


} // namespace Pointers
} // namespace Usul


#endif // _USUL_POINTERS_DO_NOTHING_POLICY_H_
