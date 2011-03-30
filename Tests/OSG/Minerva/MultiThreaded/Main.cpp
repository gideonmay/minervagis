
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Program to test a Tile
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/TileEngine/Body.h"
#include "Minerva/Core/TileEngine/Tile.h"
#include "Minerva/Core/Functions/MakeBody.h"
#include "Minerva/Core/Layers/RasterLayerWMS.h"

#include "Usul/Factory/ObjectFactory.h"
#include "Usul/Jobs/Manager.h"
#include "Usul/Registry/Database.h"

#include "osgViewer/Viewer"
#include "osgViewer/ViewerEventHandlers"
#include "osgGA/StateSetManipulator"
#include "osgGA/TrackballManipulator"

int main ( int argc, char** argv )
{
  // Initialize the thread pool.
  Usul::Jobs::Manager::init ( "Job Manager", 4 );

  // Make the viewer.
  osgViewer::Viewer viewer;
  //viewer.setThreadingModel ( osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext );
  viewer.setCameraManipulator ( new osgGA::TrackballManipulator );

  // This allows 'w' -> wireframe.
  viewer.addEventHandler ( new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() ) );
  viewer.addEventHandler ( new osgViewer::ThreadingHandler );

  // Make a body.
  Minerva::Core::TileEngine::Body::RefPtr body ( Minerva::Core::Functions::makeEarth ( &Usul::Jobs::Manager::instance() ) );

  {
    const Minerva::Core::Layers::RasterLayerWms::Extents extents ( -180.0, -90.0, 180.0, 90.0 );
    const std::string url ( "http://hypercube.telascience.org/cgi-bin/landsat7" );
    Minerva::Core::Layers::RasterLayerWms::Options options;
    options["format"] = "image/jpeg";
    options["layers"] = "landsat7";
    options["styles"] = "";
    options["request"] = "GetMap";
    options["srs"] = "EPSG:4326";
    options["version"] = "1.1.1";

    Minerva::Core::Layers::RasterLayerWms::RefPtr layer ( new Minerva::Core::Layers::RasterLayerWms ( extents, url, options ) );

    body->rasterAppend ( Usul::Interfaces::IRasterLayer::QueryPtr ( layer ) );
  }

  // Add the tile to the viewer.
  viewer.setSceneData ( body->scene() );

  // Run the viewer.
  while ( !viewer.done() )
  {
    viewer.frame();

    body->purgeTiles();
  }

  // Clear the scene.
  viewer.setSceneData ( 0x0 );

  // Delete the body.
  body = 0x0;

  // Wait for all threads.
  Usul::Jobs::Manager::instance().cancel();
  Usul::Jobs::Manager::instance().wait();

  // Clear the registry.
  Usul::Registry::Database::destroy();

  // Clear the ObjectFactory.
  Usul::Factory::ObjectFactory::instance().clear();

  // Destroy the job manager.
  Usul::Jobs::Manager::destroy();

  // return the result.
  return 0;
}
