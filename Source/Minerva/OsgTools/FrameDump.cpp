
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Frame-dump info.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __sgi
#define _CPP_CMATH 1
#endif

#include "Minerva/OsgTools/FrameDump.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace Minerva::OsgTools;


///////////////////////////////////////////////////////////////////////////////
//
//  Default constructor.
//
///////////////////////////////////////////////////////////////////////////////

FrameDump::FrameDump() : 
  _dump    ( FrameDump::NEVER_DUMP ),
  _dir     (), 
  _base    (), 
  _ext     (), 
  _start   (), 
  _digits  (), 
  _current ( _start )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

FrameDump::FrameDump ( FrameDump::DumpState dump,
                       const std::string &dir, 
                       const std::string &base, 
                       const std::string &ext, 
                       unsigned int start, 
                       unsigned int digits ) :
  _dump    ( dump ), 
  _dir     ( dir ), 
  _base    ( base ), 
  _ext     ( ext ), 
  _start   ( start ), 
  _digits  ( digits ), 
  _current ( start )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy constructor.
//
///////////////////////////////////////////////////////////////////////////////

FrameDump::FrameDump ( const FrameDump &f ) : 
  _dump    ( f._dump ), 
  _dir     ( f._dir ), 
  _base    ( f._base ),
  _ext     ( f._ext ), 
  _start   ( f._start ), 
  _digits  ( f._digits ), 
  _current ( f._current )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

FrameDump::~FrameDump()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Assignment.
//
///////////////////////////////////////////////////////////////////////////////

FrameDump &FrameDump::operator = ( const FrameDump &f )
{
  _dump    = f._dump;
  _dir     = f._dir;
  _base    = f._base;
  _ext     = f._ext;
  _start   = f._start;
  _digits  = f._digits;
  _current = f._current;
  return *this;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the current filename.
//
///////////////////////////////////////////////////////////////////////////////

std::string FrameDump::file() const
{
  // Make the zero-padded number.
  std::string number;
  {
    std::ostringstream out;
    out << std::setw ( _digits ) << _current;
    number = out.str();
    std::replace ( number.begin(), number.end(), ' ', '0' );
  }

  // Make the full path.
  std::string path;
  {
    std::ostringstream out;
    out << _dir << '/' << number << _base << _ext;
    path = out.str();
  }

  // Increment the counter.
  ++_current;

  // Return the file name.
  return path;
}
