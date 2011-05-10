
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Redirect stdout and stderr to files.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_IO_REDIRECT_OUTPUT_H_
#define _USUL_IO_REDIRECT_OUTPUT_H_

#include "Usul/Config/Config.h"
#include "Usul/Functions/SafeCall.h"

#include <fstream>
#include <iostream>
#include <string>


namespace Usul {
namespace IO {


struct Redirect
{
  Redirect ( const std::string &file ) : 
    _file ( file ),
    _out(),
    _origCout ( std::cout.rdbuf() ), 
    _origCerr ( std::cerr.rdbuf() )
  {
    if ( false == _file.empty() )
    {
      _out.open ( _file.c_str() );
      if ( true == _out.is_open() )
      {
        std::cout.rdbuf ( _out.rdbuf() );
        std::cerr.rdbuf ( _out.rdbuf() );
        std::cout.sync_with_stdio();
        std::cerr.sync_with_stdio();
      }
    }
  }

  ~Redirect()
  {
    if ( 0x0 != _origCout )
    {
      USUL_TRY_BLOCK
      {
        std::cout.sync_with_stdio();
        std::cout.rdbuf ( _origCout );
      }
      catch ( ... )
      {
      }
    }

    if ( 0x0 != _origCerr )
    {
      USUL_TRY_BLOCK
      {
        std::cerr.sync_with_stdio();
        std::cerr.rdbuf ( _origCerr );
      }
      catch ( ... )
      {
      }
    } 
  }

  std::string file() const
  {
    return _file;
  }

private:

  std::string _file;
  std::ofstream _out;
  std::streambuf *_origCout;
  std::streambuf *_origCerr;
};


} // namespace IO
} // namespace Usul


#endif // _USUL_IO_REDIRECT_OUTPUT_H_
