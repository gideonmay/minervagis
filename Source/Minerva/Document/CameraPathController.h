
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class to manage the active path.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_DOCUMENT_CAMERA_PATH_CONTROLLER_H__
#define __MINERVA_DOCUMENT_CAMERA_PATH_CONTROLLER_H__

#include "Minerva/Document/Export.h"
#include "Minerva/Document/CameraPath.h"

#include "Minerva/Core/Data/CameraState.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"

#include "osg/Matrix"

namespace Minerva {
namespace Document {

class MINERVA_DOCUMENT_EXPORT CameraPathController : public Usul::Base::Referenced
{
public:

  typedef Usul::Base::Referenced BaseClass;
  typedef Minerva::Core::Data::CameraState CameraState;

  USUL_DECLARE_REF_POINTERS ( CameraPathController );

  CameraPathController();

  // Set/get the current path.
  void               currentPath ( CameraPath::RefPtr );
  CameraPath::RefPtr currentPath() const;

  void currentCameraAppend ( CameraState::RefPtr camera );
  void currentCameraInsert ( CameraState::RefPtr camera );
  void currentCameraPrepend ( CameraState::RefPtr camera );
  void currentCameraRemove ( CameraState::RefPtr camera );
  void currentCameraReplace ( CameraState::RefPtr camera );
  
  void closePath();

protected:

  virtual ~CameraPathController();

private:

  CameraPath::RefPtr _currentPath;
};

}
}

#endif // __MINERVA_DOCUMENT_CAMERA_PATH_CONTROLLER_H__
