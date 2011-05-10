
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  AltitudeMode values and functions to work with them.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_ALTITUDE_MODE_H__
#define __MINERVA_CORE_DATA_ALTITUDE_MODE_H__

#include "Minerva/Common/IElevationDatabase.h"

namespace Minerva {
namespace Core {
namespace Data {

  enum AltitudeMode
  {
    ALTITUDE_MODE_CLAMP_TO_GROUND,
    ALTITUDE_MODE_RELATIVE_TO_GROUND,
    ALTITUDE_MODE_ABSOLUTE
  };

  template<class Vertex>
  inline double getElevationAtPoint ( const Vertex& point, Minerva::Common::IElevationDatabase* elevation, AltitudeMode mode )
  {
    switch ( mode )
    {
      case ALTITUDE_MODE_CLAMP_TO_GROUND:
        return ( 0x0 != elevation ? elevation->elevationAtLatLong ( point[1], point[0] ) : 0.0 );
      case ALTITUDE_MODE_RELATIVE_TO_GROUND:
        return ( point[2] + ( 0x0 != elevation ? elevation->elevationAtLatLong ( point[1], point[0] ) : 0.0 ) );
      case ALTITUDE_MODE_ABSOLUTE:
        return point[2];
    }
    return 0.0;
  }

}
}
}

#endif // __MINERVA_CORE_DATA_ALTITUDE_MODE_H__
