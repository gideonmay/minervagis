
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class that manages reading and writing of cache data to disk.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/DiskCache.h"

#include "Usul/File/Temp.h"
#include "Usul/Math/Absolute.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Strings/Format.h"
#include "Usul/Threads/Guard.h"
#include "Usul/Threads/Mutex.h"

#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/bind.hpp"
#include "boost/filesystem.hpp"

#include "osgDB/ReadFile"
#include "osgDB/WriteFile"

#include <iomanip>
#include <sstream>

using namespace Minerva::Core;

typedef Usul::Threads::Guard<Usul::Threads::Mutex> Guard;

///////////////////////////////////////////////////////////////////////////////
//
//  Initialize static data members.
//
///////////////////////////////////////////////////////////////////////////////

DiskCache* DiskCache::_instance ( 0x0 );


///////////////////////////////////////////////////////////////////////////////
//
//  Get the instance.
//
///////////////////////////////////////////////////////////////////////////////

DiskCache& DiskCache::instance()
{
  if ( 0x0 == _instance )
    _instance = new DiskCache;

  return *_instance;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

DiskCache::DiskCache() : 
  _readerMutex ( new Usul::Threads::Mutex ),
  _writerMutex ( new Usul::Threads::Mutex ),
  _cacheDirMutex ( new Usul::Threads::Mutex ),
  _baseCacheDirectory ( Usul::File::Temp::directory() + "/Minerva" )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

DiskCache::~DiskCache()
{
  delete _readerMutex;
  delete _writerMutex;
  delete _cacheDirMutex;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the cache directory.
//
///////////////////////////////////////////////////////////////////////////////

void DiskCache::cacheDirectory ( const std::string& directory )
{
  Usul::Threads::Guard<Usul::Threads::Mutex> guard ( *_cacheDirMutex );
  _baseCacheDirectory = directory;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the cache directory.
//
///////////////////////////////////////////////////////////////////////////////

std::string DiskCache::cacheDirectory() const
{
  Usul::Threads::Guard<Usul::Threads::Mutex> guard ( *_cacheDirMutex );
  return _baseCacheDirectory;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read an image file.
//
///////////////////////////////////////////////////////////////////////////////

DiskCache::ImagePtr DiskCache::readImage ( const std::string& filename, ReaderPtr reader ) const
{
  // Try to use the given reader.
  if ( reader.valid() )
    return reader->readImageFile ( filename );

  Usul::Threads::Guard<Usul::Threads::Mutex> guard ( *_readerMutex );

  // Temporarily turn off verbose output.
  osg::NotifySeverity level ( osg::getNotifyLevel() );
  osg::setNotifyLevel ( osg::ALWAYS ); // Yes, this turns it off.
  Usul::Scope::Caller::RefPtr reset ( Usul::Scope::makeCaller ( boost::bind ( osg::setNotifyLevel, boost::cref ( level ) ) ) );

  // Fall back on OSG if we don't have 
  return osgDB::readImageFile ( filename );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Write the image to cache.
//
///////////////////////////////////////////////////////////////////////////////

void DiskCache::writeImage ( const std::string& filename, ImagePtr image )
{
  // Check the image.
  if ( false == image.valid() )
    return;

  Usul::Threads::Guard<Usul::Threads::Mutex> guard ( *_writerMutex );
  
  // Write the file.
  osgDB::writeImageFile ( *image, filename );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the string for the value.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  std::string makeString ( unsigned int value )
  {
    std::ostringstream out;
    out << std::setw ( 3 ) << std::setfill ( '0' ) << value;
    return out.str();
  }
  std::string makeString ( double value )
  {
    std::ostringstream out;

    const double positive ( Usul::Math::absolute ( value ) );

    const unsigned long integer ( static_cast < unsigned long > ( positive ) );
    const double decimal ( positive - static_cast < double > ( integer ) );

    const unsigned int bufSize ( 2047 );
    char buffer[bufSize + 1];
    ::sprintf ( buffer, "%0.15f", decimal );

    std::string temp ( buffer );
    boost::replace_first ( temp, "0.", " " );
    boost::trim_left ( temp );

    out << ( ( value >= 0 ) ? 'P' : 'N' ) << std::setfill ( '0' ) << std::setw ( 3 ) << integer << '_' << temp;
    return out.str();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make a string for image directory.
//
///////////////////////////////////////////////////////////////////////////////

std::string DiskCache::makeDirectoryString ( const std::string& cacheDir, unsigned int width, unsigned int height, unsigned int level )
{
  const std::string resolution  ( Usul::Strings::format ( 'W', width, '_', 'H', height ) );
  const std::string levelString ( Usul::Strings::format ( 'L', Helper::makeString ( level ) ) );

  std::string dir ( Usul::Strings::format ( cacheDir, resolution, '/', levelString, '/' ) );
  std::replace ( dir.begin(), dir.end(), '\\', '/' );
  return dir;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make a string for extents.
//
///////////////////////////////////////////////////////////////////////////////

std::string DiskCache::makeExtentsString ( const Extents& extents )
{
  std::string file ( Usul::Strings::format ( 
                     Helper::makeString ( extents.minimum()[0] ), '_', 
                     Helper::makeString ( extents.minimum()[1] ), '_', 
                     Helper::makeString ( extents.maximum()[0] ), '_', 
                     Helper::makeString ( extents.maximum()[1] ) ) );
  std::replace ( file.begin(), file.end(), '.', '-' );

  return file;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make a string for level.
//
///////////////////////////////////////////////////////////////////////////////

std::string DiskCache::makeLevelString ( unsigned int level )
{
  return Usul::Strings::format ( 'L', Helper::makeString ( level ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the cache directory for the layer name and id.
//
///////////////////////////////////////////////////////////////////////////////

std::string DiskCache::buildCacheDir ( const std::string &layerName, const std::size_t hashValue )
{
  const std::string rootDir ( DiskCache::instance().cacheDirectory() );
  std::string dir ( Usul::Strings::format ( rootDir, '/', layerName, '/', hashValue, '/' ) );
  std::replace ( dir.begin(), dir.end(), '\\', '/' );

  return dir;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the cache filename for layer key and tile key.
//
///////////////////////////////////////////////////////////////////////////////

std::string DiskCache::getCacheDirectory ( const LayerKey& layerKey, const TileKey& tileKey, unsigned int width, unsigned int height ) const
{
  const std::string cacheDir ( Minerva::Core::DiskCache::buildCacheDir ( layerKey.name(), layerKey.id() ) );
  const std::string baseDir ( Minerva::Core::DiskCache::makeDirectoryString ( cacheDir, width, height, tileKey.level() ) );
  return baseDir;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the cache filename for layer key and tile key.
//
///////////////////////////////////////////////////////////////////////////////

std::string DiskCache::getCacheDirectory ( const LayerKey& layerKey, const TileKey& tileKey ) const
{
  return this->getCacheDirectory ( layerKey, tileKey, 0, 0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Delete the cache.
//
///////////////////////////////////////////////////////////////////////////////

void DiskCache::deleteCache ( const LayerKey& layerKey )
{
  const std::string directory ( Minerva::Core::DiskCache::buildCacheDir ( layerKey.name(), layerKey.id() ) );

  if ( boost::filesystem::exists ( directory ) && boost::filesystem::is_directory ( directory ) )
    boost::filesystem::remove_all ( directory );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the cache filename.
//
///////////////////////////////////////////////////////////////////////////////

DiskCache::CacheStatus DiskCache::getAndCheckCacheFilename ( const LayerKey& layerKey,
                                                             const TileKey& key,
                                                             unsigned int width,
                                                             unsigned int height,
                                                             const std::string& extension,
                                                             std::string& filename )
{
  if ( true == layerKey.name().empty() )
    return CACHE_STATUS_FILE_NAME_ERROR;

  // Make the directory. Guard it so that it's atomic.
  const std::string baseDir ( Minerva::Core::DiskCache::instance().getCacheDirectory ( layerKey, key, width, height ) );
  {
    Guard guard ( *_writerMutex );
    boost::filesystem::create_directories ( baseDir );
  }

  // Make the file name.
  filename = Usul::Strings::format (
    baseDir, Minerva::Core::DiskCache::makeExtentsString ( key.extents() ), '.', extension );

  // If the file does not exist then return.
  if ( false == boost::filesystem::exists ( filename ) )
    return CACHE_STATUS_FILE_DOES_NOT_EXIST;

  // If the file is empty then remove it.
  if ( 0 == boost::filesystem::file_size ( filename ) )
  {
    boost::filesystem::remove ( filename );
  }

  return CACHE_STATUS_FILE_OK;
}
