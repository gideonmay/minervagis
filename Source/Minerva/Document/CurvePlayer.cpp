
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

#include "Minerva/Document/CurvePlayer.h"
#include "Minerva/Document/CameraPath.h"

#include "GN/Algorithms/KnotVector.h"
#include "GN/Algorithms/Parameterize.h"
#include "GN/Evaluate/Point.h"
#include "GN/Interpolate/Global.h"
#include "GN/Tessellate/Bisect.h"

#include "Usul/Scope/Reset.h"
#include "Usul/Math/MinMax.h"
#include "Usul/Math/Transpose.h"
#include "Usul/Threads/Safe.h"

#include <algorithm>

using namespace Minerva::Document;

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

CurvePlayer::CurvePlayer() : BaseClass(),
  _playing ( false ),
  _curve(),
  _pathParams(),
  _currentStep ( 0 ),
  _stepsPerSpan ( 100 ),
  _looping ( false )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

CurvePlayer::~CurvePlayer()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the step size for each knot span.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::numStepsPerSpan ( unsigned int num )
{
  Guard guard ( this );
  _stepsPerSpan = num;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the step size for each knot span.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int CurvePlayer::numStepsPerSpan() const
{
  Guard guard ( this );
  return _stepsPerSpan;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play the animation from the current parameter.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::_play ( const CameraPath *path, unsigned int degree, bool reverseOrder )
{
  Guard guard ( this );

  // Set current step.
  _currentStep = 0;

  // Initialize.
  this->playing ( false );

  // Try to make the curve.
  CurvePlayer::interpolate ( path, degree, reverseOrder, _curve.first, _curve.second );
  if ( false == _curve.first.valid() )
    return;

  // Make the path's parameters.
  this->_makePathParams();
  if ( true == _pathParams.empty() )
    return;

  // We are now playing.
  this->playing ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play the animation forward from the current parameter.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::playForward ( const CameraPath *path, unsigned int degree )
{
  Guard guard ( this );
  this->_play ( path, degree, false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play the animation backward from the current parameter.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::playBackward ( const CameraPath *path, unsigned int degree )
{
  Guard guard ( this );
  this->_play ( path, degree, true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::playing ( bool b )
{
  Guard guard ( this );
  _playing = b;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Are we playing?
//
///////////////////////////////////////////////////////////////////////////////

bool CurvePlayer::playing() const
{
  Guard guard ( this );
  return _playing;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear the curve.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::clear()
{
  Guard guard ( this );
  _curve.first.clear();
  _curve.second.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Interpolate the path.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::interpolate ( const CameraPath *path, unsigned int degree, bool reverseOrder, Curve &curve, IndependentSequence &params )
{
  // Handle bad input.
  if ( 0x0 == path )
    return;

  // For readability.
  typedef Curve::IndependentType IndependentType;
  typedef Curve::SizeType SizeType;
  typedef Curve::ErrorCheckerType ErrorCheckerType;
  typedef GN::Algorithms::KnotVector < IndependentSequence, ErrorCheckerType > KnotVectorBuilder;
  typedef GN::Algorithms::Parameterize < IndependentSequence, DependentContainer, Curve::Power, ErrorCheckerType > Parameterize;

  // Clear what we have.
  curve.clear();

  // Get a copy of the values.
  CameraPath::Values values;
  path->values ( values, reverseOrder );

  // Make sure there are no adjacent values that are identical.
  values.erase ( std::unique ( values.begin(), values.end(), CameraPath::EqualValue() ), values.end() );

  // Now that we have unique frames, make sure there are enough.
  if ( values.size() < Curve::Limits::MIN_NUM_CTR_PTS )
    return;

  // Container for the data.
  DependentContainer points;
  points.reserve ( values.size() );

  // Separate container for just the eye positions.
  DependentContainer eyes;
  eyes.reserve ( values.size() );

  // Loop through the values.
  for ( CameraPath::Values::const_iterator i = values.begin(); i != values.end(); ++i )
  {
    // Get the data.
    const CameraPath::Triplet &value ( *i );
    const CameraPath::Vec3d &eye    ( value[0] );
    const CameraPath::Vec3d &center ( value[1] );
    const CameraPath::Vec3d &up     ( value[2] );

    // Make point for just the eye position.
    DependentSequence point;
    point.push_back ( eye[0] );
    point.push_back ( eye[1] );
    point.push_back ( eye[2] );

    // Append the eye position.
    eyes.push_back ( point );

    // Continue adding the rest of the dimensions.
    point.push_back ( center[0] );
    point.push_back ( center[1] );
    point.push_back ( center[2] );
    point.push_back ( up[0] );
    point.push_back ( up[1] );
    point.push_back ( up[2] );

    // Append all dimensions (including eye position).
    points.push_back ( point );
  }

  // Transpose so that the first index is the dimension.
  Usul::Math::transpose ( eyes );
  Usul::Math::transpose ( points );

  // Fit parameters to the eye positions.
  params.clear();
  params.reserve ( values.size() );
  Parameterize::fit ( eyes, GN::Algorithms::Constants::CENTRIPETAL_FIT, params );

  // Make the knot vector. Size it for interpolation.
  const SizeType order ( Usul::Math::minimum<SizeType> ( degree + 1, params.size() ) );
  IndependentSequence knots;
  knots.resize ( params.size() + order );
  KnotVectorBuilder::build ( params, order, knots );

  // Interpolate the points.
  GN::Interpolate::global ( order, params, knots, points, curve );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Go to the current parameter.
//
///////////////////////////////////////////////////////////////////////////////

CurvePlayer::Matrix CurvePlayer::go ( Parameter u )
{
  Guard guard ( this );

  // Make sure we have a valid curve.
  if ( false == _curve.first.valid() )
    return Matrix();

  // Make sure the parameter is in range.
  if ( ( u < _curve.first.firstKnot() ) || ( u > _curve.first.lastKnot() ) )
    return Matrix();

  // Check number of dependent variables.
  if ( 9 != _curve.first.numDepVars() )
    return Matrix();

  // Evaluate the dependent variables. Have to size the point!
  Curve::Vector point ( _curve.first.numDepVars() );
  GN::Evaluate::point ( _curve.first, u, point );

  // Get the point's components.
  osg::Vec3d eye    ( point[0], point[1], point[2] );
  osg::Vec3d center ( point[3], point[4], point[5] );
  osg::Vec3d up     ( point[6], point[7], point[8] );

  // Make sure the vectors are normalized.
  up.normalize();

  // Make the matrix.
  osg::Matrixd mat ( osg::Matrixd::identity() );
  mat.makeLookAt ( eye, center, up );

  return mat;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the player.
//
///////////////////////////////////////////////////////////////////////////////

CurvePlayer::Matrix CurvePlayer::update()
{
  Guard guard ( this );

  // Return now if we're not playing.
  if ( ( false == this->playing() ) || ( true == _pathParams.empty() ) )
    return Matrix();

  // If the curve is bad, stop playing and return.
  if ( false == _curve.first.valid() )
  {
    this->playing ( false );
    return Matrix();
  }

  // Determine parameter.
  const Parameter u ( _pathParams.at ( _currentStep ) );

  // Feedback.
  if ( ( _currentStep > 0 ) && ( 0 == ( _currentStep % 100 ) ) )
  {
    std::cout << Usul::Strings::format ( "Rendering step ", _currentStep, " of ", _pathParams.size() ) << std::endl;
  }

  // Go to the parametric position.
  Matrix m ( this->go ( u ) );

  // Increment the current step.
  ++_currentStep;

  // Check to see if we're off the end.
  if ( _currentStep >= _pathParams.size() )
  {
    // Are we supposed to loop?
    if ( false == this->looping() )
    {
      // No longer playing.
      this->playing ( false );
    }
    else
    {
      // We're looping so reset the step.
      _currentStep = 0;
    }
  }

  return m;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make the path's parameters.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::_makePathParams()
{
  Guard guard ( this );
  CurvePlayer::_makePathParams ( _curve, this->numStepsPerSpan(), _pathParams );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make the path's parameters.
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::_makePathParams ( const CurveData &curve, unsigned int stepsPerSpan, IndependentSequence &pathParams )
{
  // Initialize.
  pathParams.clear();

  // Check input.
  if ( 0 == stepsPerSpan )
    return;

  // Note: totalNumControlPoints() should not throw but return 0 if the curve is empty.
  const unsigned int numControlPoints ( curve.first.totalNumControlPoints() );
  if ( numControlPoints < 2 )
    return;

  // Reserve space.
  pathParams.reserve ( stepsPerSpan * numControlPoints );

  // Loop through the spans.
  const unsigned int numSpans ( numControlPoints - 1 );
  for ( unsigned int s = 0; s < numSpans; ++s )
  {
    // The parameters to stay between.
    const Parameter u0 ( curve.second.at ( s ) );
    const Parameter u1 ( curve.second.at ( s + 1 ) );

    // Loop through the steps.
    for ( unsigned int i = 0; i < stepsPerSpan; ++i )
    {
      // Determine parameter.
      Parameter u ( static_cast<Parameter> ( i ) / static_cast<Parameter> ( stepsPerSpan - 1 ) );
      u = u0 + ( u * ( u1 - u0 ) );

      // Ensure last parameter is exactly u1.
      u = ( ( i + 1 == stepsPerSpan ) ? u1 : u );

      // Add the parameter.
      pathParams.push_back ( u );
    }
  }

  // Have to remove non-unique parameters.
  std::sort ( pathParams.begin(), pathParams.end() );
  pathParams.erase ( std::unique ( pathParams.begin(), pathParams.end() ), pathParams.end() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Are we looping?
//
///////////////////////////////////////////////////////////////////////////////

bool CurvePlayer::looping() const
{
  Guard guard ( this );
  return _looping;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Are we looping?
//
///////////////////////////////////////////////////////////////////////////////

void CurvePlayer::looping ( bool state )
{
  Guard guard ( this );
  _looping = state;
}
