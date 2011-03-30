
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Defines assert.
//
///////////////////////////////////////////////////////////////////////////////

#include "Usul/Errors/Assert.h"
#include "Usul/Strings/Format.h"
#include "Usul/Threads/ThreadId.h"

#include <stdexcept>
#include <cstdio>

///////////////////////////////////////////////////////////////////////////////
//
//  Function to handle assert in debug.
//
///////////////////////////////////////////////////////////////////////////////

namespace Usul
{
  namespace Errors
  {
    void _assert_debug ( bool state, const char *exp, const char *file, int line )
    {
      if ( false == state )
      {
        const std::string message ( Usul::Strings::format ( 
          "Assertion in file: ", file, 
          ", Line: ", line, 
          ", Expression: ", ( ( 0x0 == exp ) ? "" : exp ), 
          ", System thread id: ", Usul::Threads::currentThreadId() ) );
        ::printf ( "%s", message.c_str() );
        ::printf ( "\n" ); 
        ::fflush ( stdout );

        throw std::runtime_error ( message );
      }
    }
  }
}

