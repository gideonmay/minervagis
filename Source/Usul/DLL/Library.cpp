
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class that represents a dynamically loaded library.
//
///////////////////////////////////////////////////////////////////////////////

#include "Usul/DLL/Library.h"
#include "Usul/DLL/Exceptions.h"
#include "Usul/System/LastError.h"
#include "Usul/Threads/Variable.h"
#include "Usul/Threads/Guard.h"

#ifdef _WIN32
# define NOMINMAX
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
# endif
# include <windows.h>
#else
# include <dlfcn.h>
#endif

#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>

using namespace Usul;
using namespace Usul::DLL;


///////////////////////////////////////////////////////////////////////////////
//
//  The details...
//
///////////////////////////////////////////////////////////////////////////////

namespace Usul {
namespace DLL {
namespace Detail {


///////////////////////////////////////////////////////////////////////////////
//
//  Load the library.
//
///////////////////////////////////////////////////////////////////////////////

Library::ModuleHandle _load ( const std::string &filename )
{
  // Handle bad string.
  if ( filename.empty() )
    return 0x0;

#ifdef _WIN32
  return ::LoadLibrary ( filename.c_str() );
#else
  return ::dlopen ( filename.c_str(), RTLD_LAZY | RTLD_GLOBAL );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Free the library.
//
///////////////////////////////////////////////////////////////////////////////

bool _free ( Library::ModuleHandle module )
{
  // Null modules are already free. Do not change this without 
  // also changing the code that calls this.
  if ( 0x0 == module )
    return true;

#ifdef _WIN32
  return ( 0 != ::FreeLibrary ( reinterpret_cast<HMODULE> ( module ) ) );
#else
  return ( 0 == ::dlclose ( module ) );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the address of the function.
//
///////////////////////////////////////////////////////////////////////////////

Library::Function _function ( const std::string &name, Library::ModuleHandle module )
{
  // Handle bad module.
  if ( 0x0 == module )
    return 0x0;

  // Handle bad string.
  if ( name.empty() )
    return 0x0;

#ifdef _WIN32
  return ::GetProcAddress ( reinterpret_cast<HMODULE> ( module ), name.c_str() );
#else
  return ::dlsym ( module, name.c_str() );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  End of the details.
//
///////////////////////////////////////////////////////////////////////////////

} // namespace Detail
} // namespace DLL
} // namespace Usul


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Library::Library ( const std::string &filename ) : BaseClass(),
  _module   ( 0x0 ),
  _filename ( filename )
{
  // Initialize the last error.
  System::LastError::init();

  // Load the library.
  _module = Usul::DLL::Detail::_load ( filename );

  // See if it worked.
  if ( 0x0 == _module )
  {
    std::ostringstream out;
    out << "Error: 3942529130, failed to load library: " << _filename 
        << "\n\tSystem error number: "  << System::LastError::number()
        << "\n\tSystem error message: " << System::LastError::message();
    throw Usul::DLL::Exceptions::FailedToLoad ( out.str() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Library::~Library()
{
  try
  {
    // Initialize the last error.
    System::LastError::init();

    // Free the library.
    bool success = Usul::DLL::Detail::_free ( _module );

    // Make sure it worked.
    if ( false == success )
    {
      std::ostringstream out;
      out << "Error: 4286324342, failed to free library: " << _filename 
          << "\n\tSystem error number: "  << System::LastError::number()
          << "\n\tSystem error message: " << System::LastError::message();
      std::cout << out << std::endl;
    }
  }

  catch ( const std::exception &e )
  {
    std::cout << "Error 3130313481: Standard exception caught while releasing: " << _filename << std::endl;
    if ( e.what() )
      std::cout << ". " << e.what() << std::endl;
  }
  catch ( ... )
  {
    std::cout << "Error 3947493394: Unknown exception caught while releasing: " << _filename << std::endl;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the address of the function.
//
///////////////////////////////////////////////////////////////////////////////

Library::Function Library::function ( const std::string &name ) const
{
  Function fun ( Usul::DLL::Detail::_function ( name, _module ) );
  return fun;
}
