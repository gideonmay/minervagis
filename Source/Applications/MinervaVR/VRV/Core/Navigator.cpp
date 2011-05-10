
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class to manage planetary navigation
//
///////////////////////////////////////////////////////////////////////////////

#include "Navigator.h"

#include "VRV/Devices/JoystickDevice.h"

#include "Usul/Math/Constants.h"

#include "osg/CoordinateSystemNode"

using namespace VRV::Core;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Navigator::Navigator() : BaseClass(),
  _navigator ( new Minerva::Core::Navigator ( 0x0 ) )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Navigator::~Navigator()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the body.
//
///////////////////////////////////////////////////////////////////////////////

void Navigator::body ( Minerva::Core::TileEngine::Body::RefPtr body )
{
  _navigator->body ( body );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the camera.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Camera::RefPtr Navigator::camera() const
{
  return _navigator->copyCameraState();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Go home.
//
///////////////////////////////////////////////////////////////////////////////

void Navigator::home()
{
  _navigator->home();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the new view matrix.
//
///////////////////////////////////////////////////////////////////////////////

void Navigator::viewMatrix ( const osg::Matrixd& m )
{
  _navigator->viewMatrix ( m );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the new view matrix.
//
///////////////////////////////////////////////////////////////////////////////

osg::Matrixd Navigator::viewMatrix ( Wingman::RefPtr device, double frameTime )
{
  if ( device )
  {
    this->_handleRotation ( *device, frameTime );
    this->_handleElevation ( *device, frameTime );
    this->_handleLook ( *device, frameTime );
  }

  return _navigator->viewMatrix();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Handle rotation.
//
///////////////////////////////////////////////////////////////////////////////

void Navigator::_handleRotation ( const Wingman& device, double frameTime )
{
  VRV::Devices::JoystickDevice::RefPtr js ( device.leftJoystick() );
  const double dy ( this->_getRotationAmount ( js->joystickVertical(), frameTime ) );
  const double dx ( this->_getRotationAmount ( js->joystickHorizontal(), frameTime ) );

  _navigator->rotate ( dx, dy );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the rotation amount.
//
///////////////////////////////////////////////////////////////////////////////

double Navigator::_getRotationAmount ( double analog, double frameTime )
{
  // The stick value is just a unit-less scalar in the range [-1,1].
  const double stick ( Navigator::_getClampedAnalog ( analog ) ); // no units

  // This is the number of seconds since the last time we were here.
  const double interval ( frameTime ); // seconds

  const double minSpeed ( this->minAngularSpeed() );
  const double maxSpeed ( this->maxAngularSpeed() );

  const double ratio ( Usul::Math::minimum ( this->camera()->altitude() / osg::WGS_84_RADIUS_EQUATOR, 1.0 ) );
  const double delta ( maxSpeed - minSpeed );
  const double speed ( minSpeed + ( delta * ratio ) );

  //const double radians = stick * speed * interval * Usul::Math::DEG_TO_RAD;
  //return radians;

  const double degrees = stick * speed * interval;
  return degrees;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the minimum angular speed.
//
///////////////////////////////////////////////////////////////////////////////

double Navigator::minAngularSpeed() const
{
  return 0.05;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the maximum angular speed.
//
///////////////////////////////////////////////////////////////////////////////

double Navigator::maxAngularSpeed() const
{
  return 15;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Handle elevation change.
//
///////////////////////////////////////////////////////////////////////////////

void Navigator::_handleElevation ( const Wingman& device, double frameTime )
{
  VRV::Devices::JoystickDevice::RefPtr js ( device.rightJoystick() );

  const double value = Navigator::_getClampedAnalog ( js->joystickVertical() );
  const double minSpeed ( 600.0 );
  const double maxSpeed ( 5000.0 );

  const double ratio ( Usul::Math::minimum ( this->camera()->altitude() / osg::WGS_84_RADIUS_EQUATOR, 1.0 ) );
  const double delta ( maxSpeed - minSpeed );
  const double speed ( minSpeed + ( delta * ratio ) );

  const double dy ( value * speed * frameTime );
  
  _navigator->elevation ( dy );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Handle looking around.
//
///////////////////////////////////////////////////////////////////////////////

void Navigator::_handleLook ( const Wingman& device, double frameTime )
{
  VRV::Devices::JoystickDevice::RefPtr js ( device.directionalPad() );
  const double vertical ( Navigator::_getClampedAnalog ( js->joystickVertical() ) );
  const double horizontal ( Navigator::_getClampedAnalog ( js->joystickHorizontal() ) );

  const double speed ( 2 );

  const double dy ( vertical * frameTime * speed );
  const double dx ( horizontal * frameTime * speed );

  _navigator->look ( dx, dy );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clamp the analog value.
//
///////////////////////////////////////////////////////////////////////////////

double Navigator::_getClampedAnalog ( double value )
{
  if ( value > 0.15 || value < -0.15 )
  {
    return value;
  }
  return 0.0;
}
