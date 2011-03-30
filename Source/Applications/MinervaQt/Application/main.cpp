
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Minerva program.
//
///////////////////////////////////////////////////////////////////////////////

#include "Program.h"

#include "Usul/Functions/SafeCall.h"
#include "Usul/System/DateTime.h"

#include <string>


///////////////////////////////////////////////////////////////////////////////
//
//  Main function.
//
///////////////////////////////////////////////////////////////////////////////

int main ( int argc, char **argv )
{
  // Initialize the result.
  int result ( 1 );

  // Branding.
  const std::string program ( "Minerva" );
  const std::string version ( Usul::System::DateTime::version ( __DATE__ ) );
  const std::string vendor  ( "CadKit" );
  const std::string url     ( "www.minerva-gis.org" );
  const std::string icon    ( "minerva_icon.png" );
  const std::string splash  ( "minerva_splash.png" );

  // Other configurations.
  // Since Minerva documents have their own thread pool for tiling, use a small number here.
  const unsigned int jobManagerThreadPoolSize ( 2 );

  try
  {
    // Run the application.
    Program::run (
      argc, argv,
      program, version, vendor, url, icon, splash,
      jobManagerThreadPoolSize,
      result );
  }

  // Catch exceptions.
  USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS ( "1192223129" );

  // Return the result.
  return result;
}
