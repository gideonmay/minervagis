
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Helper functions for creating common bodies.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Functions/MakeBody.h"

#include "Minerva/Core/TileEngine/Body.h"
#include "Minerva/Core/TileEngine/Typedefs.h"
#include "Minerva/Core/TileEngine/LandModelEllipsoid.h"

#include "osg/CoordinateSystemNode"


///////////////////////////////////////////////////////////////////////////////
//
//  Make a body for the earth.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::TileEngine::Body* Minerva::Core::Functions::makeEarth ( Usul::Jobs::Manager *manager )
{
  // Local typedefs to shorten lines.
  typedef Minerva::Core::TileEngine::MeshSize MeshSize;
  typedef Minerva::Core::TileEngine::ImageSize ImageSize;

  // Sizes.
  MeshSize meshSize ( 16, 16 );
  ImageSize imageSize ( 256, 256 );

  // Make a good split distance.
  const double splitDistance ( osg::WGS_84_RADIUS_EQUATOR * 3.0 ); // Was 2.5

  return makeEarth ( manager, meshSize, imageSize, splitDistance );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make a body for the earth.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::TileEngine::Body* Minerva::Core::Functions::makeEarth ( 
  Usul::Jobs::Manager *manager, 
  const Minerva::Core::TileEngine::MeshSize& meshSize, 
  const Minerva::Core::TileEngine::ImageSize& imageSize,
  double splitDistance )
{
  // Local typedefs to shorten lines.
  typedef Minerva::Core::TileEngine::Extents Extents;
  typedef Minerva::Core::TileEngine::Body Body;
  typedef Minerva::Core::TileEngine::LandModelEllipsoid Land;

  // Make the land model.
  Land::Vec2d radii ( osg::WGS_84_RADIUS_EQUATOR, osg::WGS_84_RADIUS_POLAR );
  Land::RefPtr land ( new Land ( radii ) );

  // Add the body.
  Body::RefPtr body ( new Body ( 1, 2, Extents ( -180, -90, 180, 90 ), land, manager, meshSize, imageSize, splitDistance ) );
  body->name ( "Earth" );
  body->useSkirts ( true );
  
  return body.release();
}
