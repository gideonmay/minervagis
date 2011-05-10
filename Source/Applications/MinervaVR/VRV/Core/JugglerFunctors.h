
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author(s): Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Functors that work with Juggler.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VRV_CORE_JUGGLER_FUNCTORS_H_
#define _VRV_CORE_JUGGLER_FUNCTORS_H_

#include "VRV/Core/Exceptions.h"

#include "vrj/Kernel/Kernel.h"

#include "Usul/Exceptions/Thrower.h"

#include "boost/filesystem.hpp"

namespace VRV {
namespace Core {
namespace Detail {


///////////////////////////////////////////////////////////////////////////////
//
//  Functor that loads a config file.
//
///////////////////////////////////////////////////////////////////////////////

struct LoadConfigFile
{
  void operator () ( const std::string &filename ) const
  {
    // If the file exists...
    if ( boost::filesystem::exists ( filename ) )
    {
      std::cout << "Loading VRJuggler configuration file: " 
                << filename 
                << std::endl;

      // Crashes with VC7 if we don't pass internal char-array.
      vrj::Kernel::instance()->loadConfigFile ( filename.c_str() );
    }

    // Otherwise, we failed.
    else
    {
      Usul::Exceptions::Thrower < VRV::Core::Exceptions::UserInput > 
        ( "VRJuggler config file '", filename, "' does not exist." );
    }
  }
};


} // namespace Detail
} // namespace Core
} // namespace CV


#endif // _VRV_CORE_JUGGLER_FUNCTORS_H_
