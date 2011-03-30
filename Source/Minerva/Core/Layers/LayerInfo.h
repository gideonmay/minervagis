
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_LAYER_INFO_H__
#define __MINERVA_CORE_LAYER_INFO_H__

#include "Minerva/Common/Extents.h"

#include "osg/Vec2d"

namespace Minerva {
namespace Core {
namespace Layers {

  
struct LayerInfo
{
  typedef Minerva::Common::Extents Extents;
  
  std::string name;
  std::string style;
  std::string title;
  Extents extents;
};

  
}
}
}

#endif // __MINERVA_CORE_LAYER_INFO_H__
