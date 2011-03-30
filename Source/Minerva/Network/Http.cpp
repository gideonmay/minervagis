
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
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

#include "Minerva/Network/Http.h"

#include "Usul/Exceptions/Canceled.h"
#include "Usul/Exceptions/TimedOut.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Interfaces/ICancel.h"
#include "Usul/Interfaces/ICanceledStateGet.h"
#include "Usul/Math/MinMax.h"
#include "Usul/Scope/Reset.h"
#include "Usul/Scope/RemoveFile.h"
#include "Usul/Strings/Format.h"

#include "curl/curl.h"

#include <fstream>
#include <limits>
#include <cstring>

using namespace Minerva::Network;

const unsigned int CURL_ERROR_BUFFER_SIZE ( 2 * CURL_ERROR_SIZE );

/////////////////////////////////////////////////////////////////////////////
//
//  Nested class to initialize and clean up the curl library.
//
/////////////////////////////////////////////////////////////////////////////

Http::Life::Life()
{
  ::curl_global_init ( CURL_GLOBAL_ALL );
}
Http::Life::~Life()
{
  ::curl_global_cleanup();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Wrapper around curl handle.
//
/////////////////////////////////////////////////////////////////////////////

Http::Handle::Handle() : _curl ( ::curl_easy_init() )
{
  if ( 0x0 == _curl )
  {
    throw std::runtime_error ( "Error 2234813705: Failed to open curl easy handle" );
  }
}
Http::Handle::~Handle()
{
  this->cleanup();
}
void Http::Handle::cleanup()
{
  ::CURL *curlHandle ( _curl );
  _curl = 0x0;
  
  if ( 0x0 != curlHandle )
  {
    ::curl_easy_cleanup ( curlHandle );
  }
}
::CURL * Http::Handle::handle()
{
  return _curl;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
/////////////////////////////////////////////////////////////////////////////

Http::Http ( const std::string &url, std::ostream *out, Unknown *caller ) :
  _url  ( url ),
  _stream ( out ),
  _error ( CURL_ERROR_BUFFER_SIZE, '\0' ),
  _caller ( caller ),
  _handle()
{
  // Set properties.
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_ERRORBUFFER, &_error[0] ) );
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_URL, _url.c_str() ) );
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_WRITEDATA, this ) );
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_WRITEFUNCTION, &Http::_writeDataCB ) );
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_PROGRESSDATA, this ) );
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_PROGRESSFUNCTION, &Http::_progressCB ) );
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_NOPROGRESS, false ) );
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_NOSIGNAL, true ) );
  this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_FOLLOWLOCATION, true ) );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
/////////////////////////////////////////////////////////////////////////////

Http::~Http()
{
}


/////////////////////////////////////////////////////////////////////////////
//
//  Encode a url.
//
/////////////////////////////////////////////////////////////////////////////

std::string Http::encode ( const std::string& url )
{
  Http::Handle h;
  char *encodedUrl ( ::curl_easy_escape ( h.handle(), url.c_str(), static_cast<int> ( url.size() ) ) );
  
  std::string result;
  
  try
  {
    result.assign ( encodedUrl, encodedUrl + ::strlen ( encodedUrl ) );
    ::curl_free ( encodedUrl );
  }
  USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS ( "1112159745" );
  
  return result;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Download.
//
/////////////////////////////////////////////////////////////////////////////

void Http::download ( unsigned int timeoutMilliSeconds )
{
  // Add timeout if it's valid.
  if ( timeoutMilliSeconds > 0 )
    this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_TIMEOUT_MS, static_cast<long> ( timeoutMilliSeconds ) ) );
  
  // Get the data.
  this->_check ( ::curl_easy_perform ( _handle.handle() ) );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Post.
//
/////////////////////////////////////////////////////////////////////////////

void Http::post ( unsigned int timeoutMilliSeconds, const std::string& post )
{
  // Add timeout if it's valid.
  if ( timeoutMilliSeconds > 0 )
    this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_TIMEOUT_MS, static_cast<long> ( timeoutMilliSeconds ) ) );
  
  // Add post data if it's there.
  if ( false == post.empty() )
    this->_check ( ::curl_easy_setopt ( _handle.handle(), CURLOPT_POSTFIELDS, post.c_str() ) );
  
  // Get the data.
  this->_check ( ::curl_easy_perform ( _handle.handle() ) );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Download the file.
//
/////////////////////////////////////////////////////////////////////////////

void Http::download ( const std::string &url, const std::string &file, unsigned int timeoutMilliSeconds, Unknown *caller )
{
  // Open file.
  std::ofstream stream ( file.c_str(), std::ofstream::binary | std::ofstream::out );
  if ( false == stream.is_open() )
  {
    throw std::runtime_error ( "Error 2742979881: Failed to open file '" + file + "' for writing" );
  }
 
  // This will remove the file is there's an exception.
  Usul::Scope::RemoveFile removeFile ( file );
  
  Http http ( url, &stream, caller );
  http.download( timeoutMilliSeconds );
  
  // Keep the file.
  removeFile.remove ( false );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Get the canceled state.
//
/////////////////////////////////////////////////////////////////////////////

bool Http::_isCanceled()
{
  Usul::Interfaces::ICanceledStateGet::QueryPtr canceledState ( _caller );
  return ( ( true == canceledState.valid() ) ? canceledState->canceled() : false );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Check the canceled state and cancel (throw) if we should.
//
/////////////////////////////////////////////////////////////////////////////

void Http::_checkCanceledState()
{
  if ( true == this->_isCanceled() )
  {
    Usul::Interfaces::ICancel::QueryPtr cancelJob ( _caller );
    if ( true == cancelJob.valid() )
    {
      cancelJob->cancel();
    }
  }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Called during a download.
//
/////////////////////////////////////////////////////////////////////////////

size_t Http::_writeDataCB ( void *buffer, size_t sizeOfOne, size_t numElements, void *userData )
{
  Http *me ( reinterpret_cast<Http *> ( userData ) );
  return ( ( 0x0 == me ) ? 0 : me->_writeData ( buffer, sizeOfOne, numElements ) );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Called during a download.
//
/////////////////////////////////////////////////////////////////////////////

size_t Http::_writeData ( void *buffer, size_t sizeOfOne, size_t numElements )
{
  std::ostream *file ( _stream );
  if ( 0x0 == file )
  {
    return 0;
  }
  
  // Initialize.
  const size_t totalBytes ( sizeOfOne * numElements );
  const char *bytes ( reinterpret_cast<const char *> ( buffer ) );
  
  // Since size_t is unsigned but std::streamsize is signed, 
  // we have to write in pieces.
  while ( static_cast<size_t> ( ( bytes - ( reinterpret_cast<const char *> ( buffer ) ) ) ) < totalBytes )
  {
    // Determine the max we can write.
    const std::streamsize writeSize ( static_cast<std::streamsize> ( Usul::Math::minimum ( static_cast<size_t> ( std::numeric_limits<std::streamsize>::max() ), totalBytes ) ) );
    
    // Write the bytes.
    file->write ( bytes, writeSize );
    
    // Offset the buffer.
    bytes += writeSize;
    
    // Check to see if we've been canceled.
    if ( true == this->_isCanceled() )
    {
      // Return zero size to stop downloading. Results in CURLE_WRITE_ERROR.
      return 0;
    }
  }
  
  // If we get to here then we should have written the entire buffer.
  return totalBytes;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Called during a download to indicate progress.
//
/////////////////////////////////////////////////////////////////////////////

int Http::_progressCB ( void *userData, double thisDownload, double totalDownloaded, double thisUpload, double totalUploaded )
{
  Http *me ( reinterpret_cast<Http *> ( userData ) );
  return ( ( 0x0 == me ) ? 0 : me->_progress ( thisDownload, totalDownloaded, thisUpload, totalUploaded ) );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Called during a download to indicate progress.
//
/////////////////////////////////////////////////////////////////////////////

int Http::_progress ( double thisDownload, double totalDownloaded, double thisUpload, double totalUploaded )
{
  // Check to see if we've been canceled.
  if ( true == this->_isCanceled() )
  {
    return CURLE_ABORTED_BY_CALLBACK;
  }
  
  // Keeps things going. Non-zero return results in CURLE_ABORTED_BY_CALLBACK.
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Check return code.
//
/////////////////////////////////////////////////////////////////////////////

void Http::_check ( int code )
{
  // Check canceled state and throw if we should.
  this->_checkCanceledState();
  
  // No error.
  if ( 0 == code )
    return;
  
  // Convert curl error to a message.
  const std::string error ( &_error[0] );
  const std::string message ( Usul::Strings::format ( 
    ( ( error.empty() ) ? Usul::Strings::format ( "curl error code = ", code ) : error ), 
    ", URL = ", _url ) );
  
  // Did it time out?
  if ( CURLE_OPERATION_TIMEDOUT == code )
    throw Usul::Exceptions::TimedOut::NetworkDownload ( "Error 1884185898: " + message );
  
  // It didn't time out so throw a standard exception.
  throw std::runtime_error ( "Error 3886626924: " + message );
}
