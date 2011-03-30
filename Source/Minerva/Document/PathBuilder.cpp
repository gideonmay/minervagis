
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Build a path for animation.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Document/PathBuilder.h"

#include "Minerva/Core/Data/Camera.h"
#include "Minerva/Core/Data/Feature.h"

#include "Minerva/OsgTools/ConvertMatrix.h"

#include "GN/Algorithms/Fill.h"
#include "GN/Algorithms/KnotVector.h"
#include "GN/Config/UsulConfig.h"
#include "GN/Evaluate/Point.h"
#include "GN/Interpolate/Global.h"
#include "GN/Splines/Curve.h"

#include "Usul/Math/Constants.h"
#include "Usul/Math/Transpose.h"

using namespace Minerva::Document;


///////////////////////////////////////////////////////////////////////////////
//
//  Get the distance in meters between two points.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  double distance ( const Usul::Math::Vec3d& from, const Usul::Math::Vec3d& to, Minerva::Core::TileEngine::LandModel& landModel )
  {
    Usul::Math::Vec3d p0, p1;
    landModel.latLonHeightToXYZ ( from[0], from[1], from[2], p0[0], p0[1], p0[2] );
    landModel.latLonHeightToXYZ ( to[0], to[1], to[2], p1[0], p1[1], p1[2] );
    const double d ( p0.distance ( p1 ) );
    return d;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Use trig to slow down the beginning and end of the path.
//  See http://mathworld.wolfram.com/Cosine.html
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  inline double slowBothEnds ( double param, double mn, double mx )
  {
    param *= Usul::Math::PIE;
    param += Usul::Math::PIE;
    param  = Usul::Math::cos ( param );
    param += 1.0;
    param /= 2.0;
    
    // Keep in range.
    param = ( ( param < mn ) ? mn : param );
    param = ( ( param > mx ) ? mx : param );
    
    return param;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make a camera to view the feature.
//
///////////////////////////////////////////////////////////////////////////////

PathBuilder::Camera* PathBuilder::makeCamera ( const Feature& feature, const Minerva::Core::TileEngine::Body& body )
{
  Minerva::Common::Extents extents ( feature.extents() );
  
  // Get the center.
  Minerva::Common::Extents::Vertex center ( extents.center() );
  
  // For now use this formula to get meters per degree.
  // http://books.google.com/books?id=wu7zHtd2LO4C&pg=PA167&lpg=PA167&dq=meters+per+degree+average+earth&source=web&ots=yF1Q6sp1nP&sig=S8gdbKLvNzrXGKoUn3ha-oqYMaU&hl=en
  const double lat ( Usul::Math::DEG_TO_RAD * center[1] );
  const double metersPerDegree ( 111132.09 - 566.05 * Usul::Math::sin ( 2 * lat ) + 120 * Usul::Math::cos ( 4 * lat ) - 0.0002 * Usul::Math::cos ( 6 * lat ) );
  
  // Calculate an altitude.
  const double altitude ( ( extents.maximum() - extents.minimum() ).length() * metersPerDegree );
  
  Camera::RefPtr camera ( new Camera );
  camera->longitude ( center[0] );
  camera->latitude ( center[1] );
  camera->altitude ( Usul::Math::maximum ( 2500.0, altitude ) );
  
  return camera.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  View the layer extents.
//
///////////////////////////////////////////////////////////////////////////////

void PathBuilder::lookAtLayer ( Camera* from, Feature * layer, Minerva::Core::TileEngine::Body::RefPtr body, Matrices& matrices )
{
  if ( 0x0 == layer || 0x0 == from || !body.valid() )
  {
    return;
  }
  
  Camera::RefPtr to ( PathBuilder::makeCamera ( *layer, *body ) );
  PathBuilder::generateAnimatePath ( from, to.get(), 80, body->landModel(), matrices );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Go to a point.
//
///////////////////////////////////////////////////////////////////////////////

void PathBuilder::lookAtPoint ( Camera* from, const Usul::Math::Vec2d& location, Minerva::Core::TileEngine::Body::RefPtr body, Matrices& matrices )
{
  if ( 0x0 == from || !body.valid() )
  {
    return;
  }

  // Point we want to go to.
  Camera::RefPtr to ( new Camera );
  to->longitude ( location[0] );
  to->latitude ( location[1] );
  to->altitude ( 2500.0 );
    
  PathBuilder::generateAnimatePath ( from, to.get(), 80, body->landModel(), matrices );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create view matrix from camera.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  Usul::Math::Matrix44d createViewMatrix ( PathBuilder::Camera& camera, Minerva::Core::TileEngine::LandModel* landModel )
  {
    Usul::Math::Matrix44d matrix ( camera.viewMatrix ( landModel ) );
    Usul::Math::Matrix44d viewMatrix;
    
    matrix.inverse ( viewMatrix );
    return viewMatrix;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Generate the path to animate from start to end.
//
///////////////////////////////////////////////////////////////////////////////

void PathBuilder::generateAnimatePath ( 
                                  Camera* start, 
                                  Camera* end, 
                                  unsigned int numPoints,
                                  Minerva::Core::TileEngine::LandModel* landModel, 
                                  Matrices& matrices )
{
  if ( 0x0 == landModel || 0x0 == start || 0x0 == end || 0 == numPoints )
    return;
  
  matrices.reserve ( numPoints + 1 );
  
#if 0
  const double deltaLongitude ( end->longitude() - start->longitude() );
  const double deltaLatitude ( end->latitude() - start->latitude() );
  const double deltaAltitude ( end->altitude() - start->altitude() );
  
  for ( unsigned int i = 0; i < numPoints; ++i )
  {
    const double u ( static_cast<double> ( i ) / ( numPoints - 1 ) );
    
    Minerva::Core::Data::Camera::RefPtr currentFrame ( new Minerva::Core::Data::Camera );
    currentFrame->longitude ( start->longitude() + ( u * ( deltaLongitude ) ) );
    currentFrame->latitude ( start->latitude() + ( u * ( deltaLatitude ) ) );
    currentFrame->altitude ( start->altitude() + ( u * ( deltaAltitude ) ) );
    currentFrame->heading ( start->heading() + ( u * ( end->heading() - start->heading() ) ) );
    currentFrame->tilt ( start->tilt() + ( u * ( end->tilt() - start->tilt() ) ) );
    currentFrame->roll ( start->roll() + ( u * ( end->roll() - start->roll() ) ) );
    
    matrices.push_back ( Detail::createViewMatrix ( *currentFrame, landModel ) );
  }
  
  // Add one more so we end up where we are suppose to.
  matrices.push_back ( Detail::createViewMatrix ( *end, landModel ) );
#else
  
  // Needed below.
  typedef Usul::Errors::ThrowingPolicy < std::runtime_error > ErrorCheckerType;
  typedef GN::Config::UsulConfig < double, double, unsigned int, ErrorCheckerType > Config;
  typedef GN::Splines::Curve < Config > Curve;
  
  typedef Curve::IndependentSequence IndependentSequence;
  typedef Curve::DependentContainer DependentContainer;
  typedef Curve::DependentSequence DependentSequence;
  typedef Curve::IndependentType Parameter;
  typedef Curve::IndependentType IndependentType;
  typedef Curve::SizeType SizeType;
  typedef GN::Algorithms::KnotVector < IndependentSequence, ErrorCheckerType > KnotVectorBuilder;
  typedef GN::Algorithms::Parameterize < IndependentSequence, DependentContainer, Curve::Power, ErrorCheckerType > Parameterize;
  
  // Container for the data.
  DependentContainer points;
  
  // Separate container for just the eye positions.
  DependentContainer eyes;
  
  {
    DependentSequence point;
    point.push_back ( start->longitude() );
    point.push_back ( start->latitude() );
    point.push_back ( start->altitude() );
    
    eyes.push_back ( point );
    
    point.push_back ( start->heading() );
    point.push_back ( start->tilt() );
    point.push_back ( start->roll() );
    
    points.push_back ( point );
  }
  
  {
    DependentSequence point;
    point.push_back ( end->longitude() );
    point.push_back ( end->latitude() );
    point.push_back ( end->altitude() );
    
    eyes.push_back ( point );
    
    point.push_back ( end->heading() );
    point.push_back ( end->tilt() );
    point.push_back ( end->roll() );
    
    points.push_back ( point );
  }

  // Transpose so that the first index is the dimension.
  Usul::Math::transpose ( eyes );
  Usul::Math::transpose ( points );
  
  IndependentSequence params;
  Parameterize::fit ( eyes, GN::Algorithms::Constants::CENTRIPETAL_FIT, params );
  
  // Make the knot vector. Size it for interpolation.
  const unsigned int degree ( 4 );
  const SizeType order ( Usul::Math::minimum<SizeType> ( degree + 1, params.size() ) );
  IndependentSequence knots;
  knots.resize ( params.size() + order );
  KnotVectorBuilder::build ( params, order, knots );
  
  // Interpolate the points.
  Curve curve;
  GN::Interpolate::global ( order, params, knots, points, curve );
  
  for ( unsigned int i = 0; i < numPoints; ++i )
  {
    double param ( static_cast<double> ( i ) / ( numPoints - 1 ) );
    param = Detail::slowBothEnds ( param, 0.0, 1.0 );
    param = Detail::slowBothEnds ( param, 0.0, 1.0 );
    
    // Evaluate the dependent variables. Have to size the point!
    Curve::Vector point ( curve.numDepVars() );
    
    GN::Evaluate::point ( curve, param, point );
    
    Minerva::Core::Data::Camera::RefPtr currentFrame ( new Minerva::Core::Data::Camera );
    currentFrame->longitude ( point[0] );
    currentFrame->latitude ( point[1] );
    currentFrame->altitude ( point[2] );
    currentFrame->heading ( point[3] );
    currentFrame->tilt ( point[4] );
    currentFrame->roll ( point[5] );
    
    matrices.push_back ( Detail::createViewMatrix ( *currentFrame, landModel ) );
  }
  
#endif
}
