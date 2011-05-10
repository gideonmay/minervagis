
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Wrapper class for two instances of gadget::AnalogInterface.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VRJGA_JOYSTICK_DEVICE_H_
#define _VRJGA_JOYSTICK_DEVICE_H_

#include "VRV/Export.h"
#include "VRV/Devices/Callback.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"

#include "Usul/Math/Vector2.h"

#ifdef _MSC_VER
# pragma warning ( disable : 4290 )
#endif
#include "gadget/Type/AnalogInterface.h"

#include "gmtl/Matrix.h"

#include <string>
#include <map>


namespace VRV {
namespace Devices {


class VRV_EXPORT JoystickDevice : public Usul::Base::Referenced
{
public:

  // Useful typedefs.
  typedef Usul::Base::Referenced BaseClass;
  typedef gadget::AnalogInterface AI;
  typedef VRV::Devices::Message Message;
  typedef VRV::Devices::Callback Callback;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( JoystickDevice );

  // Constructor.
  JoystickDevice ( const std::string &h, const std::string &v );

  // Set/get the callback.
  void              callback ( Message m, Callback::Ptr c );
  Callback *        callback ( Message m );

  // Get the device's state.
  Message           horizontalState() const;
  Message           verticalState() const;

  // set the analog trim value
  void              calibrate();

  // Notify the client of any state changes.
  void              notify();

  // Update the internal state.
  void              update();

  // Set/get the name.
  void              name ( const std::string &name );
  std::string       name() const;

  // Get the joystick value in the range [-1,1].
  double            joystickHorizontal() const;
  double            joystickVertical()   const;

protected:

  virtual ~JoystickDevice();

private:

  // Get the device's raw values.
  double             horizontal() const { return _h; }
  double             vertical()   const { return _v; }

  typedef std::pair<Message,Message> MessagePair;
  typedef std::map<MessagePair,MessagePair> Transitions;
  typedef std::map<Message,Callback::Ptr> Callbacks;

  // Not copyable.
  JoystickDevice ( const JoystickDevice & );
  JoystickDevice& operator = ( const JoystickDevice & );

  void              _notify ( Message last, Message current );

  void              _sendMessage ( Message m );

  void              _updateState();

  Callbacks _cbm;
  Transitions _tm;
  AI _hai;
  AI _vai;
  double _h;
  double _v;
  Message _hs;
  Message _vs;

  Usul::Math::Vec2d                      _analogTrim;
  std::string                            _name;
};


} // namespace Devices
} // namespace VRV


#endif // _VRJGA_JOYSTICK_DEVICE_H_
