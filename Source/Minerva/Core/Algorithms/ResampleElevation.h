
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_ALGORITHMS_RESAMPLE_ELEVATION_H__
#define __MINERVA_CORE_ALGORITHMS_RESAMPLE_ELEVATION_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/TileEngine/Tile.h"

#include "Minerva/Common/Extents.h"

namespace Minerva { namespace Common { struct IElevationData; } }

namespace Minerva {
namespace Core {
namespace Algorithms {

  typedef Minerva::Core::TileEngine::Tile Tile;
  typedef Minerva::Core::TileEngine::Extents Extents;

  MINERVA_EXPORT Minerva::Common::IElevationData* resampleElevation ( Tile::RefPtr tile, const Extents& request );

}
}
}

#endif // __MINERVA_CORE_ALGORITHMS_RESAMPLE_ELEVATION_H__
