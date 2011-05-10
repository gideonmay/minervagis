
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
//  WMS layer class.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "Minerva/Core/Layers/RasterLayerWms.h"
#include "Minerva/Network/Http.h"
#include "Minerva/Network/WMS.h"

#include "Usul/Factory/RegisterCreator.h"
#include "Usul/File/Path.h"
#include "Usul/File/Temp.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Jobs/Job.h"
#include "Usul/Math/Absolute.h"
#include "Usul/Registry/Database.h"
#include "Usul/Strings/Format.h"
#include "Usul/Scope/RemoveFile.h"
#include "Usul/Threads/Safe.h"

#include "XmlTree/Document.h"

#include "osgDB/ReadFile"

#include "osg/ref_ptr"
#include "osg/Image"

#include <iomanip>
#include <fstream>

using namespace Minerva::Core::Layers;

USUL_FACTORY_REGISTER_CREATOR ( RasterLayerWms );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerWms::RasterLayerWms ( const Extents &maxExtents, const std::string &url, const Options &options ) : 
  BaseClass( maxExtents, url, options )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerWms::RasterLayerWms ( const RasterLayerWms& rhs ) :
  BaseClass ( rhs )
{  
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerWms::~RasterLayerWms()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clone.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Feature* RasterLayerWms::clone() const
{
  return new RasterLayerWms ( *this );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the texture.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerWms::_download ( const std::string& file, const TileKey& key, unsigned int width, unsigned int height, Usul::Jobs::Job *job, IUnknown * )
{
  // Get url.
  std::string baseUrl ( this->urlBase() );
  if ( true == baseUrl.empty() )
    return;

  // Get all the options.
  Options options ( this->_options ( key.extents(), width, height, key.level() ) );
  if ( true == options.empty() )
    return;

  // Make wms object.
  Minerva::Network::WMS wms ( baseUrl, file, options.begin(), options.end() );

  // Download the file.
  Usul::Interfaces::IUnknown::QueryPtr caller ( job );
  wms.download ( this->timeoutMilliSeconds(), this->maxNumAttempts(), caller.get() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the full url.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayerWms::urlFull ( const TileKey& key, unsigned int width, unsigned int height ) const
{
  // Get base url.
  std::string baseUrl ( this->urlBase() );

  // Get all the options.
  Options options ( this->_options ( key.extents(), width, height, key.level() ) );

  // Ask the WMS class for the full url.
  const std::string fullUrl ( Minerva::Network::WMS::fullUrl ( baseUrl, options ) );
  return fullUrl;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get all the options.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerWms::Options RasterLayerWms::_options ( const Extents& extents, unsigned int width, unsigned int height, unsigned int level ) const
{
  // Get the base options.
  Options options ( this->options() );

  // Add bounding box. Use Usul convert for full precision.
  options[Minerva::Network::Names::BBOX] = Usul::Strings::format ( 
    Usul::Convert::Type<Extents::ValueType,std::string>::convert ( extents.minimum()[0] ), ',', 
    Usul::Convert::Type<Extents::ValueType,std::string>::convert ( extents.minimum()[1] ), ',', 
    Usul::Convert::Type<Extents::ValueType,std::string>::convert ( extents.maximum()[0] ), ',', 
    Usul::Convert::Type<Extents::ValueType,std::string>::convert ( extents.maximum()[1] ) );

  // Add width and height.
  options[Minerva::Network::Names::WIDTH]  = Usul::Convert::Type<unsigned int,std::string>::convert ( width  );
  options[Minerva::Network::Names::HEIGHT] = Usul::Convert::Type<unsigned int,std::string>::convert ( height );

  return options;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Parse the extents.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  inline RasterLayerWms::Extents parseExtents ( const XmlTree::Node & node, const RasterLayerWms::Extents& defaultExtents )
  {
    typedef XmlTree::Node::Children    Children;
    typedef XmlTree::Node::Attributes  Attributes;
    typedef Usul::Convert::Type<std::string,double> ToDouble;
    
    Children bbNode ( node.find ( "LatLonBoundingBox", false ) );
    if ( false == bbNode.empty() )
    {
      XmlTree::Node::ValidRefPtr bb ( bbNode.front() );
      Attributes& attributes ( bb->attributes() );
      
      return RasterLayerWms::Extents ( ToDouble::convert ( attributes["minx"] ),
                      ToDouble::convert ( attributes["miny"] ),               
                      ToDouble::convert ( attributes["maxx"] ),
                      ToDouble::convert ( attributes["maxy"] ) );
    }
    
    return defaultExtents;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the layer information for the server.  TODO: Handle styles.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerWms::LayerInfos RasterLayerWms::availableLayers ( const std::string& server )
{
  typedef XmlTree::Node::Children    Children;
  typedef XmlTree::Node::Attributes  Attributes;

  LayerInfos layerInfo;

  const std::string::size_type pos ( server.find_first_of ( '?' ) );
  const std::string prefix ( ( std::string::npos == pos ) ? "?" : "&" );

  // Url request.
  const std::string request ( Usul::Strings::format ( server, prefix, "request=GetCapabilities&service=WMS&version=1.1.1" ) );

  // Get the timeout.
  const unsigned int timeout ( Usul::Registry::Database::instance()["network_download"]["wms_get_capabilities"]["timeout_milliseconds"].get<unsigned int> ( 60000, true ) );

  // Download.
  std::ostringstream os;
  Minerva::Network::Http http ( request, &os );
  http.download ( timeout );

  // Open the xml document.
  XmlTree::Document::RefPtr document ( new XmlTree::Document );
  document->loadFromMemory ( os.str() );

  // Look for errors first.
  Children exceptions ( document->find ( "ServiceException", true ) );

  if ( false == exceptions.empty() )
  {
    std::string message ( Usul::Strings::format ( "Could not retrieve capabilities using request: ", request ) );
    for ( Children::const_iterator iter = exceptions.begin(); iter != exceptions.end(); ++iter )
    {
      message += Usul::Strings::format ( "  Reason: ", (*iter)->value() );
    }

    throw std::runtime_error ( message.c_str() );
  }

  Children layers ( document->find ( "Layer", true ) );

  //if ( layers.size() <= 1 )
  //  return layerInfo;

  Extents defaultExtents ( Detail::parseExtents ( *layers.front(), Extents ( -180, -90, 180, 90 ) ) );

  for ( Children::const_iterator iter = layers.begin(); iter != layers.end(); ++iter )
  {
    Children nameNode ( (*iter)->find ( "Name", false ) );
    Children titleNode ( (*iter)->find ( "Title", false ) );

    std::string name ( nameNode.empty() ? "" : nameNode.front()->value() );
    std::string title ( titleNode.empty() ? "" : titleNode.front()->value() );

    LayerInfo info;
    info.name = name;
    info.title = title;
    info.extents = Detail::parseExtents ( *(*iter), defaultExtents );

    layerInfo.push_back ( info );
  }

  return layerInfo;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the default options.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerWms::Options RasterLayerWms::defaultOptions()
{
  Options options;

  options[Minerva::Network::Names::REQUEST] = "GetMap";
  options[Minerva::Network::Names::SRS    ] = "EPSG:4326";
  options[Minerva::Network::Names::VERSION] = "1.1.1";
  options["service"] = "WMS";

  return options;
}
