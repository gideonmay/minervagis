
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class to hold the state from a camera.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_CAMERA_STATE_H__
#define __MINERVA_CORE_DATA_CAMERA_STATE_H__

#include "Minerva/Core/Export.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Math/Matrix44.h"
#include "Usul/Pointers/Pointers.h"

namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT CameraState : public Usul::Base::Referenced
{
public:
  typedef Usul::Base::Referenced BaseClass;
  typedef Usul::Math::Matrix44d Matrix;
  
  USUL_DECLARE_REF_POINTERS ( CameraState );
  
  CameraState ( double longitude, 
                double latitude,
                double altitude,
                double heading, 
                double tilt,
                double roll,
                const Matrix& matrix );
  
  /// Get the longitude.
  double      longitude() const;
  
  /// Get the latitude.
  double      latitude() const;
  
  /// Get the altitude.
  double      altitude() const;
  
  /// Get the heading.
  double      heading() const;
  
  /// Get the tilt.
  double      tilt() const;
  
  /// Get the roll.
  double      roll() const;
  
  const Matrix &viewMatrix() const;
  
protected:
  
  virtual ~CameraState();
  
private:
  
  CameraState ( const CameraState& rhs );
  CameraState& operator= ( const CameraState& rhs );
  
  double _longitude;
  double _latitude;
  double _altitude;
  double _heading;
  double _tilt;
  double _roll;
  Matrix _viewMatrix;
};


}
}
}


#endif // __MINERVA_CORE_DATA_CAMERA_STATE_H__
