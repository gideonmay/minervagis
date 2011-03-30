
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2005, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_INTERFACE_BUILD_SCENE_H_
#define _MINERVA_INTERFACE_BUILD_SCENE_H_

#include "Usul/Interfaces/IUnknown.h"

namespace osg { class Node; }

namespace Minerva { namespace Common { struct IPlanetCoordinates; struct IElevationDatabase; } }

namespace Minerva {
namespace Common {

struct IBuildScene : public Usul::Interfaces::IUnknown
{
  /// Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( IBuildScene );

  /// Id for this interface.
  enum { IID = 3373393853u };

  virtual osg::Node * buildScene ( IPlanetCoordinates *planet, IElevationDatabase *elevation ) = 0;

}; // class IBuildScene


} // namespace Interfaces
} // namespace Usul


#endif // _MINERVA_INTERFACE_BUILD_SCENE_H_
