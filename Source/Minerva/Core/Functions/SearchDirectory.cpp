
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Functions/SearchDirectory.h"
#include "Minerva/Core/Factory/Readers.h"
#include "Minerva/Core/Functions/ReadFile.h"
#include "Minerva/Core/Data/Container.h"

#include "Usul/Functions/SafeCall.h"
#include "Usul/Strings/Split.h"

#include "boost/filesystem.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/version.hpp"

#include <set>


///////////////////////////////////////////////////////////////////////////////
//
//  Typedef.
//
///////////////////////////////////////////////////////////////////////////////

typedef std::set<std::string> Extensions;

///////////////////////////////////////////////////////////////////////////////
//
//  Search directory implementation.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  void searchDirectoryImpl ( Minerva::Core::Data::Container& group, const std::string& directory, bool showFoundLayers, const Extensions& extensions )
  {
    typedef boost::filesystem::directory_iterator Iterator;
  
    Iterator iter ( directory );
    Iterator end;
    for( ; iter != end; ++iter )
    {
      const boost::filesystem::path &path = iter->path();

#if BOOST_VERSION >= 104600
      const std::string name ( path.string() );
#else
      const std::string name ( path.native_directory_string() );
#endif
      
      // Make a recursive call if its a directory.
      if ( boost::filesystem::is_directory ( path ) )
      {
        Minerva::Core::Data::Container::RefPtr subGroup ( new Minerva::Core::Data::Container );
        subGroup->visibilitySet ( showFoundLayers );
        subGroup->name ( name );
        
        // Search this directory.
        searchDirectoryImpl ( *subGroup, name, showFoundLayers, extensions );
        
        // Add if we found files that we could load.
        if ( subGroup->size() > 0 )
          group.add ( subGroup.get() );
      }
      
      // Add it to our list if its a file and the extenstion matches.
      else if ( extensions.end() != extensions.find ( boost::filesystem::extension ( name ) ) )
      {
        const std::string extension ( boost::filesystem::extension ( name ) );
        const std::string base ( boost::filesystem::basename ( name ) );
        
        /// Hack so ArcAscii grids don't get loaded more than once.
        if ( "adf" == extension && "w001001" != base )
          continue;
        
        try
        {
          // Read the file.
          Minerva::Core::Data::Feature::RefPtr feature ( Minerva::Core::Functions::readFile ( name ) );
          if ( feature )
          {
            // Set the name.
            feature->name ( base + "." + extension );
          
            // Hide by default.
            feature->visibilitySet ( showFoundLayers );
          
            // Add the unknown to the group.
            group.add ( feature );
          }
        }
        USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS ( "2020505170" );
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//
//  Search a directory (and all sub-directories) and add all readable files to group.
//
///////////////////////////////////////////////////////////////////////////////

void Minerva::Core::Functions::searchDirectory ( Minerva::Core::Data::Container& group, const std::string directory, bool showFoundLayers )
{
  typedef Minerva::Core::Factory::Readers::Filters Filters;
  typedef std::vector<std::string>     Strings;
  
  Filters filters ( Minerva::Core::Factory::Readers::instance().filters() );
  
  Extensions extensions;
  
  // Find all the extensions that we can load.
  for ( Filters::const_iterator iter = filters.begin(); iter != filters.end(); ++iter )
  {
    const std::string extension ( iter->second );
    
    Strings strings;
    Usul::Strings::split ( extension, ",", false, strings );
    for ( Strings::const_iterator iter = strings.begin(); iter != strings.end(); ++iter )
      extensions.insert ( boost::filesystem::extension ( *iter ) );
  }
  
  // Redirect.
  Detail::searchDirectoryImpl ( group, directory, showFoundLayers, extensions );
}
