
#ifndef __VRV_DEVICES_WINGMAN_H__
#define __VRV_DEVICES_WINGMAN_H__

#include "VRV/Devices/JoystickDevice.h"
#include "VRV/Devices/ButtonDevice.h"


namespace VRV {
namespace Devices {

class Wingman : public Usul::Base::Referenced
{
 public:

  typedef Usul::Base::Referenced BaseClass;

  USUL_DECLARE_REF_POINTERS ( Wingman );

  Wingman();

  /// Calibrate all the analogs.
  void analogsCalibrate();

  void addButtonPressCallback ( const std::string& name, ButtonDevice::Callback callback );

  void update();

  JoystickDevice::RefPtr leftJoystick() const { return _leftJoystick; }
  JoystickDevice::RefPtr rightJoystick() const { return _rightJoystick; }
  JoystickDevice::RefPtr directionalPad() const { return _directionalPad; }

 protected:

  virtual ~Wingman();

  void _calibrate ( JoystickDevice::RefPtr );

 private:

  typedef std::map<std::string,ButtonDevice::RefPtr> Buttons;

  Buttons _buttons;
  JoystickDevice::RefPtr _leftJoystick;
  JoystickDevice::RefPtr _rightJoystick;
  JoystickDevice::RefPtr _directionalPad;
};


}
}

#endif
