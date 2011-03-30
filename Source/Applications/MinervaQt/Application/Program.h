
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Helios program class. Handles startup, shutdown, and branding.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _CADKIT_HELIOS_QT_CORE_PROGRAM_H_
#define _CADKIT_HELIOS_QT_CORE_PROGRAM_H_

#include <string>


struct Program
{
  static void run ( int argc, char **argv,
                    const std::string &program, 
                    const std::string &version,
                    const std::string &vendor, 
                    const std::string &url, 
                    const std::string &icon, 
                    const std::string &splash,
                    unsigned int poolSize,
                    int &result );
};


#endif // _CADKIT_HELIOS_QT_CORE_PROGRAM_H_
