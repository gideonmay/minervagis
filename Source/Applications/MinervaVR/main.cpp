
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, John K. Grant and Perry L. Miller IV.
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "VRV/Core/Application.h"

#include "VRV/Core/Program.h"


///////////////////////////////////////////////////////////////////////////////
//
//  Procter & Gamble -- Virtual Reality Viewer.
//
///////////////////////////////////////////////////////////////////////////////

int main ( int argc, char **argv )
{
  VRV::Core::Program < VRV::Core::Application > program ( "MinervaVR" );

  // Isolate application run inside this function.
  return program.run ( argc, argv );
}
