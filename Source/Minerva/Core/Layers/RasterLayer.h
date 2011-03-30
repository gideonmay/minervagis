
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_RASTER_LAYER_H__
#define __MINERVA_CORE_RASTER_LAYER_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/DiskCache.h"
#include "Minerva/Core/Data/Feature.h"

#include "Minerva/Common/Extents.h"
#include "Minerva/Common/IElevationData.h"
#include "Minerva/Common/IReadImageFile.h"
#include "Minerva/Common/LayerKey.h"
#include "Minerva/Common/TileKey.h"

#include "Serialize/XML/Macros.h"

#include "Usul/Interfaces/ILog.h"
#include "Usul/Math/Vector2.h"

#include "osg/Image"
#include "osg/ref_ptr"

#include <functional>
#include <map>

namespace Usul { namespace Jobs { class Job; } }


namespace Minerva {
namespace Core {
namespace Layers {


class MINERVA_EXPORT RasterLayer : public Minerva::Core::Data::Feature
{
public:

  typedef Minerva::Core::Data::Feature BaseClass;
  typedef BaseClass::Extents Extents;
  typedef osg::ref_ptr < osg::Image > ImagePtr;
  typedef Usul::Interfaces::IUnknown IUnknown;
  typedef Usul::Types::Uint32 Color;
  typedef std::map < Color, unsigned short > Alphas; // Unsigned short will serialize better.
  typedef Minerva::Common::IReadImageFile IReadImageFile;
  typedef IReadImageFile::RefPtr ReaderPtr;
  typedef Usul::Interfaces::ILog::RefPtr LogPtr;
  typedef Minerva::Common::IElevationData IElevationData;
  typedef Minerva::Common::LayerKey LayerKey;
  typedef Minerva::Common::TileKey TileKey;
  typedef Minerva::Core::DiskCache DiskCache;

  USUL_DECLARE_REF_POINTERS ( RasterLayer );
  
  RasterLayer();

  /// Accept the visitor.
  virtual void          accept ( Minerva::Core::Visitor& visitor );

  // Add an alpha value for the color, or an overall alpha.
  virtual void          alpha ( float );
  virtual void          alpha ( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha );

  // Get the alpha values.
  virtual float         alpha() const;
  virtual Alphas        alphas() const;
  virtual void          alphas ( const Alphas& alphas );

  virtual LayerKey::RefPtr cacheKey() const = 0;

  /// Get the raster data as elevation data.
  virtual IElevationData::RefPtr elevationData ( 
    const TileKey& key,
    unsigned int width,
    unsigned int height,
    Usul::Jobs::Job* job,
    Usul::Interfaces::IUnknown* caller );

  /// See if the given level falls within this layer's range of levels.
  virtual bool          isInLevelRange ( unsigned int level ) const;

  // Set/get the log.
  virtual void          logSet ( LogPtr );
  LogPtr                logGet();

  /// Get the texture.
  virtual ImagePtr      texture ( 
                                 const TileKey& key, 
                                 unsigned int width,
                                 unsigned int height, 
                                 Usul::Jobs::Job *, 
                                 IUnknown *caller );
  
protected:

  virtual ~RasterLayer();
  
  RasterLayer ( const RasterLayer& );

  DiskCache::CacheStatus _getAndCheckCacheFilename (
                                                   const TileKey& key, 
                                                   unsigned int width,
                                                   unsigned int height, 
                                                   std::string& filename );
  
  virtual std::string   _cacheFileExtension() const;

  static void           _checkForCanceledJob ( Usul::Jobs::Job *job );

  static std::size_t    _hashString ( const std::string &s );

  ReaderPtr             _imageReaderGet() const;
  void                  _imageReaderSet ( ReaderPtr );
  void                  _imageReaderFind ( const std::string &ext );

  void                  _logEvent ( const std::string &s );

  virtual ImagePtr      _readImageFile ( const std::string & ) const;
  static ImagePtr       _readImageFile ( const std::string &, ReaderPtr );

  // Get the texture.
  virtual ImagePtr      _textureImplementation ( 
                                                const std::string& filename, 
                                                const TileKey& key, 
                                                unsigned int width,
                                                unsigned int height,
                                                Usul::Jobs::Job *, 
                                                IUnknown *caller );

private:

  // Do not use.
  RasterLayer& operator= ( const RasterLayer& );
  
  // Register members for serialization.
  void                  _registerMembers();

  Alphas _alphas;
  float _alpha;
  IReadImageFile::RefPtr _reader;
  LogPtr _log;
  Usul::Math::Vec2ui _levelRange;

  SERIALIZE_XML_CLASS_NAME( RasterLayer )
};


} // namespace Layers
} // namespace Core
} // namespace Minerva


#endif // __MINERVA_CORE_RASTER_LAYER_H__
