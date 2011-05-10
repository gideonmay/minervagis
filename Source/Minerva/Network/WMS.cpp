
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
//  Wraps up calls to the WMS servers.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Network/WMS.h"
#include "Minerva/Network/Http.h"

#include "Usul/File/Path.h"
#include "Usul/Interfaces/ICancel.h"
#include "Usul/Interfaces/ICanceledStateGet.h"
#include "Usul/System/Sleep.h"

#include "boost/filesystem.hpp"

#include <fstream>

using namespace Minerva::Network;


/////////////////////////////////////////////////////////////////////////////
//
//  Download the file.
//
/////////////////////////////////////////////////////////////////////////////

void WMS::download ( unsigned int timeoutMilliSeconds, unsigned int maxNumAttempts, Usul::Interfaces::IUnknown *caller )
{
  // Get the requested extension.
  const std::string extension ( Usul::File::extension ( _file ) );
  
  // Get file name.
  const std::string file ( ( true == extension.empty() ) ? Usul::Strings::format ( _file, '.', this->extension() ) : _file );
  
  // Get the full url.
  const std::string url ( this->fullUrl() );
  
  // Used below.
  const std::string xml ( "<?xml" );
  bool success ( false );
  
  for ( unsigned int i = 0; i < maxNumAttempts; ++i )
  {
    // Download the file.
    Minerva::Network::Http::download ( url, file, timeoutMilliSeconds, caller );
    
    // Check for file existance.
    std::ifstream in ( file.c_str(), std::ifstream::binary | std::ifstream::in );
    if ( true == in.is_open() )
    {
      // Make sure the file is not xml
      std::vector<char> header ( xml.size() + 1, '\0' );
      in.read ( &header[0], static_cast<std::streamsize> ( xml.size() ) );
      const int compare ( ::strcmp ( &header[0], xml.c_str() ) );
      if ( 0 != compare )
      {
        // This is not an xml file, so assume we got the desired file.
        success = true;
        break;
      }
    }
    
    // Check to see if we've been canceled.
    Usul::Interfaces::ICanceledStateGet::QueryPtr canceledState ( caller );
    if ( ( true == canceledState.valid() ) && ( true == canceledState->canceled() ) )
    {
      Usul::Interfaces::ICancel::QueryPtr cancelJob ( caller );
      if ( true == cancelJob.valid() )
      {
        cancelJob->cancel();
      }
    }
    
    // If we get to here then sleep to give the server a chance to recover.
    Usul::System::Sleep::milliseconds ( 500 );
  }
  
  // If it didn't work...
  if ( false == success )
  {
    boost::filesystem::remove ( file );
    throw std::runtime_error ( "Error 1822747149: failed to download: " + url );
  }
}
