
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_VISITORS_TEMPORAL_ANIMATION_H__
#define __MINERVA_VISITORS_TEMPORAL_ANIMATION_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Visitor.h"
#include "Minerva/Core/Data/Date.h"

namespace Minerva {
namespace Core {
namespace Visitors {

class MINERVA_EXPORT TemporalAnimation : public Minerva::Core::Visitor
{
public:
  typedef Minerva::Core::Visitor BaseClass;
  typedef Minerva::Core::Data::Date Date;

  USUL_DECLARE_REF_POINTERS ( TemporalAnimation );

  TemporalAnimation ( const Date& first, const Date& last );

  virtual void visit ( Minerva::Core::Data::Feature &object );
  virtual void visit ( Minerva::Core::Data::Container &object );

protected:
  
  /// Do not use.
  TemporalAnimation ();

  /// Use reference counting.
  virtual ~TemporalAnimation ();

private:
  boost::posix_time::time_period  _period;
};


}
}
}

#endif // __MINERVA_VISITORS_TEMPORAL_ANIMATION_H__
