
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, John K. Grant and Perry L. Miller IV.
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Functor sequence.
//
///////////////////////////////////////////////////////////////////////////////

#include "Interaction/Common/Sequence.h"

using namespace Usul::Functors::Interaction::Common;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Sequence::Sequence ( Unknown *caller, const std::string &name ) : 
  BaseClass ( caller, name ),
  _functors ()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy constructor.
//
///////////////////////////////////////////////////////////////////////////////

Sequence::Sequence ( const Sequence &s ) : BaseClass ( s ),
  _functors ( s._functors )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Sequence::~Sequence()
{
  _functors.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Have the functors do their thing.
//
///////////////////////////////////////////////////////////////////////////////

void Sequence::operator()()
{
  Guard guard ( this->mutex() );
  for ( Functors::iterator i = _functors.begin(); i != _functors.end(); ++i )
  {
    BaseFunctor::RefPtr functor ( i->get() );
    if ( true == functor.valid() )
    {
      (*functor)();
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Append a functor.
//
///////////////////////////////////////////////////////////////////////////////

void Sequence::append ( BaseClass *functor )
{
  if ( 0x0 != functor )
  {
    Guard guard ( this->mutex() );
    _functors.push_back ( BaseFunctor::RefPtr ( functor ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the caller.
//
///////////////////////////////////////////////////////////////////////////////

void Sequence::caller ( Unknown* caller )
{
  BaseClass::caller ( caller );

  Guard guard ( this->mutex() );
  for ( Functors::iterator i = _functors.begin(); i != _functors.end(); ++i )
  {
    BaseFunctor::RefPtr functor ( i->get() );
    if ( true == functor.valid() )
    {
      functor->caller ( caller );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clone.
//
///////////////////////////////////////////////////////////////////////////////

BaseFunctor* Sequence::clone()
{
  return new Sequence ( *this );
}
