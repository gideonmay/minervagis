
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_VISITORS_FIND_RASTER_LAYERS_H__
#define __MINERVA_CORE_VISITORS_FIND_RASTER_LAYERS_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Visitor.h"
#include "Minerva/Core/Layers/RasterLayer.h"

#include "Minerva/Common/Extents.h"

#include "Usul/Math/MinMax.h"

#include "osg/Vec2d"

#include <vector>

namespace Minerva {
namespace Core {
namespace Visitors {


class MINERVA_EXPORT FindRasterLayers : public Minerva::Core::Visitor
{
public:
  
  typedef Minerva::Core::Visitor BaseClass;
  typedef std::vector<Minerva::Core::Layers::RasterLayer::RefPtr> RasterLayers;
  typedef Minerva::Common::Extents Extents;
  
  USUL_DECLARE_REF_POINTERS ( FindRasterLayers );
  
  FindRasterLayers ( const Extents& extents, RasterLayers& container );
  
  virtual void visit ( Minerva::Core::Layers::RasterLayer &raster );
  
protected:
  
  virtual ~FindRasterLayers();
  
private:
  
  Extents _extents;
  RasterLayers& _container;
  
};

  
}
}
}

#endif // __MINERVA_CORE_VISITORS_FIND_RASTER_LAYERS_H__
