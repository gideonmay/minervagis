
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "Minerva/Network/Download.h"
#include "Minerva/Network/Http.h"

#include "Usul/Functions/SafeCall.h"
#include "Usul/File/Temp.h"
#include "Usul/Registry/Database.h"
#include "Usul/Strings/Format.h"

#include "boost/algorithm/string/replace.hpp"
#include "boost/filesystem/operations.hpp"

///////////////////////////////////////////////////////////////////////////////
//
//  Download file.
//
///////////////////////////////////////////////////////////////////////////////

bool Minerva::Network::download ( const std::string& href, std::string& filename )
{
  return Minerva::Network::download ( href, filename, false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Download file.  TODO: Re-write function to handle work offline state.
//
///////////////////////////////////////////////////////////////////////////////

bool Minerva::Network::download ( const std::string& href, std::string& filename, bool useCache )
{
  bool success ( false );
  
  try
  {
    // Make the filename.
    filename = href;
    
    // Replace illegal characters for filename.
    boost::replace_first ( filename, "http://", "" );  
    boost::algorithm::replace_all ( filename, "/", "_" );
    boost::algorithm::replace_all ( filename, "?", "_" );
    boost::algorithm::replace_all ( filename, "=", "_" );
    boost::algorithm::replace_all ( filename, "&", "_" );
    boost::algorithm::replace_all ( filename, "@", "_" );
    boost::algorithm::replace_all ( filename, "\\", "/" );
    
    filename = Usul::Strings::format ( Usul::File::Temp::directory(), "/Minerva/", filename );
    
    if ( useCache && boost::filesystem::exists ( filename ) )
    {
      return true;
    }
    
    // Download.
    if ( boost::filesystem::exists ( filename ) && boost::filesystem::is_directory ( filename ) )
      boost::filesystem::remove_all ( filename );

    const unsigned int timeout ( Usul::Registry::Database::instance()["network_download"]["timeout_milliseconds"].get<unsigned int> ( 600000, true ) );
      
    success = Minerva::Network::downloadToFile ( href, filename, timeout );
  }
  USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS ( "1638679894" )
  
  return success;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Download to given filename.
//
///////////////////////////////////////////////////////////////////////////////

bool Minerva::Network::downloadToFile ( const std::string& href, const std::string& filename )
{
  const unsigned int timeout ( Usul::Registry::Database::instance()["network_download"]["timeout_milliseconds"].get<unsigned int> ( 600000, true ) );
  return downloadToFile ( href, filename, timeout );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Download to given filename.
//
///////////////////////////////////////////////////////////////////////////////

bool Minerva::Network::downloadToFile ( const std::string& href, const std::string& filename, unsigned int timeout )
{
  // See if we can use the network.
  const bool workOffline ( Usul::Registry::Database::instance()["work_offline"].get<bool> ( false, true ) );
  
  // Return now if we are suppose to work offline.
  if ( true == workOffline )
    return false;

  bool success ( false );

  try
  {
    Minerva::Network::Http::download ( href, filename, timeout );

    success = true;
  }
  USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS ( "1638679894" )

  return success;
}
