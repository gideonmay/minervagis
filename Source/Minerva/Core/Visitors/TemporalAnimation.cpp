
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Visitors/TemporalAnimation.h"

#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/TimeSpan.h"
#include "Minerva/Core/Data/TimeStamp.h"

using namespace Minerva::Core::Visitors;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

TemporalAnimation::TemporalAnimation ( const Date& first, const Date& last ) : BaseClass (),
  _period ( first.date(), last.date() )
{
  Minerva::Core::Data::Date end ( last );
  end.increment ( Minerva::Core::Data::Date::INCREMENT_SECOND );

  _period = boost::posix_time::time_period ( first.date(), end.date() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

TemporalAnimation::~TemporalAnimation ()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Visit.
//
///////////////////////////////////////////////////////////////////////////////

void TemporalAnimation::visit ( Minerva::Core::Data::Feature &object )
{
  Minerva::Core::Data::TimePrimitive::RefPtr timePrimitive ( object.timePrimitive() );
  if ( timePrimitive.valid() )
  {
    object.visibilitySet ( timePrimitive->isVisible ( _period ) );
  }
  else
  {
    object.visibilitySet ( true );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update min/max dates.
//
///////////////////////////////////////////////////////////////////////////////

void TemporalAnimation::visit ( Minerva::Core::Data::Container &object )
{
  TemporalAnimation::visit ( (Minerva::Core::Data::Feature& ) object );
  BaseClass::visit ( object );
}
