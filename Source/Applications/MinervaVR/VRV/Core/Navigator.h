
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

#ifndef __MINERVA_VR_NAVIGATOR_H__
#define __MINERVA_VR_NAVIGATOR_H__

#include "Minerva/Core/Navigator.h"

#include "VRV/Devices/Wingman.h"

namespace VRV {
namespace Core {

class Navigator : public Usul::Base::Referenced
{
public:

  typedef Usul::Base::Referenced BaseClass;
  typedef VRV::Devices::Wingman Wingman;

  USUL_DECLARE_REF_POINTERS ( Navigator );

  Navigator();

  void body ( Minerva::Core::TileEngine::Body::RefPtr body );

  Minerva::Core::Data::Camera::RefPtr camera() const;

  // Go to the home position.
  void home();

  // Set the view matrix.
  void         viewMatrix ( const osg::Matrixd& m );

  // Calculate new view matrix based on the device's inputs.
  osg::Matrixd viewMatrix ( Wingman::RefPtr device, double frameTime );

  double minAngularSpeed() const;
  double maxAngularSpeed() const;

protected:

  virtual ~Navigator();

private:

  void _handleRotation ( const Wingman& device, double frameTime );
  void _handleElevation ( const Wingman& device, double frameTime );
  void _handleLook ( const Wingman& device, double frameTime );

  double _getRotationAmount ( double analog, double frameTime );

  static double _getClampedAnalog ( double analog );

  Minerva::Core::Navigator::RefPtr _navigator;
};

}
}

#endif 
