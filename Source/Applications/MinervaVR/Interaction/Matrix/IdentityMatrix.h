
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, John K. Grant and Perry L. Miller IV.
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Identity matrix functor.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_FUNCTOR_IDENTITY_MATRIX_FUNCTOR_H_
#define _USUL_FUNCTOR_IDENTITY_MATRIX_FUNCTOR_H_

#include "Interaction/Matrix/MatrixFunctor.h"


namespace Usul {
namespace Functors {
namespace Interaction {
namespace Matrix {


class IdentityMatrix : public Usul::Functors::Interaction::Matrix::MatrixFunctor
{
public:

  // Useful typedef(s).
  typedef Usul::Functors::Interaction::Matrix::MatrixFunctor BaseClass;
  typedef BaseClass::Unknown Unknown;
  typedef Usul::Math::Matrix44f Matrix;

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( IdentityMatrix );

  IdentityMatrix ( Unknown *caller = 0x0, const std::string &name = "" );
  IdentityMatrix ( const IdentityMatrix &a );

  // Update the matrix.
  virtual void          operator()();

  // Clone this functor.
  virtual BaseFunctor*  clone ();

protected:

  virtual ~IdentityMatrix();
};


} // namespace Matrix
} // namespace Interaction
} // namespace Functors
} // namespace CV


#endif // _USUL_FUNCTOR_IDENTITY_MATRIX_FUNCTOR_H_
