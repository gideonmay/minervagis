
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Specialized layer for city names.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/OSM/CityNameLayer.h"
#include "Minerva/Plugins/OSM/TileVectorJob.h"
#include "Minerva/Plugins/OSM/Functions.h"
#include "Minerva/Plugins/OSM/Parser.h"

#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/Point.h"

#include "Usul/Factory/RegisterCreator.h"
#include "Usul/Strings/Format.h"
#include "Usul/User/Directory.h"

using namespace Minerva::Layers::OSM;

USUL_FACTORY_REGISTER_CREATOR_WITH_NAME ( "OSM:CityNameLayer", CityNameLayer );

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

CityNameLayer::CityNameLayer() : BaseClass()
{
  this->_initializeCache ( "city_names" );

  /// Possible place values: http://wiki.openstreetmap.org/wiki/Key:place
  this->addRequest ( 5, Predicate ( "place", "city" ) );
  this->addRequest ( 10, Predicate ( "place", "town" ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

CityNameLayer::~CityNameLayer()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Job class.
//
///////////////////////////////////////////////////////////////////////////////

class CityNameJob : public TileVectorJob
{
public:
  typedef TileVectorJob BaseClass;

  CityNameJob ( Usul::Jobs::Manager* manager, 
    Cache::RefPtr cache, 
    const std::string& url, 
    const Extents& extents, 
    unsigned int level,
    const Predicate& predicate ) : 
    BaseClass ( manager, cache, url, extents, level, predicate )
  {
    this->priority ( static_cast<int> ( level ) );
  }

protected:

  virtual ~CityNameJob()
  {
  }

  /// Make the request.  Check the cache first.
  void _started()
  {
    this->_setStatus ( "Started" );

    XAPIMapQuery query ( this->_makeQuery() );

    Nodes nodes;
    query.makeNodesQuery ( nodes, this );

    this->_setStatus ( "Building data objects" );

    this->_buildDataObjects ( nodes );

    this->_setStatus ( "Finished" );
  }

  void _buildDataObjects ( const Nodes& nodes )
  {
    // Add all the nodes.
    for ( Nodes::const_iterator iter = nodes.begin(); iter != nodes.end(); ++iter )
    {
      OSMNodePtr node ( *iter );
      if ( node.valid() )
      {
        Usul::Math::Vec3d location (  node->location()[0], node->location()[1], 0.0 );

        Minerva::Core::Data::Point::RefPtr point ( new Minerva::Core::Data::Point );
        point->point ( location );

        Minerva::Core::Data::PointStyle::RefPtr pointStyle ( new Minerva::Core::Data::PointStyle );
        pointStyle->primitiveId ( Minerva::Core::Data::PointStyle::NONE );

        DataObject::RefPtr object ( new DataObject );
        object->geometry ( point.get() );
        object->name ( Usul::Strings::format ( node->tag<std::string> ( "name" ) ) );
        object->label ( object->name() );
        object->showLabel ( true );
        object->getOrCreateStyle()->pointstyle ( pointStyle );

        this->_addData ( object );
      }
    }
  }
};


///////////////////////////////////////////////////////////////////////////////
//
//  Launch a job for the predicate.
//
///////////////////////////////////////////////////////////////////////////////

CityNameLayer::JobPtr CityNameLayer::_launchJob ( 
    const Predicate& predicate, 
    const Extents& extents, 
    unsigned int level, 
    Usul::Jobs::Manager *manager, 
    Usul::Interfaces::IUnknown::RefPtr caller )
{
  return new CityNameJob ( manager, this->_getCache(), this->url(), extents, level, predicate );
}
