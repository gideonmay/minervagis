
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_FACTORY_READERS_H__
#define __MINERVA_CORE_FACTORY_READERS_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Common/IReadFeature.h"

#include "Usul/Threads/RecursiveMutex.h"
#include "Usul/Threads/Guard.h"

#include <string>
#include <map>
#include <vector>

namespace Minerva {
namespace Core {
namespace Factory {


class MINERVA_EXPORT Readers
{
public:

  typedef Minerva::Common::IReadFeature::Filters Filters;
  typedef Minerva::Common::IReadFeature::RefPtr ReaderPtr;
  typedef std::vector<ReaderPtr> FeatureReaders;
  typedef Usul::Threads::RecursiveMutex Mutex;
  typedef Usul::Threads::Guard<Mutex> Guard;
  
  /// Get the instance.
  static Readers& instance();

  /// Add a creator.
  void                              add ( ReaderPtr reader );

  /// Remove a creator.
  void                              remove ( ReaderPtr reader );

  /// Read.
  Minerva::Core::Data::Feature *    read ( const std::string &filename );

  /// Get all filters.
  Filters                           filters() const;

private:

  Readers();
  ~Readers();

  static Readers *_instance;
  mutable Mutex *_mutex;
  FeatureReaders _readers;
};


  ///////////////////////////////////////////////////////////////////////////////
  //
  //  Class for automatically registering readers.
  //
  ///////////////////////////////////////////////////////////////////////////////
  
  struct RegisterReader
  {
    RegisterReader ( Minerva::Common::IReadFeature::RefPtr reader ) : _reader ( reader )
    {
      Readers::instance().add ( reader );
    }
    ~RegisterReader()
    {
      Readers::instance().remove ( _reader );
    }
    
  private:
    Minerva::Common::IReadFeature::RefPtr _reader;
  };
  
}
}
}


#endif // __MINERVA_CORE_FACTORY_READERS_H__
