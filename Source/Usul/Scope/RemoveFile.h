
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Removes the file in the destructor.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_SCOPED_FILE_REMOVE_H_
#define _USUL_SCOPED_FILE_REMOVE_H_

#include "Usul/Config/Config.h"

#include "boost/filesystem.hpp"


namespace Usul {
namespace Scope {


struct RemoveFile
{
  RemoveFile ( const std::string &file, bool allowThrow = false ) : 
    _file ( file ), 
    _remove ( true ),
    _allowThrow ( allowThrow )
  {
  }

  ~RemoveFile()
  {
    if ( true == _remove )
    {
      try
      {
        boost::filesystem::remove ( _file );
      }
      catch ( ... )
      {
        if ( true == _allowThrow )
        {
          throw;
        }
      }
    }
  }

  void remove ( bool state )
  {
    _remove = state;
  }

private:

  std::string _file;
  bool _remove;
  bool _allowThrow;
};


} // namespace Scope
} // namespace Usul


#endif // _USUL_SCOPED_FILE_REMOVE_H_
