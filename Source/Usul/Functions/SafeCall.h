
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Perry L Miller IV
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Wraps a function call with try-catch block. 
//
//  Note: sensible output in multi-threaded environment requires first 
//  building the std::ostringstream and then pushing this all at once to 
//  std::cout.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_FUNCTIONS_SAFE_CALL_H_
#define _USUL_FUNCTIONS_SAFE_CALL_H_

#include "Usul/Exceptions/Canceled.h"

#ifdef _MSC_VER
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include "windows.h"
# include "eh.h"
# include "Usul/Diagnostics/StackTraceWindows.h"
# include <vector>
#endif

#include <stdexcept>
#include <iostream>
#include <sstream>


namespace Usul {
namespace Functions {


///////////////////////////////////////////////////////////////////////////////
//
//  Temporarily register a new structured-exception translator.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

namespace Helper
{
  struct ScopedStructuredExceptionTranslator
  {
    ScopedStructuredExceptionTranslator() : _original ( 0x0 )
    {
      _original = ::_set_se_translator ( &ScopedStructuredExceptionTranslator::_translateException );
    }

    ~ScopedStructuredExceptionTranslator()
    {
      ::_set_se_translator ( _original );
    }

  private:

    static void _translateException ( unsigned int u, EXCEPTION_POINTERS *e )
    {
      std::ostringstream out;
      out << "Error 1153181950: Structured exception " << std::hex << u << " generated. Re-throwing as C++ exception.";

	    std::vector<std::string> stackTrace;
	    Usul::Diagnostics::Helper::getStackTrace ( stackTrace, *e->ContextRecord );
	    for ( unsigned int i = 0; i < stackTrace.size(); ++i )
	    {
        std::cout << "\t" << stackTrace[i] << std::endl;
	    }

      throw std::runtime_error ( out.str() );
    }

    _se_translator_function _original;
  };
}

#endif


///////////////////////////////////////////////////////////////////////////////
//
//  Helper functions for handling exceptions.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  inline void exceptionHandler ( const char *id, const Usul::Exceptions::Canceled & )
  {
  }
  inline void exceptionHandler ( const char *id, const std::exception &e )
  {
    std::ostringstream out;
    out << "Error " << ( ( 0x0 == id ) ? "3830268935" : id ) << ": " << e.what() << '\n';
    std::cout << out.str() << std::flush;
  }
  inline void exceptionHandler ( const char *id )
  {
    std::ostringstream out;
    out << "Error " << ( ( 0x0 == id ) ? "1129262471" : id ) << ": exception generated when calling wrapped function" << '\n';
    std::cout << out.str() << std::flush;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Macro for try blocks.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#define USUL_TRY_BLOCK Usul::Functions::Helper::ScopedStructuredExceptionTranslator _scoped_structured_exception_translator; try
#else
#define USUL_TRY_BLOCK try
#endif


///////////////////////////////////////////////////////////////////////////////
//
//  Macro for catch blocks.
//
///////////////////////////////////////////////////////////////////////////////

#define USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS(error_id) \
catch ( const Usul::Exceptions::Canceled &e ) \
{ \
  Usul::Functions::Helper::exceptionHandler ( error_id, e ); \
} \
catch ( const std::exception &e ) \
{ \
  Usul::Functions::Helper::exceptionHandler ( error_id, e ); \
} \
catch ( ... ) \
{ \
  Usul::Functions::Helper::exceptionHandler ( error_id ); \
}
  
#define USUL_DEFINE_CATCH_BLOCKS USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS


///////////////////////////////////////////////////////////////////////////////
//
//  Macro for catch blocks.
//
///////////////////////////////////////////////////////////////////////////////

template < class F > void safeCall ( F function, const char *id = 0x0 )
{
  USUL_TRY_BLOCK
  {
    function();
  }
  USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS ( id );
}


} // namespace Functions
} // namespace Usul


#endif // _USUL_FUNCTIONS_SAFE_CALL_H_
