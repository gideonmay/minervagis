
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Optimized class for drawing multiple points at once.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_MULTI_POINT_H__
#define __MINERVA_CORE_DATA_MULTI_POINT_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Common/Coordinates.h"
#include "Minerva/Core/Data/Geometry.h"

#include "Usul/Math/Vector3.h"

#include <vector>

namespace Minerva {
namespace Core {
namespace Data {
      
      
class MINERVA_EXPORT MultiPoint : public Geometry
{
public:
  typedef Geometry BaseClass;
  typedef Usul::Math::Vec4f Color;
  typedef Minerva::Common::Coordinates Coordinates;

  USUL_DECLARE_QUERY_POINTERS ( MultiPoint );

  MultiPoint();

  /// Get/Set the coordinates.
  void                    coordinates ( Coordinates::RefPtr );
  Coordinates::RefPtr     coordinates() const;
  
protected:
  
  /// Use reference counting.
  virtual ~MultiPoint();
  
  /// Build the scene branch.
  virtual osg::Node*    _buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation );
  
private:

  Coordinates::RefPtr _coordinates;
};

}
}
}


#endif // __MINERVA_CORE_DATA_MULTI_POINT_H__
