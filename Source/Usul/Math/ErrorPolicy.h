
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Error-checking class that throws.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_MATH_ERROR_POLICY_THROWER_H_
#define _USUL_MATH_ERROR_POLICY_THROWER_H_

#include "Usul/Exceptions/Thrower.h"

#include <stdexcept>

namespace Usul {
namespace Math {

template < typename Int_ >
struct ErrorPolicy
{
  static void bounds ( unsigned int num, Int_ size, Int_ index )
  {
    if ( index < 0 || index >= size )
      Usul::Exceptions::Thrower < std::runtime_error > ( "Error ", num, ": Index out of range" );
  }
};

template<>
struct ErrorPolicy<unsigned int>
{
  static void bounds ( unsigned int num, unsigned int size, unsigned int index )
  {
    if ( index >= size )
      Usul::Exceptions::Thrower < std::runtime_error > ( "Error ", num, ": Index out of range" );
  }
};

} // namespace Errors
} // namespace Usul


#endif // _USUL_MATH_ERROR_POLICY_THROWER_H_
