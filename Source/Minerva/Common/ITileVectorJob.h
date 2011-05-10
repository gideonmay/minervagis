
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_INTERFACES_PER_TILE_VECTOR_JOB_H_
#define _MINERVA_INTERFACES_PER_TILE_VECTOR_JOB_H_

#include "Usul/Interfaces/IUnknown.h"

#include "Minerva/Core/Data/Feature.h"

#include <vector>


namespace Minerva {
namespace Common {
    
    
struct ITileVectorJob : public Usul::Interfaces::IUnknown
{
  /// Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( ITileVectorJob );
      
  /// Id for this interface.
  enum { IID = 3703891219u };
  
  typedef std::vector<Minerva::Core::Data::Feature::RefPtr> Data;

  /// Cancel the job.
  virtual void                  cancelVectorJob() = 0;
  
  /// Get the container of data.
  virtual void                  takeVectorData ( Data& ) = 0;

  /// Is the job done?
  virtual bool                  isVectorJobDone() const = 0;
};
    
    
} // namespace Interfaces
} // namespace Minerva


#endif // _MINERVA_INTERFACES_PER_TILE_VECTOR_JOB_H_
