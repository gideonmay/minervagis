
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2005, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Usul/Components/Manager.h"
#include "Usul/DLL/Library.h"
#include "Usul/Interfaces/IPlugin.h"

#include <iostream>
#include <iterator>
#include <set>

using namespace Usul;
using namespace Usul::Components;


///////////////////////////////////////////////////////////////////////////////
//
//  Start of details.
//
///////////////////////////////////////////////////////////////////////////////

namespace Usul {
namespace Components {
namespace Helper {


///////////////////////////////////////////////////////////////////////////////
//
//  The library pool.
//
///////////////////////////////////////////////////////////////////////////////

typedef Usul::DLL::Library::ValidRefPtr LibPtr;
typedef std::set < LibPtr > LibraryPool;
LibraryPool _pool;


///////////////////////////////////////////////////////////////////////////////
//
//  Return true if this is a debug build.
//
///////////////////////////////////////////////////////////////////////////////

bool _isDebug()
{
  #ifdef _DEBUG
    return true;
  #else
    return false;
  #endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return a string describing the build mode.
//
///////////////////////////////////////////////////////////////////////////////

std::string _buildMode ( bool debug )
{
  return ( debug ) ? "Debug" : "Release";
}


///////////////////////////////////////////////////////////////////////////////
//
//  End of details.
//
///////////////////////////////////////////////////////////////////////////////

}
}
}


///////////////////////////////////////////////////////////////////////////////
//
//  Static data member
//
///////////////////////////////////////////////////////////////////////////////

Manager* Manager::_instance ( 0x0 );


///////////////////////////////////////////////////////////////////////////////
//
//  Get the instance
//
///////////////////////////////////////////////////////////////////////////////

Manager& Manager::instance()
{
  if ( !_instance )
    _instance = new Manager();
  return *_instance;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
///////////////////////////////////////////////////////////////////////////////

Manager::Manager() :
  _unknowns()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Load a plugin with the given filename.
//
///////////////////////////////////////////////////////////////////////////////

void Manager::load ( const std::string& filename )
{
  try
	{
    // Load the library. It throws if it fails to load.
    Usul::DLL::Library::ValidRefPtr lib ( new Usul::DLL::Library ( filename ) );

    // Cache in our pool.
    Helper::_pool.insert ( lib.get() );

    // Get the debug function. Note: g++ does not allow a reinterpret_cast.
    typedef bool (*DebugFunction)();
    DebugFunction df = (DebugFunction) lib->function ( "usul_is_debug_build" );

    // Make sure it matches.
    if ( 0x0 != df )
    {
      const bool isDebugMode ( df() );
#ifdef _DEBUG
      if ( false == isDebugMode )
#else
      if ( true == isDebugMode )
#endif
      {
        throw std::runtime_error ( Usul::Strings::format (
          "Error: 4210150186, mismatched build modes.",
          "\n\tBuild mode for loading module is ", Helper::_buildMode ( Helper::_isDebug() ),
          "\n\tBuild mode for '", filename, "' is ", Helper::_buildMode ( isDebugMode ) ) );
      }
    }

    // Look for the function used to initialize a plugin.
    typedef void (*Initialize)();
    Initialize initialize ( (Initialize) lib->function ( "usul_plugin_initialize" ) );

    // If we found it then call it.
    if ( 0x0 != initialize )
    {
      initialize();
    }
  }
	catch ( const std::exception& e )
	{
		std::cout << "Error 4241786283: Exception caught while trying to load " << filename << std::endl;
		std::cout << "Message: " << e.what() << std::endl;
	}
	catch ( ... )
	{
		std::cout << "Error 7167132960: Unknown exception caught while trying to load. " << std::endl;
	}
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add the plugin.
//
///////////////////////////////////////////////////////////////////////////////

void Manager::addPlugin ( Usul::Interfaces::IUnknown::RefPtr unknown )
{
  if ( true == unknown.valid() )
  {
    _unknowns.push_back ( unknown.get() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear plugins and release libraries
//
///////////////////////////////////////////////////////////////////////////////

void Manager::clear ( std::ostream *out ) 
{
  if ( 0x0 == out )
  {
    _unknowns.clear(); 
    Helper::_pool.clear();
    return;
  }

  while ( false == _unknowns.empty() )
  {
    UnknownSet::iterator i ( _unknowns.begin() );
    Usul::Interfaces::IUnknown::RefPtr unknown ( i->get() );
    Usul::Interfaces::IPlugin::QueryPtr plugin ( unknown.get() );
    if ( plugin.valid() )
      (*out) << "Releasing component: " << plugin->getPluginName() << std::endl;
    _unknowns.erase ( i );
  }

  while ( false == Helper::_pool.empty() )
  {
    Helper::LibraryPool::iterator i ( Helper::_pool.begin() );
    (*out) << "Releasing file: " << (*i)->filename() << std::endl;
    Helper::_pool.erase ( i );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find first IUnknown with given iid.,
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown* Manager::getInterface( unsigned long iid )
{
  for ( UnknownItr i = _unknowns.begin(); i != _unknowns.end(); ++i )
  {
    IUnknown *u ( (*i).get() );
    if( u->queryInterface( iid ) )
      return (*i).get();
  }

  return 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find all IUnknowns with given iid
//
///////////////////////////////////////////////////////////////////////////////

Manager::UnknownSet Manager::getInterfaces ( unsigned long iid )
{
  UnknownSet set;
  for ( UnknownItr i = _unknowns.begin(); i != _unknowns.end(); ++i )
  {
    IUnknown *u ( (*i).get() );
    if( u->queryInterface( iid ) )
      set.push_back ( *i );
  }

  return set;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return list of plugin names.
//
///////////////////////////////////////////////////////////////////////////////

Manager::Strings Manager::names ( bool sort ) const
{
  Manager::Strings names;
  UnknownSet unknowns ( _unknowns );
  for ( UnknownSet::iterator i = unknowns.begin(); i != unknowns.end(); ++i )
  {
    Usul::Interfaces::IPlugin::QueryPtr plugin ( i->get() );
    if ( plugin.valid() )
      names.push_back ( plugin->getPluginName() );
  }

  if ( true == sort )
    names.sort();

  return names;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Print message about loaded plugins.
//
///////////////////////////////////////////////////////////////////////////////

void Manager::print ( std::ostream &out ) const
{
  const Manager::Strings names ( this->names() );
  out << names.size() << ( ( 1 == names.size() ) ? ( " plugin" ) : ( " plugins" ) );
  if ( false == names.empty() )
  {
    out << ": ";
    std::copy ( names.begin(), names.end(), std::ostream_iterator<std::string> ( out, "; " ) );
  }
  out << std::endl;
}
