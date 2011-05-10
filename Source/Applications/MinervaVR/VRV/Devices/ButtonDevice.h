
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Wrapper class for a single gadget::DigitalInterface.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VRJGA_BUTTON_DEVICE_H_
#define _VRJGA_BUTTON_DEVICE_H_

#include "VRV/Export.h"

#include "Usul/Base/Object.h"

#include "gadget/Type/DigitalInterface.h"

#include "boost/function.hpp"

#include <string>
#include <vector>


namespace VRV {
namespace Devices {

class VRV_EXPORT ButtonDevice : public Usul::Base::Object
{
public:

  // Useful typedefs.
  typedef Usul::Base::Object BaseClass;
  typedef gadget::DigitalInterface DI;
  typedef boost::function<void ()> Callback;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( ButtonDevice );

  // Constructor.
  ButtonDevice ( unsigned long mask, const std::string &name );

  // Add the listener.
  void addButtonPressListener ( Callback callback );

  // Remove all listeners.
  void clearButtonPressListeners();

  // Get the bit-mask for this button.
  unsigned long         mask() const;

  // Notify listeners if state changed.
  void                  notify();

  ///  Get the device state.
  unsigned long         state() const;

  /// Get the button ID.
  unsigned long         buttonID () const;

  /// Get the button name
  std::string           getButtonName() const;

protected:

  virtual ~ButtonDevice();

  void                  _notifyPressed();
  void                  _notifyReleased();

private:

  // Not copyable.
  ButtonDevice ( const ButtonDevice & );
  ButtonDevice& operator = ( const ButtonDevice & );

  typedef std::vector<Callback> ButtonPressListeners;

  DI _di;
  unsigned long _mask;
  ButtonPressListeners _pressed;
  std::string _buttonName;
};

} // namespace Devices
} // namespace VRV


#endif // _VRJGA_BUTTON_DEVICE_H_
