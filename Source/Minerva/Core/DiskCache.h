
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

#ifndef __MINERVA_CORE_DISK_CACHE_H__
#define __MINERVA_CORE_DISK_CACHE_H__

#include "Minerva/Core/Export.h"

#include "Minerva/Common/Extents.h"
#include "Minerva/Common/IReadImageFile.h"
#include "Minerva/Common/LayerKey.h"
#include "Minerva/Common/TileKey.h"

#include "osg/Vec2d"
#include "osg/Image"

#include <string>

namespace Usul { namespace Threads { class Mutex; } }

namespace Minerva {
namespace Core {


class MINERVA_EXPORT DiskCache
{
public:

  typedef Minerva::Common::Extents Extents;
  typedef osg::ref_ptr<osg::Image> ImagePtr;
  typedef Minerva::Common::IReadImageFile IReadImageFile;
  typedef IReadImageFile::RefPtr ReaderPtr;
  typedef Minerva::Common::LayerKey LayerKey;
  typedef Minerva::Common::TileKey TileKey;

  static DiskCache& instance();

  /// Set/get the cache directory.
  void cacheDirectory ( const std::string& directory );
  std::string cacheDirectory() const;

  // Read and write.
  ImagePtr readImage ( const std::string& filename, ReaderPtr reader ) const;
  void     writeImage ( const std::string& filename, ImagePtr image );

  std::string getCacheDirectory ( const LayerKey& layerKey, const TileKey& tileKey, unsigned int width, unsigned int height ) const;
  std::string getCacheDirectory ( const LayerKey& layerKey, const TileKey& tileKey ) const;

  void deleteCache ( const LayerKey& key );

  // Make a string for filename.
  static std::string makeExtentsString ( const Extents& extents );

  enum CacheStatus
  {
    CACHE_STATUS_FILE_OK,
    CACHE_STATUS_FILE_DOES_NOT_EXIST,
    CACHE_STATUS_FILE_NAME_ERROR
  };

  CacheStatus getAndCheckCacheFilename ( const LayerKey& layerKey,
                                         const TileKey& key,
                                         unsigned int width,
                                         unsigned int height,
                                         const std::string& extension,
                                         std::string& filename );

private:

  static std::string buildCacheDir ( const std::string &layerName, const std::size_t hashValue );
  static std::string makeDirectoryString ( const std::string& cacheDir, unsigned int width, unsigned int height, unsigned int level );
  static std::string makeLevelString ( unsigned int level );

  DiskCache();
  ~DiskCache();

  Usul::Threads::Mutex *_readerMutex;
  Usul::Threads::Mutex *_writerMutex;
  Usul::Threads::Mutex *_cacheDirMutex;
  std::string _baseCacheDirectory;

  static DiskCache *_instance;
};


}
}

#endif // __MINERVA_CORE_DISK_CACHE_H__
