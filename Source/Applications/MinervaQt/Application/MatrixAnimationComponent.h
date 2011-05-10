
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2004, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  The component class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MATRIX_ANIMATION_COMPONENT_CLASS_H_
#define _MATRIX_ANIMATION_COMPONENT_CLASS_H_

#include "Usul/Interfaces/IAnimateMatrices.h"

#include "QtCore/QTimer"

class MatrixAnimationComponent : public QObject
{
  Q_OBJECT;
public:

  // Typedefs.
  typedef Usul::Interfaces::IAnimateMatrices IAnimateMatrices;
  typedef IAnimateMatrices::Matrices Matrices;

  // Default construction.
  MatrixAnimationComponent();
  virtual ~MatrixAnimationComponent();
  
  void animateMatrices ( const Matrices &, unsigned int milliSeconds );

signals:

  void setViewMatrix ( Usul::Math::Matrix44d matrix );

private slots:

  void _onTimeout();

private:

  void                          _timerStart ( unsigned int milliSeconds );
  void                          _timerStop();
  
  // Do not copy.
  MatrixAnimationComponent ( const MatrixAnimationComponent & );
  MatrixAnimationComponent &operator = ( const MatrixAnimationComponent & );

  Matrices _matrices;
  unsigned int _current;
  QTimer *_timer;
};


#endif // _MATRIX_ANIMATION_COMPONENT_CLASS_H_
