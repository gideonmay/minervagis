
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_INTERFACES_PER_TILE_VECTOR_DATA_H_
#define _MINERVA_INTERFACES_PER_TILE_VECTOR_DATA_H_

#include "Usul/Interfaces/IUnknown.h"

#include <list>

namespace Usul { namespace Jobs { class Manager; } }


namespace Minerva {
namespace Common {
    
    
struct ITileVectorData : public Usul::Interfaces::IUnknown
{
  /// Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( ITileVectorData );
      
  /// Id for this interface.
  enum { IID = 3934263470u };
  
  typedef std::list<Usul::Interfaces::IUnknown::RefPtr> Jobs;
  
  /// Launch the jobs to fetch vector data.
  virtual Jobs                  launchVectorJobs ( double minLon, 
                                                   double minLat, 
                                                   double maxLon, 
                                                   double maxLat, 
                                                   unsigned int level, 
                                                   Usul::Jobs::Manager *manager,
                                                   Usul::Interfaces::IUnknown::RefPtr caller ) = 0;
};
    
    
} // namespace Interfaces
} // namespace Minerva


#endif // _MINERVA_INTERFACES_PER_TILE_VECTOR_DATA_H_
