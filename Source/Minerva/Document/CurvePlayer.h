
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2004, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Interpolates a path with a B-Spline curve and animates through the curve.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PATH_ANIMATION_CURVE_PLAYER_H_
#define _PATH_ANIMATION_CURVE_PLAYER_H_

#include "Minerva/Document/Export.h"

#include "GN/Config/UsulConfig.h"
#include "GN/Splines/Curve.h"

#include "Usul/Base/Object.h"
#include "Usul/Interfaces/IUnknown.h"

#include "boost/function.hpp"

#include "osg/Matrix"

#include <stdexcept>

namespace Minerva {
namespace Document {

  class CameraPath;

class MINERVA_DOCUMENT_EXPORT CurvePlayer : public Usul::Base::Object
{
public:

  // Smart pointers.
  USUL_DECLARE_QUERY_POINTERS ( CurvePlayer );

  // Typedefs.
  typedef Usul::Base::Object BaseClass;
  typedef Usul::Interfaces::IUnknown IUnknown;
  typedef Usul::Errors::ThrowingPolicy < std::runtime_error > ErrorChecker;
  typedef GN::Config::UsulConfig < double, double, std::size_t, ErrorChecker > Config;
  typedef GN::Splines::Curve < Config > Curve;
  typedef Curve::IndependentSequence IndependentSequence;
  typedef Curve::DependentContainer DependentContainer;
  typedef Curve::DependentSequence DependentSequence;
  typedef Curve::IndependentType Parameter;
  typedef std::pair < Curve, IndependentSequence > CurveData;
  typedef osg::Matrixd Matrix;
  typedef Minerva::Document::CameraPath CameraPath;

  // Constructor.
  CurvePlayer();

  // Clear the player.
  void                          clear();

  // Go to the current parameter.
  Matrix                        go ( Parameter u );

  // Interpolate the path.
  static void                   interpolate ( const CameraPath *, unsigned int degree, bool reverse, Curve &curve, IndependentSequence &params );

  // Set/get the flag that says to loop.
  void                          looping ( bool );
  bool                          looping() const;

  // Play the path from the current paramater.
  void                          playBackward ( const CameraPath *, unsigned int degree );
  void                          playForward  ( const CameraPath *, unsigned int degree );

  // Get/set flag.
  void                          playing ( bool );
  bool                          playing() const;

  // Set/get the number of steps per knot span.
  void                          numStepsPerSpan ( unsigned int num );
  unsigned int                  numStepsPerSpan() const;

  // Update the player.
  Matrix                        update();

protected:

  // Use reference counting.
  virtual ~CurvePlayer();

  static void                   _makePathParams ( const CurveData &, unsigned int stepsPerSpan, IndependentSequence & );
  void                          _makePathParams();

  void                          _play ( const CameraPath *, unsigned int degree, bool reverse );

private:

  bool _playing;
  CurveData _curve;
  IndependentSequence _pathParams;
  unsigned int _currentStep;
  unsigned int _stepsPerSpan;
  bool _looping;
};


}
}

#endif // _PATH_ANIMATION_CURVE_PLAYER_H_
