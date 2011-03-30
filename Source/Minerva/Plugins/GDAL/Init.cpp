
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Config.h"

#include "Usul/CommandLine/Arguments.h"
#include "Usul/System/Environment.h"

#include "gdal.h"
#include "ogr_api.h"
#include "cpl_csv.h"

#include "boost/filesystem.hpp"
#include "boost/version.hpp"

#include <iostream>
#include <string>

///////////////////////////////////////////////////////////////////////////////
//
//  Initialize Gdal.
//
///////////////////////////////////////////////////////////////////////////////

namespace
{
  struct Init
  {
    ///  Constructor.
    Init()
    {
#if !defined WIN32 && !defined CPL_MULTIPROC_PTHREAD
      std::cout << "Gdal was not built with thread-safety turned on, a global mutex will be used instead." << std::endl;
#endif
      
      boost::filesystem::path gdalDataDir;

      if ( Usul::System::Environment::has ( MINERVA_DATA_DIR_VARIABLE ) )
      {
        gdalDataDir = Usul::System::Environment::get ( MINERVA_DATA_DIR_VARIABLE );
      }

      else
      {
#if __APPLE__
        const std::string path ( "/../Frameworks/GDAL.framework/Resources/gdal/" );
#elif _MSC_VER
        const std::string path ( "/proj4/" );
#else 
        const std::string path ( "../share/minerva/gdal" );
#endif

        // I forget why the no_check is needed.  It's causing a link error with boost 1.44.  Need to investigate this more...
#if BOOST_VERSION >= 104400
        boost::filesystem::path programDir ( Usul::CommandLine::Arguments::instance().directory() );
#else
        boost::filesystem::path programDir ( Usul::CommandLine::Arguments::instance().directory(), boost::filesystem::no_check );
#endif
        gdalDataDir = programDir / path;
      }
      if ( false == Usul::System::Environment::has ( "GDAL_DATA" ) && boost::filesystem::is_directory ( gdalDataDir ) )
      {
#if BOOST_VERSION >= 104600
        Usul::System::Environment::set ( "GDAL_DATA", gdalDataDir.string() );
#else
        Usul::System::Environment::set ( "GDAL_DATA", gdalDataDir.native_directory_string() );
#endif
      }

      /// If there are no drivers for gdal, assume that it hasn't been initialized yet.
      if ( 0 == ::GDALGetDriverCount() )
      {
        ::GDALAllRegister();
      }

      ::OGRRegisterAll();

      ::GDALSetCacheMax ( 512 * 1024 * 1024 );
    }
    
    ///  Destructor.
    ~Init()
    {
      ::GDALDestroyDriverManager();
      ::OGRCleanupAll();
    }
    
  } _init;
}
