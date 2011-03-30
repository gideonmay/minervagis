
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Helper functions to parse xml and create DataObjects.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/OSM/Functions.h"

#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/Line.h"
#include "Minerva/Core/Data/Point.h"
#include "Minerva/Core/Data/MultiPoint.h"
#include "Minerva/Core/Data/TimeStamp.h"
#include "Minerva/Core/Data/Style.h"

#include "Usul/Convert/Convert.h"

#include "boost/foreach.hpp"

using Minerva::Layers::OSM::DataObject;
using Minerva::Layers::OSM::Extents;


///////////////////////////////////////////////////////////////////////////////
//
//  Create a data object.
//
///////////////////////////////////////////////////////////////////////////////

DataObject* Minerva::Layers::OSM::createForAllNodes ( const Nodes& nodes )
{
  // Make a vertex array.
  Minerva::Common::Coordinates::RefPtr points ( new Minerva::Common::Coordinates );
  points->reserve ( nodes.size() );
  
  // All all the points.
  for ( Nodes::const_iterator iter = nodes.begin(); iter != nodes.end(); ++iter )
  {
    OSMNodePtr node ( *iter );
    if ( node.valid() )
    {
      points->addPoint ( node->location()[0], node->location()[1], 0.0 );
    }
  }
  
  // Make a point.
  Minerva::Core::Data::MultiPoint::RefPtr point ( new Minerva::Core::Data::MultiPoint );
  point->coordinates ( points );

  Minerva::Core::Data::Style::RefPtr style ( new Minerva::Core::Data::Style );
  Minerva::Core::Data::PointStyle::RefPtr pointStyle ( new Minerva::Core::Data::PointStyle );
  style->pointstyle ( pointStyle );

  pointStyle->size ( 6.0 );
  pointStyle->color ( Usul::Math::Vec4f ( 1.0, 0.0, 0.0, 1.0 ) );
  
  DataObject::RefPtr object ( new DataObject );
  object->geometry ( point.get() );
  object->name ( "All Nodes" );
  object->style ( style );
  
  return object.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create a data object.
//
///////////////////////////////////////////////////////////////////////////////

DataObject* Minerva::Layers::OSM::createForNode ( const Minerva::Layers::OSM::Node& node )
{
  // Make a point.
  Minerva::Core::Data::Point::RefPtr point ( new Minerva::Core::Data::Point );
  point->point ( Usul::Math::Vec3d (  node.location()[0], node.location()[1], 0.0 ) );

  Minerva::Core::Data::PointStyle::RefPtr pointStyle ( new Minerva::Core::Data::PointStyle );
  pointStyle->size ( 10.0 );
  pointStyle->color ( Usul::Math::Vec4f ( 1.0, 0.0, 0.0, 1.0 ) );

  DataObject::RefPtr object ( new DataObject );
  object->geometry ( point.get() );
  object->name ( Usul::Strings::format ( node.id() ) );
  object->timePrimitive ( new Minerva::Core::Data::TimeStamp ( node.timestamp() ) );
  object->getOrCreateStyle()->pointstyle ( pointStyle );

  return object.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create a data object.
//
///////////////////////////////////////////////////////////////////////////////

DataObject* Minerva::Layers::OSM::createForWay  ( const Minerva::Layers::OSM::Way&  way  )
{
  // Make a line.
  Minerva::Core::Data::Line::RefPtr line ( new Minerva::Core::Data::Line );
  Minerva::Core::Data::Line::Vertices vertices;
  vertices.reserve ( way.numNodes() );

  for ( unsigned int i = 0; i < way.numNodes(); ++i )
  {
    OSMNodePtr node ( way.node ( i ) );

    if ( true == node.valid() )
      vertices.push_back ( Usul::Math::Vec3d ( node->location()[0], node->location()[1], 0.0 ) );
  }

  line->line ( vertices );

  Minerva::Core::Data::Style::RefPtr style ( new Minerva::Core::Data::Style );
  style->linestyle (  Minerva::Core::Data::LineStyle::create ( Usul::Math::Vec4f ( 1.0, 1.0, 0.0, 1.0 ), 2.0 ) );

  DataObject::RefPtr object ( new DataObject );
  object->style ( style );
  object->geometry ( line.get() );
  object->name ( Usul::Strings::format ( way.id() ) );
  object->timePrimitive ( new Minerva::Core::Data::TimeStamp ( way.timestamp() ) );

  return object.release();
}
