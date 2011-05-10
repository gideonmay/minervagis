
#include "Wingman.h"

using namespace VRV::Devices;

Wingman::Wingman() : 
  BaseClass(),
  _buttons(),
  _leftJoystick ( new JoystickDevice ( "VJAnalog2", "VJAnalog3" ) ),
  _rightJoystick ( new JoystickDevice ( "VJAnalog0", "VJAnalog1") ),
  _directionalPad ( new JoystickDevice ( "VJAnalog4", "VJAnalog5") )
{
  _buttons.insert ( std::make_pair ( "VJButton0", new ButtonDevice ( 0x00000001, "VJButton0" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton1", new ButtonDevice ( 0x00000002, "VJButton1" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton2", new ButtonDevice ( 0x00000004, "VJButton2" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton3", new ButtonDevice ( 0x00000008, "VJButton3" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton4", new ButtonDevice ( 0x00000010, "VJButton4" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton5", new ButtonDevice ( 0x00000020, "VJButton5" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton6", new ButtonDevice ( 0x00000040, "VJButton6" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton7", new ButtonDevice ( 0x00000080, "VJButton7" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton8", new ButtonDevice ( 0x00000100, "VJButton8" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton9", new ButtonDevice ( 0x00000200, "VJButton9" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton10", new ButtonDevice ( 0x00000400, "VJButton10" ) ) );
  _buttons.insert ( std::make_pair ( "VJButton11", new ButtonDevice ( 0x00000800, "VJButton11" ) ) );
}

Wingman::~Wingman()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Calibrate all the analogs. This assumes the user is not tilting the joystick one way 
//  or the other. It records the value at the neutral position. If the value 
//  is 0.5 (like it should be) then the "trim" will be zero.
//
///////////////////////////////////////////////////////////////////////////////

void Wingman::analogsCalibrate()
{
  this->_calibrate ( _leftJoystick );
  this->_calibrate ( _rightJoystick );
  this->_calibrate ( _directionalPad );
}

void Wingman::_calibrate ( JoystickDevice::RefPtr joystick )
{
  if ( joystick.valid() )
  {
    joystick->calibrate();
  }
}


void Wingman::addButtonPressCallback ( const std::string& name, ButtonDevice::Callback callback )
{
  Buttons::iterator iter ( _buttons.find ( name ) );
  if ( iter != _buttons.end() )
  {
    iter->second->addButtonPressListener ( callback );
  }
}

void Wingman::update()
{
  _leftJoystick->update();
  _rightJoystick->update();
  _directionalPad->update();

  for ( Buttons::iterator iter = _buttons.begin(); iter != _buttons.end(); ++iter )
  {
    iter->second->notify();
  }
}
