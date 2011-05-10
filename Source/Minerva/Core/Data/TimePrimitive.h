
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_TIME_PRIMITIVE_H__
#define __MINERVA_CORE_DATA_TIME_PRIMITIVE_H__

#include "Minerva/Core/Data/Object.h"

#include "boost/date_time/posix_time/time_period.hpp"

#include <string>

namespace Minerva { namespace Core { namespace Data { class Date; } } }

namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT TimePrimitive : public Minerva::Core::Data::Object
{
public:
  typedef Minerva::Core::Data::Object BaseClass;
  typedef Minerva::Core::Data::Date Date;
  
  USUL_DECLARE_REF_POINTERS ( TimePrimitive );

  virtual bool isVisible ( const boost::posix_time::time_period& ) const = 0;

protected:
  
  TimePrimitive();
  virtual ~TimePrimitive();

  bool _isVisible ( const Date& begin, const Date& end, const boost::posix_time::time_period& ) const;

};


}
}
}


#endif // __MINERVA_CORE_DATA_TIME_PRIMITIVE_H__
