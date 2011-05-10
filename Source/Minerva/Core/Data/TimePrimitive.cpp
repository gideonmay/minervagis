
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/TimePrimitive.h"
#include "Minerva/Core/Data/Date.h"

#include "Usul/Convert/Convert.h"
#include "Usul/Strings/Format.h"
#include "Usul/Strings/Split.h"

#include <vector>

using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

TimePrimitive::TimePrimitive() : BaseClass()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

TimePrimitive::~TimePrimitive()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is this time primitive visible over this time period?
//
///////////////////////////////////////////////////////////////////////////////

bool TimePrimitive::_isVisible ( const Date& begin, const Date& end, const boost::posix_time::time_period& period ) const
{
  Date theEnd ( end ); theEnd.increment ( Date::INCREMENT_SECOND, 1 );

  // This is [first,last), so for proper animation, make the object's last date one day past the actual last date.
  boost::posix_time::time_period timespan ( begin.date(), theEnd.date() );
  
  const bool visible ( period.intersects ( timespan ) ? true : false );
  return visible;
}
