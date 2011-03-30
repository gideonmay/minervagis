
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2005, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class for managing IUnknowns
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __USUL_COMPONENT_MANAGER_H__
#define __USUL_COMPONENT_MANAGER_H__

#include "Usul/Export/Export.h"

#include "Usul/Interfaces/IUnknown.h"
#include "Usul/Interfaces/IClassFactory.h"

#include <string>
#include <list>
#include <map>
#include <iosfwd>
#include <vector>

namespace Usul {
namespace Components {


class USUL_EXPORT Manager
{
public:
  typedef Usul::Interfaces::IUnknown  IUnknown;
  typedef IUnknown::ValidRefPtr       UnknownPtr;
  typedef std::vector < UnknownPtr >  UnknownSet;
  typedef std::list < std::string >   Strings;

  static Manager& instance();

  // Add a plugin.
  void                          addPlugin ( IUnknown::RefPtr );

  // Clear the list of plugins.
  void                          clear ( std::ostream *out );

  // Do we have any unknowns?
  bool                          empty () const { return _unknowns.empty(); }

  // Load the plugin.
  void                          load ( unsigned long iid, const std::string& file );

  // Return list of plugin names. This queries each unknown pointer for IPlugin.
  Strings                       names ( bool sort = true ) const;

  // Print message about loaded plugins.
  void                          print ( std::ostream & ) const;

  // Return the unknowns
  UnknownSet&                   unknowns() { return _unknowns; }

  // Get a single IUnknown
  Usul::Interfaces::IUnknown*   getInterface( unsigned long iid );

  // Get a set of IUnknowns
  UnknownSet                    getInterfaces( unsigned long iid );

private:

  typedef UnknownSet::iterator UnknownItr;
  typedef Usul::Interfaces::IClassFactory Factory;

  Manager();

  Factory*  _factory ( const std::string &filename );

  UnknownSet            _unknowns;
  static Manager *      _instance;
};


} // namespace Components
} // namespace Usul


#endif // __USUL_COMPONENT_MANAGER_H__
