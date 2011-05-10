
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

#ifndef __MINERVA_CORE_RASTER_LAYER_NETWORK_H__
#define __MINERVA_CORE_RASTER_LAYER_NETWORK_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Layers/RasterLayer.h"

#include <map>
#include <string>


namespace Minerva {
namespace Core {
namespace Layers {


class MINERVA_EXPORT RasterLayerNetwork : public RasterLayer
{
public:

  typedef RasterLayer BaseClass;
  typedef std::map < std::string, std::string > Options;
  typedef BaseClass::IReadImageFile IReadImageFile;

  USUL_DECLARE_REF_POINTERS ( RasterLayerNetwork );

  RasterLayerNetwork ( const Extents &maxExtents = Extents ( -180, -90, 180, 90 ), const std::string &url = std::string(), const Options &options = Options() );

  /// The cache file extension.
  std::string           cacheFileExtension() const;

  virtual LayerKey::RefPtr cacheKey() const;
  
  /// Deserialize.
  virtual void          deserialize ( const XmlTree::Node& node );

  virtual IElevationData::RefPtr elevationData ( 
    const TileKey& key,
    unsigned int width,
    unsigned int height,
    Usul::Jobs::Job* job,
    Usul::Interfaces::IUnknown* caller );
  
  /// Set/get the options map.
  void                  options ( const Options& options );
  Options               options() const;

  /// Set/get the max number of attempts.
  void                  maxNumAttempts ( unsigned int );
  unsigned int          maxNumAttempts() const;

  /// Set/get the timeout in milliseconds.
  void                  timeoutMilliSeconds ( unsigned int );
  unsigned int          timeoutMilliSeconds() const;
  
  /// Set/get the url.
  void                  urlBase ( const std::string& url );
  std::string           urlBase() const;

  /// Get the full url.
  virtual std::string   urlFull ( const TileKey& key, unsigned int width, unsigned int height ) const;

	/// Should this layer use the network?
	bool                  useNetwork() const;
  
protected:

  virtual ~RasterLayerNetwork();
  
  RasterLayerNetwork ( const RasterLayerNetwork& );

  std::string           _getAllOptions() const;

  virtual void          _download ( const std::string& file, const TileKey& key, unsigned int width, unsigned int height, Usul::Jobs::Job *, IUnknown *caller ) = 0;

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
  RasterLayerNetwork& operator = ( const RasterLayerNetwork& );

  virtual std::string   _cacheFileExtension() const;

  void                  _destroy();
  
  void                  _findImageReader();

  void                  _registerMembers();

  void                  _setJobName ( Usul::Jobs::Job* job, const Extents& extents, const std::string& url, unsigned int level );

  void                  _downloadFailed ( const std::string &file, const std::string &url );

  std::string _url;
  Options _options;
  bool _useNetwork;
  bool _writeFailedFlags;
  bool _readFailedFlags;
  unsigned int _maxNumAttempts;
  unsigned int _timeout;

  SERIALIZE_XML_CLASS_NAME ( RasterLayerNetwork );
};


} // namespace Layers
} // namespace Core
} // namespace Minerva


#endif // __MINERVA_CORE_RASTER_LAYER_NETWORK_H__
