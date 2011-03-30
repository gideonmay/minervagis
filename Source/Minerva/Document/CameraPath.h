
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2004, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  The camera path class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PATH_ANIMATION_CAMERA_PATH_H_
#define _PATH_ANIMATION_CAMERA_PATH_H_

#include "Minerva/Document/Export.h"

#include "Minerva/Core/Data/CameraState.h"

#include "Serialize/XML/Macros.h"

#include "Usul/Functions/SafeCall.h"
#include "Usul/Math/Vector3.h"

#include "osg/Matrixd"

#include <vector>

namespace Minerva {
namespace Document {

class MINERVA_DOCUMENT_EXPORT CameraPath : public Usul::Base::Object
{
public:

  // Smart pointers.
  USUL_DECLARE_QUERY_POINTERS ( CameraPath );

  // Typedefs.
  typedef Usul::Base::Object BaseClass;
  typedef Usul::Interfaces::IUnknown IUnknown;
  typedef Usul::Math::Vec3d Vec3d;
  typedef Usul::Math::Vector3 < Vec3d > Triplet;
  typedef std::vector < Triplet > Values;

  // Constructor.
  CameraPath();

  // Append the path.
  void                          append  ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up );

  /// Get the camera info.
  void                          camera ( Usul::Math::Vec3d &eye, Usul::Math::Vec3d &center, Usul::Math::Vec3d &up, unsigned int num ) const;

  // Can we close the path?
  bool                          canClose() const;

  // Can we play the animation?
  bool                          canPlay() const;

  // Clear.
  void                          clear();

  // Close the path (if possible).
  void                          closePath();

  // Insert the new camera between the closest camera and it's closest neighbor.
  void                          insertBetweenNearest  ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up );

  // Prepend the path.
  void                          prepend ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up );

  // Remove the camera closest to the given eye position.
  void                          removeNearest ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up );

  // Replace the nearest camera.
  void                          replaceNearest ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up );

  // Get number of cameras.
  unsigned int                  size() const;

  // Get the values.
  void                          values ( Values &, bool reverseOrder ) const;

  // Predicate functor for testing equal values.
  struct EqualValue
  {
    bool operator () ( const Triplet &a, const Triplet &b ) const
    {
      return ( a[0].equal ( b[0] ) && 
               a[1].equal ( b[1] ) && 
               a[2].equal ( b[2] ) );
    }
  };

protected:

  // Use reference counting.
  virtual ~CameraPath();

  unsigned int                  _closest ( const Usul::Math::Vec3d &eye ) const;

private:

  Values _values;

  SERIALIZE_XML_DEFINE_MAP;
  SERIALIZE_XML_DEFINE_MEMBERS ( CameraPath );
};

template < class F > void callCameraFunction ( F f, Minerva::Core::Data::CameraState::RefPtr camera )
{
  try
  {
    if ( camera )
    {
      osg::Matrixd matrix ( &camera->viewMatrix()[0] );
      osg::Matrixd m ( osg::Matrixd::inverse ( matrix ) );

      // Get the lookat data.
      osg::Vec3d e ( 0, 0, 0 );
      osg::Vec3d c ( 0, 0, 0 );
      osg::Vec3d u ( 0, 0, 0 );
      typedef CameraPath::Vec3d Vec3;
      m.getLookAt ( e, c, u );

      // Make sure up-vector is normalized.
      u.normalize();

      // Call the function.
      f ( Vec3 ( e[0], e[1], e[2] ), Vec3 ( c[0], c[1], c[2] ), Vec3 ( u[0], u[1], u[2] ) );
    }
  }
  USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS ( "3485778525" );
}

}
}

#endif // _PATH_ANIMATION_CAMERA_PATH_H_
