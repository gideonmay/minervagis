
//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//  Planetary navigation.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_NAVIGATOR_H__
#define __MINERVA_CORE_NAVIGATOR_H__

#include "Minerva/Core/Data/Camera.h"
#include "Minerva/Core/TileEngine/Body.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"

#include "osg/Matrixd"

namespace Minerva {
namespace Core {


class MINERVA_EXPORT Navigator : public Usul::Base::Referenced
{
public:
  typedef Usul::Base::Referenced BaseClass;
  typedef Minerva::Core::TileEngine::Body Body;
  typedef Minerva::Core::Data::Camera Camera;
  typedef Usul::Math::Vec2d MousePosition;

  USUL_DECLARE_REF_POINTERS ( Navigator );

  Navigator ( Body::RefPtr body );

  /// Set the body.
  void                  body ( Body::RefPtr body );

  /// Get a copy of the camera.
  Camera*               copyCameraState() const;

  /// Set/get the view matrix.
  void                  viewMatrix ( const osg::Matrixd& m );
  osg::Matrixd          viewMatrix() const;

  /// Set data members to the home position.
  void                  home();

  // Rotate the world.
  void                  rotate    ( double dx, double dy );
  void                  rotate    ( const MousePosition& point0, const MousePosition& point1, const osg::Matrixd& projection );

  // Change the elevation.
  void                  elevation ( double delta );
  void                  elevation ( const MousePosition& point0, const MousePosition& point1 );

  /// Zoom along the line of sight.
  void                  zoomLOS   ( const MousePosition& point0, const MousePosition& point1, const osg::Matrixd& projection );

  /// Zoom along given direction.
  void                  zoomAlongDirection ( const Usul::Math::Vec3d& direction, double delta );

  // Look around.
  void                  look      ( double dx, double dy );
  void                  look      ( const MousePosition& point0, const MousePosition& point1 );

  /// Get/Set elevation speed.
  void                  elevationSpeed ( double speed ) { _elevationSpeed = speed; }
  double                elevationSpeed () const         { return _elevationSpeed; }
  
  /// Get/Set the pitch speed.
  void                  pitchSpeed ( double speed ) { _pitchSpeed = speed; }
  double                pitchSpeed () const         { return _pitchSpeed; }

  /// Get/Set the yaw speed.
  void                  yawSpeed ( double speed ) { _yawSpeed = speed; }
  double                yawSpeed () const         { return _yawSpeed; }

protected:

  virtual ~Navigator();

  // Get the elevation given the current zoom.
  double                  _elevation() const;

  Usul::Math::Vec3d       _convertMouseToWorldSpace ( const Usul::Math::Vec3d& mouse, const osg::Matrixd& projection ) const;

  bool                    _intersectionPoint ( const MousePosition& mouse, const osg::Matrixd& projection, Usul::Math::Vec3d& point );
  bool                    _intersectionPoint ( const Usul::Math::Vec3d& pt0, const Usul::Math::Vec3d& pt1, Usul::Math::Vec3d& point );

  /// Calculate where the line of sight intersects the planet.
  bool                    _losIntersectionPoint ( Usul::Math::Vec3d& point );

  /// Calculate the speed given current parameters.
  double                  _calculateLinearSpeed ( double altitude ) const;

private:

  Minerva::Core::Data::Camera::RefPtr _camera;

  double _elevationSpeed;
  double _pitchSpeed;
  double _yawSpeed;

  Body::RefPtr _body;
};


}
}

#endif // __MINERVA_CORE__NAVIGATOR_H__
