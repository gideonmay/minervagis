
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
//  Multi-geometry class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_BASE_MULTI_GEOMETRY_H__
#define __MINERVA_BASE_MULTI_GEOMETRY_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Data/Geometry.h"

#include "Usul/Math/Vector3.h"

#include <vector>

namespace Minerva {
namespace Core {
namespace Data {

class MINERVA_EXPORT MultiGeometry : public Minerva::Core::Data::Geometry
{
public:
  typedef Minerva::Core::Data::Geometry BaseClass;
  typedef std::vector<Geometry::RefPtr> Geometries;

  USUL_DECLARE_REF_POINTERS ( MultiGeometry );

  MultiGeometry();

  /// Add a geometry.
  void                 addGeometry ( Geometry::RefPtr );

  /// Get the geometries.
  Geometries           geometries() const;

  /// Is this geometry transparent?
  virtual bool         isSemiTransparent() const;

  /// Reserve geometry size.
  void                 reserveGeometry ( unsigned int size );

protected:

  /// Use reference counting.
  virtual ~MultiGeometry();

  /// Build the scene branch.
  virtual osg::Node* _buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation );

private:
  
  Geometries _geometries;
};


}
}
}


#endif // __MINERVA_BASE_MULTI_GEOMETRY_H__
