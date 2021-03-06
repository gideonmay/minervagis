
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, John K. Grant and Perry L. Miller IV.
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Base callback class for functors that transform the scene.
//
///////////////////////////////////////////////////////////////////////////////

#include "Interaction/Navigate/Transform.h"

using namespace Usul::Functors::Interaction::Navigate;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Transform::Transform ( 
  Unknown *caller, 
  const std::string &name, 
  Direction *dir, 
  AnalogInput *ai,
  double speed ) : 
  BaseClass ( caller, name ),
  _dir   ( dir ),
  _ai    ( ai ),
  _fi    ( caller ),
  _wi    ( caller ),
  _mm    ( caller ),
  _speed ( speed )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy constructor.
//
///////////////////////////////////////////////////////////////////////////////

Transform::Transform ( const Transform &t ) : 
  BaseClass ( t ),
  _dir   ( t._dir ),
  _ai    ( t._ai ),
  _fi    ( t._fi ),
  _wi    ( t._wi ),
  _mm    ( t._mm ),
  _speed ( t._speed )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Transform::~Transform()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update and return the analog input.
//
///////////////////////////////////////////////////////////////////////////////

double Transform::_analog()
{
  Guard guard ( this->mutex() );

  if ( true == _ai.valid() )
  {
    (*_ai)();
    return _ai->value();
  }

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update and return the direction.
//
///////////////////////////////////////////////////////////////////////////////

Direction::Vector Transform::_direction()
{
  Guard guard ( this->mutex() );

  if ( true == _dir.valid() )
  {
    (*_dir)();
    return _dir->vector();
  }

  return Direction::Vector ( 0, 0, -1 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the amount of time the last frame took.
//
///////////////////////////////////////////////////////////////////////////////

double Transform::_frameTime() const
{
  Guard guard ( this->mutex() );
  return ( ( true == _fi.valid() ) ? _fi->frameTime() : 0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the radius of the "world".
//
///////////////////////////////////////////////////////////////////////////////

double Transform::_worldRadius() const
{
  Guard guard ( this->mutex() );
  return ( ( true == _wi.valid() ) ? _wi->worldRadius() : 1 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Post multiply the current matrix.
//
///////////////////////////////////////////////////////////////////////////////

void Transform::_postMult ( const Matrix &M )
{
  Guard guard ( this->mutex() );

  if ( true == _mm.valid() )
  {
    _mm->postMultiply ( M.get() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Post multiply the current matrix.
//
///////////////////////////////////////////////////////////////////////////////

void Transform::_preMult ( const Matrix &M )
{
  Guard guard ( this->mutex() );

  if ( true == _mm.valid() )
  {
    _mm->preMultiply ( M.get() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the speed.
//
///////////////////////////////////////////////////////////////////////////////

double Transform::speed() const
{
  Guard guard ( this->mutex() );
  return _speed;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the speed.
//
///////////////////////////////////////////////////////////////////////////////

void Transform::speed ( double s )
{
  Guard guard ( this->mutex() );
  _speed = s;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the direction functor.
//
///////////////////////////////////////////////////////////////////////////////

void Transform::direction ( Direction *dir )
{
  Guard guard ( this->mutex() );
  _dir = dir;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the analog functor.
//
///////////////////////////////////////////////////////////////////////////////

void Transform::analog ( AnalogInput *ai )
{
  Guard guard ( this->mutex() );
  _ai = ai;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the caller.
//
///////////////////////////////////////////////////////////////////////////////

void Transform::caller ( Unknown* caller )
{
  // Call the base class.
  BaseClass::caller ( caller );

  // Set our internal query pointer to caller.
  Guard guard ( this->mutex() );
  _fi = caller;
  _wi = caller;
  _mm = caller;
}
