
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
//  Network layer class.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Layers/RasterLayerNetwork.h"
#include "Minerva/Core/DiskCache.h"
#include "Minerva/Network/Names.h"

#include "Usul/File/Path.h"
#include "Usul/Exceptions/TimedOut.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Jobs/Job.h"
#include "Usul/Math/Absolute.h"
#include "Usul/Registry/Database.h"
#include "Usul/Strings/Format.h"
#include "Usul/Strings/Mangle.h"
#include "Usul/Scope/RemoveFile.h"
#include "Usul/Threads/Safe.h"

#include "osg/ref_ptr"
#include "osg/Image"

#include "boost/algorithm/string/trim.hpp"
#include "boost/bind.hpp"
#include "boost/filesystem.hpp"

#include <fstream>

using namespace Minerva::Core::Layers;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerNetwork::RasterLayerNetwork ( const Extents &maxExtents, const std::string &url, const Options &options ) : 
  BaseClass(),
  _url              ( url ),
  _options          ( options ),
  _useNetwork       ( true ),
  _writeFailedFlags ( false ),
  _readFailedFlags  ( false ),
  _maxNumAttempts   ( Usul::Registry::Database::instance()["network_download"]["raster_layer"]["max_num_attempts"].get<unsigned int> ( 5, true ) ),
  _timeout          ( Usul::Registry::Database::instance()["network_download"]["raster_layer"]["timeout_milliseconds"].get<unsigned int> ( 5000, true ) )
{
  this->extents ( maxExtents );

  this->_registerMembers();
  
  // Find a reader.
  this->_findImageReader();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerNetwork::RasterLayerNetwork ( const RasterLayerNetwork& rhs ) :
  BaseClass ( rhs ),
  _url ( rhs._url ),
  _options ( rhs._options ),
  _useNetwork ( rhs._useNetwork ),
  _writeFailedFlags ( rhs._writeFailedFlags ),
  _readFailedFlags ( rhs._readFailedFlags ),
  _maxNumAttempts ( rhs._maxNumAttempts ),
  _timeout ( rhs._timeout )
{
  this->_registerMembers();
  
  // Find a reader.
  this->_findImageReader();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerNetwork::~RasterLayerNetwork()
{
  Usul::Functions::safeCall ( boost::bind ( &RasterLayerNetwork::_destroy, this ), "8458296530" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Register members for serialization.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::_registerMembers()
{
  // Serialization glue.
  this->_addMember ( "url", _url );
  this->_addMember ( "options", _options );
  this->_addMember ( "use_network", _useNetwork );
  this->_addMember ( "write_failed_flags", _writeFailedFlags );
  this->_addMember ( "read_failed_flags", _readFailedFlags );
  this->_addMember ( "max_num_attempts", _maxNumAttempts );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destroy.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::_destroy()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  The name of the "failed" file.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  inline std::string getFailedFileName ( const std::string &file )
  {
    return Usul::Strings::format ( Usul::File::directory ( file ), "/", Usul::File::base ( file ), ".failed" );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the texture.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerNetwork::ImagePtr RasterLayerNetwork::_textureImplementation ( 
                                                                         const std::string& file, 
                                                                         const TileKey& key, 
                                                                         unsigned int width, 
                                                                         unsigned int height, 
                                                                         Usul::Jobs::Job *job, 
                                                                         IUnknown *caller )
{
  // Get url.
  std::string url ( Usul::Threads::Safe::get ( this->mutex(), _url ) );
  if ( true == url.empty() )
    return ImagePtr ( 0x0 );
  
  const Extents extents ( key.extents() );
  const unsigned int level ( key.level() );

  // Change the name of the job for better feedback.
  this->_setJobName ( job, extents, url, level );

  // Are we supposed to look for a "failed" file?
  if ( true == Usul::Threads::Safe::get ( this->mutex(), _readFailedFlags ) )
  {
    const std::string failedFile ( Helper::getFailedFileName ( file ) );
    if ( true == boost::filesystem::exists ( failedFile ) )
    {
      // The "failed" file exists, so return null.
      return ImagePtr ( 0x0 );
    }
  }

  // See if the job has been cancelled.
  _checkForCanceledJob ( job );

  // Needed below a couple of times.
  const std::string fullUrl ( this->urlFull ( key, width, height ) );
  
  // Pull it down if we should...
	if ( ( false == boost::filesystem::exists ( file ) ) && ( true == this->useNetwork() ) )
  {
    try
    {
      this->_logEvent ( Usul::Strings::format ( "Message 3507413903: Download started: ", fullUrl ) );
      this->_download ( file, key, width, height, job, caller );
      this->_logEvent ( Usul::Strings::format ( "Message 1315552899: Download finished: ", fullUrl ) );
    }
    catch ( const Usul::Exceptions::Canceled & )
    {
      this->_logEvent ( Usul::Strings::format ( "Message 3919893899: Canceling download: ", fullUrl ) );
      throw;
    }
    catch ( const Usul::Exceptions::TimedOut::NetworkDownload &e )
    {
      // Increase the timeout amount.
      this->timeoutMilliSeconds ( this->timeoutMilliSeconds() * Usul::Registry::Database::instance()["network_download"]["raster_layer"]["timed_out_factor"].get<unsigned int> ( 2, true ) );

      // Log and re-throw.
      this->_logEvent ( Usul::Strings::format ( "Error 1710361995: ", ( ( 0x0 != e.what() ) ? e.what() : "unknown" ) ) );
      throw;
    }
    catch ( const std::exception &e )
    {
      this->_logEvent ( Usul::Strings::format ( "Error 1156606570: Standard exeption caught while downloading. Reason: ", ( ( 0x0 != e.what() ) ? e.what() : "unknown" ), ", URL: ", fullUrl ) );
      this->_downloadFailed ( file, fullUrl );
      return ImagePtr ( 0x0 );
    }
    catch ( ... )
    {
      this->_logEvent ( Usul::Strings::format ( "Error 3173082684: Unknown exeption caught while downloading. URL: ", fullUrl ) );
      this->_downloadFailed ( file, fullUrl );
      return ImagePtr ( 0x0 );
    }
  }

  // See if the job has been cancelled.
  _checkForCanceledJob ( job );

  // If the file does not exist then return.
  if ( false == boost::filesystem::exists ( file ) )
  {
    this->_logEvent ( Usul::Strings::format ( "Error 1276423772: Failed to download. File: ", file, ", URL: ", fullUrl ) );
    this->_downloadFailed ( file, fullUrl );
    return ImagePtr ( 0x0 );
  }

  // If the file is empty then remove it and return.
  if ( 0 == boost::filesystem::file_size ( file ) )
  {
    this->_logEvent ( Usul::Strings::format ( "Error 3244363936: Download file is empty. File: ", file, ", URL: ", fullUrl, ". Removing file." ) );
    boost::filesystem::remove ( file );
    this->_downloadFailed ( file, fullUrl );
    return ImagePtr ( 0x0 );
  }

  // Load the file.
  ImagePtr image ( this->_readImageFile ( file ) );

  // If it failed to load...
  if ( false == image.valid() )
  {
    this->_logEvent ( Usul::Strings::format ( "Error 2720181403: Failed to load file: ", file, ", downloaded from URL: ", fullUrl, ". Removing file." ) );
    boost::filesystem::remove ( file );
    this->_downloadFailed ( file, fullUrl );
    return ImagePtr ( 0x0 );
  }

  // Set the file name and return.
  image->setFileName ( file );
  return image;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the directory.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Common::LayerKey::RefPtr RasterLayerNetwork::cacheKey() const
{
  const std::string name ( Usul::Strings::Mangle::url ( Usul::Threads::Safe::get ( this->mutex(), _url ) ) );
  const std::size_t id ( BaseClass::_hashString ( this->_getAllOptions() ) );

  return new LayerKey ( name, id );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return a string containing all the options.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayerNetwork::_getAllOptions() const
{
  Guard guard ( this );

  std::ostringstream out;
  for ( Options::const_iterator i = _options.begin(); i != _options.end(); ++i )
  {
    const std::string key ( i->first );
    const std::string value ( i->second );
    out << key << '=' << value << '&';
  }

  const std::string result ( out.str() );
  return ( ( result.empty() ) ? "default" : result );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Called when a download fails.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::_downloadFailed ( const std::string &file, const std::string &url )
{
  bool writeFailed ( Usul::Threads::Safe::get ( this->mutex(), _writeFailedFlags ) );
  if ( true == writeFailed )
  {
    const std::string failedFile ( Helper::getFailedFileName ( file ) );
    std::ofstream out ( failedFile.c_str() );
    out << "Failed to download\nFile: " << file << "\nURL: " << url << std::endl;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the options map.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::options ( const Options& options )
{
  Guard guard ( this->mutex() );
  _options = options;

  // Find a reader.
  this->_findImageReader();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the options map.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerNetwork::Options RasterLayerNetwork::options() const
{
  Guard guard ( this->mutex() );
  return _options;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the base url.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::urlBase ( const std::string& url )
{
  Guard guard ( this->mutex() );
  _url = url;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the base url.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayerNetwork::urlBase() const
{
  Guard guard ( this->mutex() );
  return _url;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the full url.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayerNetwork::urlFull ( const TileKey& key, unsigned int width, unsigned int height ) const
{
  const std::string url ( this->urlBase() + "?" + this->_getAllOptions() );
  return url;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find a reader for our format.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::_findImageReader()
{
  Guard guard ( this->mutex() );
  
  // Clear the one we have.
  this->_imageReaderSet ( ReaderPtr ( 0x0 ) );

  // Set the reader.
  this->_imageReaderFind ( this->_cacheFileExtension() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Deserialize.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::deserialize ( const XmlTree::Node& node )
{
  BaseClass::deserialize ( node );
  
  // Find a reader.
  this->_findImageReader();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extension for the cached files.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayerNetwork::_cacheFileExtension() const
{
  Guard guard ( this->mutex() );

  // Get the format.
  Options::const_iterator i ( _options.find ( Minerva::Network::Names::FORMAT ) );
  const std::string format ( ( _options.end() == i ) ? "" : i->second );

  // Determine file extension.
  std::string ext ( ( format.size() > 6 && '/' == format.at(5) ) ? std::string ( format.begin() + 6, format.end() ) : format );
  ext = ( ( "jpeg" == ext ) ? "jpg" : ext );
  return ext;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the max number of attempts.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::maxNumAttempts ( unsigned int attempts )
{
  Guard guard ( this->mutex() );
  _maxNumAttempts = attempts;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the max number of attempts.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int RasterLayerNetwork::maxNumAttempts() const
{
  Guard guard ( this->mutex() );
  return _maxNumAttempts;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the timeout in milliseconds.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::timeoutMilliSeconds ( unsigned int timeout )
{
  Guard guard ( this->mutex() );
  _timeout = timeout;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the timeout in milliseconds.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int RasterLayerNetwork::timeoutMilliSeconds() const
{
  Guard guard ( this->mutex() );
  return _timeout;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Should this layer use the network?
//
///////////////////////////////////////////////////////////////////////////////

bool RasterLayerNetwork::useNetwork() const
{
	const bool workOffline ( Usul::Registry::Database::instance()["work_offline"].get<bool> ( false, true ) );
	if ( true == workOffline )
		return false;

	return Usul::Threads::Safe::get ( this->mutex(), _useNetwork );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The cache file extension.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayerNetwork::cacheFileExtension() const
{
  return this->_cacheFileExtension();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the job name.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerNetwork::_setJobName ( Usul::Jobs::Job* job, const Extents& extents, const std::string& url, unsigned int level )
{
  if ( 0x0 != job )
  {
    Options options ( this->options() );
    std::string layer ( Usul::Strings::format ( options["layers"], ' ', options["LAYERS"], ' ', options["Layers"] ) );
    boost::trim_left ( layer );
    boost::trim_right ( layer );
    typedef Usul::Convert::Type<float,std::string> Converter;
    job->name ( Usul::Strings::format ( "Extents: [", 
      Converter::convert ( extents.minimum()[0] ), ", ", 
      Converter::convert ( extents.minimum()[1] ), ", ", 
      Converter::convert ( extents.maximum()[0] ), ", ", 
      Converter::convert ( extents.maximum()[1] ), "]",
      ", Level: ", level,
      ", Server: ", url,
      ( ( layer.empty() ) ? "" : ( ", Layer: " + layer ) ) ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the degrees per pixel.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  inline Usul::Math::Vec2d degreesPerPixel ( const Minerva::Common::Extents& extents, unsigned int width, unsigned int height )
  {
    const Minerva::Common::Extents::Vertex &mn ( extents.minimum() );
    const Minerva::Common::Extents::Vertex &mx ( extents.maximum() );

    return Usul::Math::Vec2d ( ( mx[0] - mn[0] ) / width, ( mx[1] - mn[1] ) / height );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Expand the extents by the given amount.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  inline Minerva::Common::Extents expandExtents ( const Minerva::Common::Extents& extents, const Usul::Math::Vec2d& amount )
  {
    const Minerva::Common::Extents::Vertex &mn ( extents.minimum() );
    const Minerva::Common::Extents::Vertex &mx ( extents.maximum() );

    return Minerva::Common::Extents ( mn[0] - ( amount[0] / 2.0 ), mn[1] - ( amount[1] / 2.0 ), mx[0] + ( amount[0] / 2.0 ), mx[1] + ( amount[1] / 2.0 ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the raster data as elevation data.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::IElevationData::RefPtr RasterLayerNetwork::elevationData ( 
  const TileKey& key,
  unsigned int width,
  unsigned int height,
  Usul::Jobs::Job* job,
  Usul::Interfaces::IUnknown* caller )
{
  // Expand the request by degreesPerPixel / 2.0 on each side.
  Extents extents ( key.extents() );
  const Extents request ( Helper::expandExtents ( extents, Helper::degreesPerPixel ( extents, width, height ) ) );
  
  // Make a new key with the modified extents.
  TileKey::RefPtr newKey ( new TileKey );
  newKey->row ( key.row() );
  newKey->column ( key.column() );
  newKey->level ( key.level() );
  newKey->extents ( request );
  newKey->imageSize ( key.imageSize() );
  newKey->meshSize ( key.meshSize() );

  return RasterLayer::elevationData ( *newKey, width, height, job, caller );
}
