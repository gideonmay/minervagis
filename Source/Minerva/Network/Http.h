
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach and 
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Wraps up calls to the Curl library.
//  See http://curl.haxx.se/libcurl/c/libcurl-tutorial.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_NETWORK_CURL_H_
#define _MINERVA_NETWORK_CURL_H_

#include "Minerva/Network/Export.h"

#include "Usul/Interfaces/IUnknown.h"

#include "boost/noncopyable.hpp"

#include <string>
#include <vector>

typedef void CURL;

namespace Minerva {
namespace Network {


class MINERVA_NETWORK_EXPORT Http : public boost::noncopyable
{
public:

  // Typedefs.
  typedef Usul::Interfaces::IUnknown Unknown;

  // Constructor.
  Http ( const std::string &url, std::ostream *out, Unknown *caller = 0x0 );

  // Destructor.
  ~Http();


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Nested class to initialize and clean up the curl library.
  //
  /////////////////////////////////////////////////////////////////////////////

  struct Life
  {
    Life();
    ~Life();
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Wrapper around curl handle.
  //
  /////////////////////////////////////////////////////////////////////////////

  struct Handle
  {
    Handle();

    ~Handle();
    void cleanup();
    ::CURL *handle();
    
  private:
    ::CURL *_curl;
  };
  
  /// Encode a url.
  static std::string encode ( const std::string& url );

  /// Download.
  void download ( unsigned int timeoutMilliSeconds );
  
  /// Post.
  void post ( unsigned int timeoutMilliSeconds, const std::string& post );
  
  /// Download the file.
  static void download ( const std::string &url, const std::string &file, unsigned int timeoutMilliSeconds, Unknown *caller = 0x0 );
  
private:

  /// Get the canceled state.
  bool _isCanceled();

  /// Check the canceled state and cancel (throw) if we should.
  void _checkCanceledState();

  /// Called during a download.
  static size_t _writeDataCB ( void *buffer, size_t sizeOfOne, size_t numElements, void *userData );

  /// Called during a download.
  size_t _writeData ( void *buffer, size_t sizeOfOne, size_t numElements );

  /// Called during a download to indicate progress.
  static int _progressCB ( void *userData, double thisDownload, double totalDownloaded, double thisUpload, double totalUploaded );

  /// Called during a download to indicate progress.
  int _progress ( double thisDownload, double totalDownloaded, double thisUpload, double totalUploaded );

  /// Check return code.
  void _check ( int code );
  
  //  Data members.
  std::string _url;
  std::ostream *_stream;
  std::vector<char> _error;
  Unknown::QueryPtr _caller;
  Handle _handle;
};


} // namespace Curl
} // namespace Usul


#endif // _MINERVA_NETWORK_CURL_H_
