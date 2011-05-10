
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Perry L Miller IV
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Wraps up calls to the WMS servers.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_NETWORK_WMS_H_
#define _MINERVA_NETWORK_WMS_H_

#include "Minerva/Network/Export.h"
#include "Minerva/Network/Names.h"

#include "Usul/Convert/Convert.h"
#include "Usul/Strings/Format.h"

#include "boost/lexical_cast.hpp"

#include <map>
#include <sstream>
#include <string>

namespace Usul { namespace Interfaces { struct IUnknown; } }

namespace Minerva {
namespace Network {


///////////////////////////////////////////////////////////////////////////////
//
//  The WMS class.
//
///////////////////////////////////////////////////////////////////////////////

class MINERVA_NETWORK_EXPORT WMS
{
public:

  // Useful typedefs.
  typedef std::map<std::string,std::string> Options;


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Constructor.
  //
  /////////////////////////////////////////////////////////////////////////////

  WMS ( const std::string &url, const std::string &file ) :
    _url ( url ),
    _file ( file ),
    _options()
  {
    this->defaults();
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Constructor.
  //
  /////////////////////////////////////////////////////////////////////////////

  template < class Itr > WMS ( const std::string &url, const std::string &file, Itr first, Itr last ) :
    _url ( url ),
    _file ( file ),
    _options()
  {
    // Assign defaults first.
    this->defaults();

    // Now set given options.
    for ( Itr i = first; i != last; ++i )
    {
      const std::string name  ( i->first  );
      const std::string value ( i->second );
      _options[name] = value;
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Copy constructor.
  //
  /////////////////////////////////////////////////////////////////////////////

  WMS ( const WMS &wms ) :
    _url ( wms._url ),
    _file ( wms._file ),
    _options ( wms._options )
  {
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Assignment.
  //
  /////////////////////////////////////////////////////////////////////////////

  WMS& operator = ( const WMS &wms )
  {
    _url = wms._url;
    _file = wms._file;
    _options = wms._options;
    return *this;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Destructor.
  //
  /////////////////////////////////////////////////////////////////////////////

  ~WMS()
  {
  }

  // Download the file.
  void download ( unsigned int timeoutMilliSeconds, unsigned int maxNumAttempts = 10, Usul::Interfaces::IUnknown *caller = 0x0 );

  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get the option.
  //
  /////////////////////////////////////////////////////////////////////////////

  template < class T > T get ( const std::string &name, const T &defaultValue ) const
  {
    Options::const_iterator i ( _options.find ( name ) );
    return ( ( _options.end() == i ) ? defaultValue : boost::lexical_cast<T> ( i->second ) );
  }
  std::string get ( const std::string &name, const std::string &defaultValue ) const
  {
    Options::const_iterator i ( _options.find ( name ) );
    return ( ( _options.end() == i ) ? defaultValue : i->second );
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Set the option.
  //
  /////////////////////////////////////////////////////////////////////////////

  template < class T > void set ( const std::string &name, const T &value )
  {
    _options[name] = boost::lexical_cast<std::string,T> ( value );
  }
  template < class T > void set ( const std::string &name, const std::string &value )
  {
    _options[name] = value;
  }
  template < class Itr > void set ( Itr first, Itr last )
  {
    _options.insert ( first, last );
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get/set the option.
  //
  /////////////////////////////////////////////////////////////////////////////

  std::string &operator[] ( const std::string &name )
  {
    return _options[name];
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Clear all options.
  //
  /////////////////////////////////////////////////////////////////////////////

  void clear()
  {
    _options.clear();
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Set default options.
  //
  /////////////////////////////////////////////////////////////////////////////

  void defaults()
  {
    this->set ( Names::REQUEST, "GetMap"     );
    this->set ( Names::FORMAT,  "image/jpeg" );
    this->set ( Names::SRS,     "EPSG:4326"  );
    this->set ( Names::STYLES,  ""  );

    this->set<unsigned long> ( Names::WIDTH,  1024 );
    this->set<unsigned long> ( Names::HEIGHT, 1024 );
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get the base url.
  //
  /////////////////////////////////////////////////////////////////////////////

  std::string baseUrl() const
  {
    return _url;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get the full url.
  //
  /////////////////////////////////////////////////////////////////////////////

  static std::string fullUrl ( const std::string &baseUrl, const Options &options )
  {
    std::ostringstream url;
    url << baseUrl;
    for ( Options::const_iterator i = options.begin(); i != options.end(); ++i )
    {
      const std::string name  ( i->first  );
      const std::string value ( i->second );
      url << ( ( options.begin() == i ) ? '?' : '&' ) << name << '=' << value;
    }
    return url.str();
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get the full url.
  //
  /////////////////////////////////////////////////////////////////////////////

  std::string fullUrl() const
  {
    return Minerva::Network::WMS::fullUrl ( _url, _options );
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get the file.
  //
  /////////////////////////////////////////////////////////////////////////////

  std::string file() const
  {
    return _file;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get the file extension.
  //
  /////////////////////////////////////////////////////////////////////////////

  std::string extension() const
  {
    // Determine file extension by skipping "image/".
    const std::string format ( this->get ( Names::FORMAT, std::string ( "image/jpeg" ) ) );
    std::string ext ( ( format.size() > 6 && '/' == format.at(5) ) ? std::string ( format.begin() + 6, format.end() ) : format );
    ext = ( ( "jpeg" == ext ) ? "jpg" : ext );
    return ext;
  }

private:

  std::string _url;
  std::string _file;
  Options _options;
};


} // namespace WMS
} // namespace Usul


#endif // _MINERVA_NETWORK_WMS_H_
