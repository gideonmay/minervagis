
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/OSM/OpenStreetMapFile.h"
#include "Minerva/Plugins/OSM/Functions.h"
#include "Minerva/Plugins/OSM/Parser.h"

#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/Line.h"
#include "Minerva/Core/Data/Point.h"
#include "Minerva/Core/Data/MultiPoint.h"
#include "Minerva/Core/Data/TimeStamp.h"
#include "Minerva/Core/Factory/Readers.h"

#include "XmlTree/Document.h"

#include "Usul/Convert/Convert.h"
#include "Usul/Factory/RegisterCreator.h"
#include "Usul/Threads/Safe.h"

#include "boost/foreach.hpp"

using namespace Minerva::Layers::OSM;


///////////////////////////////////////////////////////////////////////////////
//
//  Register readers with the factory.
//
///////////////////////////////////////////////////////////////////////////////

#if 0
namespace
{
  Minerva::Core::Factory::RegisterReader < Minerva::Core::Factory::TypeCreator < OpenStreetMapFile > > _creator_for_OSM ( "OpenStreetMap (*.osm)", "*.osm" );
}
#endif

USUL_FACTORY_REGISTER_CREATOR ( OpenStreetMapFile );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

OpenStreetMapFile::OpenStreetMapFile() : BaseClass(),
  _filename()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

OpenStreetMapFile::~OpenStreetMapFile()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read the file.
//
///////////////////////////////////////////////////////////////////////////////

void OpenStreetMapFile::read ( const std::string &filename, Usul::Interfaces::IUnknown *caller, Usul::Interfaces::IUnknown *progress )
{
  Usul::Threads::Safe::set ( this->mutex(), filename, _filename );
  
  this->_read ( filename, caller, progress );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read the file.
//
///////////////////////////////////////////////////////////////////////////////

void OpenStreetMapFile::_read ( const std::string &filename, Usul::Interfaces::IUnknown *caller, Usul::Interfaces::IUnknown *progress )
{
  XmlTree::Document::RefPtr doc ( new XmlTree::Document );
  doc->load ( filename );
  
  // Get the bounds of the data set.
  XmlTree::Node::RefPtr bounds ( doc->child ( "bounds" ) );
  if ( bounds.valid() )
    this->_setBounds ( *bounds );
  
  // Nodes and ways.
  Nodes nodes;
  Ways ways;

  // Parse.
  Parser::parseNodesAndWays ( *doc, nodes, ways );

  this->add ( Minerva::Layers::OSM::createForAllNodes ( nodes ) );
  
  // Add nodes.
#if 0
  BOOST_FOREACH ( OSMNodePtr node, nodes )
  {
    if ( true == node.valid() )
    {
      Usul::Interfaces::IUnknown::QueryPtr unknown ( Minerva::Layers::OSM::createForNode ( *node ) );
      this->add ( unknown );
    }
  }
#endif

  // Add ways.
  BOOST_FOREACH ( OSMWayPtr way, ways )
  {
    if ( true == way.valid() )
    {
      this->add ( Minerva::Layers::OSM::createForWay ( *way ) );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Deserialize.
//
///////////////////////////////////////////////////////////////////////////////

void OpenStreetMapFile::deserialize( const XmlTree::Node &node )
{
  BaseClass::deserialize ( node );
  this->read ( _filename );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Serialize.
//
///////////////////////////////////////////////////////////////////////////////

void OpenStreetMapFile::serialize ( XmlTree::Node &parent ) const
{
  Serialize::XML::DataMemberMap dataMemberMap ( Usul::Threads::Safe::get ( this->mutex(), _dataMemberMap ) );
  
  // Don't serialize the layers.
  dataMemberMap.erase ( "layers" );
  
  // Serialize.
  dataMemberMap.serialize ( parent );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the bounds.
//
///////////////////////////////////////////////////////////////////////////////

void OpenStreetMapFile::_setBounds ( const XmlTree::Node& node )
{
  Extents extents ( Parser::parseExtents ( node ) );
  this->extents ( extents );
}
