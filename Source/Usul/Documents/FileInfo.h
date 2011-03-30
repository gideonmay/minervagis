
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Small struct that contains file info.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _OPEN_SCENE_GRAPH_FOX_BINDING_FILE_INFO_H_
#define _OPEN_SCENE_GRAPH_FOX_BINDING_FILE_INFO_H_

#include "Usul/Export/Export.h"

#include <string>


namespace Usul {
namespace Documents {


struct USUL_EXPORT FileInfo
{
  FileInfo() : 
    _name     (),
    _valid    ( false ),
    _modified ( false )
  {
  }

  FileInfo ( const std::string &n, bool v, bool m ) : 
    _name     ( n ), 
    _valid    ( v ),
    _modified ( m )
  {
  }

  FileInfo ( const FileInfo &f ) : 
    _name     ( f._name     ),
    _valid    ( f._valid    ),
    _modified ( f._modified )
  {
  }

  FileInfo &operator = ( const FileInfo &f )
  {
    _name     = f._name;
    _valid    = f._valid;
    _modified = f._modified;
    return *this;
  }

  const std::string &name() const
  {
    return _name;
  }

  void name ( const std::string &n )
  {
    _name = n;
  }

  bool valid() const
  {
    return _valid;
  }

  void valid ( bool v )
  {
    _valid = v;
  }

  bool modified() const
  {
    return _modified;
  }

  void modified ( bool m )
  {
    _modified = m;
  }

private:

  std::string _name;
  bool _valid;
  bool _modified;
};


} // namespace Documents
} // namespace Usul


#endif // _OPEN_SCENE_GRAPH_FOX_BINDING_FILE_INFO_H_
