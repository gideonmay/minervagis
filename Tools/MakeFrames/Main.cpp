
#include "Minerva/Core/Data/Camera.h"
#include "Minerva/Core/DiskCache.h"

#include "Minerva/Document/AnimationController.h"
#include "Minerva/Document/OffScreenView.h"
#include "Minerva/Document/MinervaDocument.h"

#include "XmlTree/Document.h"

#include "Usul/Components/Loader.h"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include <iostream>

int main ( int argc, char** argv )
{
  boost::filesystem::path exe ( argv[0] );
  boost::filesystem::path binDir ( exe.parent_path() );
  Usul::Components::Loader<XmlTree::Document> loader;

#if BOOST_VERSION >= 104600
  loader.parse( binDir.string() + "/../configs/Minerva.plugins" );
#else
  loader.parse( binDir.native_directory_string() + "/../configs/Minerva.plugins" );
#endif

  loader.load();

  boost::program_options::options_description options ( "Allowed options" );
  options.add_options()
    ( "width", boost::program_options::value<unsigned int>(), "Width of the output image" )
    ( "height", boost::program_options::value<unsigned int>(), "Height of the output image" )
    ( "input-file", boost::program_options::value<std::string>(), "Input file" )
    ( "output-dir", boost::program_options::value<std::string>(), "Output directory." )
    ( "longitude", boost::program_options::value<double>(), "Longitude of camera" )
    ( "latitude", boost::program_options::value<double>(), "Latitude of camera" )
    ( "altitude", boost::program_options::value<double>(), "Altitude of camera" )
    ( "cache-dir", boost::program_options::value<std::string>(), "Cache directory")
    ( "help", "This message" )
  ;

  boost::program_options::variables_map vm;
  boost::program_options::store ( boost::program_options::parse_command_line ( argc, argv, options ), vm );
  boost::program_options::notify ( vm );

  if ( vm.count ( "help" ) )
  {
    std::cout << options << std::endl;
    return 1;
  }

  std::string inputFile;

  if ( vm.count ( "input-file" ) )
  {
    inputFile = vm["input-file"].as<std::string>();
  }

  boost::filesystem::path currentDirectory ( boost::filesystem::current_path() );

#if BOOST_VERSION >= 104600
  std::string directory ( currentDirectory.string() );
#else
  std::string directory ( currentDirectory.native_directory_string() );
#endif
  if ( vm.count ( "output-dir" ) )
  {
    directory = vm["output-dir"].as<std::string>();
  }

  unsigned int width ( 1280 );
  unsigned int height ( 720 );

  if ( vm.count ( "width" ) )
  {
    width = vm["width"].as<unsigned int>();
  }

  if ( vm.count ( "height" ) )
  {
    height = vm["height"].as<unsigned int>();
  }

  double longitude ( -90.0 );
  if ( vm.count ( "longitude" ) )
  {
    longitude = vm["longitude"].as<double>();
  }

  double latitude ( 0.0 );
  if ( vm.count ( "latitude" ) )
  {
    latitude = vm["latitude"].as<double>();
  }

  const double WGS_84_RADIUS_EQUATOR = 6378137.0;
  double altitude ( 2.0 * WGS_84_RADIUS_EQUATOR );
  if ( vm.count ( "altitude" ) )
  {
    altitude = vm["altitude"].as<double>();
  }

  if ( vm.count ( "cache-dir" ) )
  {
    Minerva::Core::DiskCache::instance().cacheDirectory ( vm["cache-dir"].as<std::string>() );
  }

  Minerva::Document::MinervaDocument::RefPtr document ( new Minerva::Document::MinervaDocument );
  document->read ( inputFile );

  typedef Minerva::Document::OffScreenView View;
  View::RefPtr view ( new View ( document, width, height ) );

  Minerva::Core::Data::Camera::RefPtr camera ( new Minerva::Core::Data::Camera );
  camera->latitude ( latitude );
  camera->longitude ( longitude );
  camera->altitude ( altitude );

  const std::string base ( "" );
  const std::string ext ( ".jpg" );
  view->frameDumpProperties ( directory, base, ext );

  view->resize ( width, height );
  view->showLatLonText ( false );
  view->showEyeAltitude ( false );

  // Loop to bring in needed detail.
  const unsigned int times ( 4 );
  for ( unsigned int i = 0; i < times; ++i )
  {
    view->waitForDetail ( camera );
  }

  Minerva::Core::Data::TimeSpan::RefPtr timeSpan ( document->timeSpanOfData() );

  using Minerva::Core::Data::Date;
  const bool beginValid ( Date ( boost::date_time::not_a_date_time ) != timeSpan->begin() );
  const bool endValid ( Date ( boost::date_time::not_a_date_time ) != timeSpan->end() );

  if ( beginValid && endValid )
  {
    using Minerva::Document::AnimationController;
    AnimationController::RefPtr animationController ( new AnimationController );
    animationController->globalTimeSpan ( timeSpan );

    animationController->setStepSize ( 1 );
    animationController->setCurrentTimeStep ( 0 );

    while ( AnimationController::ANIMATION_RESULT_AT_END != animationController->stepForward() )
    {
      // Bring in needed detail.
      view->waitForDetail ( camera );

      document->visibleTimeSpan ( animationController->visibleTimeSpan() );

      view->render ( camera );
    }
  }
  else
  {
    view->render ( camera );
  }

  view = 0x0;
  document = 0x0;

  return 0;
}
