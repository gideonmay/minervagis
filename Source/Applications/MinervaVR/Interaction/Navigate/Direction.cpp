
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, John K. Grant and Perry L. Miller IV.
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Functor for getting a direction.
//
///////////////////////////////////////////////////////////////////////////////

#include "Interaction/Navigate/Direction.h"

using namespace Usul::Functors::Interaction::Navigate;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Direction::Direction ( Unknown *unknown, const std::string &name, const Vector &dir, MatrixFunctor *mf ) : 
  BaseClass ( unknown, name ),
  _dir ( dir ),
  _original ( dir ),
  _mf ( mf )
{

  // Make sure the vectors are normalized.
  _dir.normalize();
  _original.normalize();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy constructor.
//
///////////////////////////////////////////////////////////////////////////////

Direction::Direction ( const Direction &d ) : 
  BaseClass ( d ),
  _dir ( d._dir ),
  _original ( d._original ),
  _mf ( d._mf )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Direction::~Direction()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the direction.
//
///////////////////////////////////////////////////////////////////////////////

void Direction::operator()()
{
  Guard guard ( this->mutex() );

  // If we have a valid matrix-functor...
  if ( _mf.valid() )
  {
    // Update and get the matrix.
    (*_mf)();
    const MatrixFunctor::Matrix& M = _mf->matrix();

    // Transform the direction vector and normalize.
    _dir = M * _original;
    _dir.normalize();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the original direction.
//
///////////////////////////////////////////////////////////////////////////////

Direction::Vector Direction::original() const
{
  Guard guard ( this->mutex() );
  return Direction::Vector ( _original );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the original direction.
//
///////////////////////////////////////////////////////////////////////////////

void Direction::original ( const Direction::Vector &v )
{
  Guard guard ( this->mutex() );
  _original = v;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the current direction.
//
///////////////////////////////////////////////////////////////////////////////

Direction::Vector Direction::vector() const
{
  Guard guard ( this->mutex() );
  return Direction::Vector ( _dir );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set matrix functor.
//
///////////////////////////////////////////////////////////////////////////////

void Direction::matrix ( MatrixFunctor *m )
{
  Guard guard ( this->mutex() );
  _mf = m;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clone.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Functors::Interaction::Common::BaseFunctor* Direction::clone()
{
  return new Direction ( *this );
}
