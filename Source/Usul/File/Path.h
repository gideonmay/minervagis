
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Functions for working with a file path.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_FILE_PATH_H_
#define _USUL_FILE_PATH_H_

#include "boost/filesystem.hpp"
#include "boost/version.hpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <cstring>

#ifndef _MSC_VER
#include <limits.h>
#endif


namespace Usul {
namespace File {


///////////////////////////////////////////////////////////////////////////////
//
//  Get the directory from the path.
//
///////////////////////////////////////////////////////////////////////////////

inline std::string directory ( const std::string &path )
{
#if BOOST_VERSION >= 104600
  return boost::filesystem::path ( path ).parent_path().string();
#else
  return boost::filesystem::path ( path ).parent_path().directory_string();
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the full path name.
//
///////////////////////////////////////////////////////////////////////////////

inline std::string fullPath ( const std::string &file )
{
  typedef std::string::value_type Char;

#ifdef _MSC_VER

  const size_t size ( _MAX_PATH );

#else

  const size_t size ( PATH_MAX );

#endif

  Char buffer[size];
  ::memset ( buffer, '\0', size - 1 );

  char* result ( 0x0 );

#ifdef _MSC_VER

  // http://msdn.microsoft.com/en-us/library/506720ff(VS.80).aspx
  result = ::_fullpath ( buffer, file.c_str(), size - 1 );

#else

  // http://linux.about.com/library/cmd/blcmdl3_realpath.htm
  result = ::realpath ( file.c_str(), buffer );

#endif

  // Return an empty string if we didn't get a result.  Should we throw an exception?
  if ( 0x0 == result )
    return std::string ( "" );

  // Return the answer.
  return std::string ( buffer );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extension from the path.
//
///////////////////////////////////////////////////////////////////////////////

inline std::string extension ( const std::string &path )
{
#if BOOST_VERSION >= 104600
  return boost::filesystem::path ( path ).extension().string();
#else
  return boost::filesystem::path ( path ).extension();
#endif
}

  
///////////////////////////////////////////////////////////////////////////////
//
//  Get the base file name from the path.
//
///////////////////////////////////////////////////////////////////////////////

inline std::string base ( const std::string &path )
{
#if BOOST_VERSION >= 104600
  return boost::filesystem::path ( path ).stem().string();
#else
  return boost::filesystem::path ( path ).stem();
#endif
}


} // namespace File
} // namespace Usul


#endif // _USUL_FILE_PATH_H_
