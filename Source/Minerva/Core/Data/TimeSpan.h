
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_LAYERS_KML_TIME_SPAN_H__
#define __MINERVA_LAYERS_KML_TIME_SPAN_H__

#include "Minerva/Core/Data/TimePrimitive.h"
#include "Minerva/Core/Data/Date.h"

namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT TimeSpan : public Minerva::Core::Data::TimePrimitive
{
public:
  typedef Minerva::Core::Data::TimePrimitive BaseClass;
  
  USUL_DECLARE_REF_POINTERS ( TimeSpan );
  
  TimeSpan();
  TimeSpan ( const Date& begin, const Date& end );
  
  /// Set/get the beginning date.
  void  begin ( const Date& );
  Date  begin() const;
  
  /// Set/get the ending date.
  void  end ( const Date& );
  Date  end() const;
  
  virtual bool isVisible ( const boost::posix_time::time_period& ) const;

protected:
  
  virtual ~TimeSpan();
  
private:
  
  Date _begin;
  Date _end;
};


}
}
}


#endif // __MINERVA_LAYERS_KML_TIME_SPAN_H__
