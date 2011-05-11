
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_GEOMETRY_LINE_H__
#define __MINERVA_CORE_GEOMETRY_LINE_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Common/Coordinates.h"
#include "Minerva/Core/Data/Geometry.h"
#include "Minerva/Core/Data/LineStyle.h"

#include "Usul/Math/Vector3.h"

namespace osg { class StateSet; }

namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT Line : public Geometry
{
public:
  typedef Geometry                         BaseClass;
  typedef ColorStyle::Color                Color;
  typedef Minerva::Common::Coordinates     Coordinates;

  USUL_DECLARE_QUERY_POINTERS ( Line );

  Line();

  /// Get/Set the coordinates.
  void                  coordinates ( Coordinates::RefPtr );
  Coordinates::RefPtr   coordinates() const;

  /// Set/get tessellate flag.
  void                  tessellate ( bool );
  bool                  tessellate() const;

  /// Get/set flag to use a shader.
  bool                  useShader() const;
  void                  useShader ( bool b );
  
protected:

  virtual ~Line();

  virtual osg::Node*    _buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation );
  osg::Node*            _buildScene ( const Color& color, float width, IPlanetCoordinates *planet, IElevationDatabase* elevation );
  
private:
  
  // Set proper state.
  void                  _setState ( osg::StateSet*, const Color& color, float width ) const;

  Coordinates::RefPtr _coordinates;
  bool _tessellate;
  bool _useShader;
};

}
}
}


#endif // __MINERVA_CORE_GEOMETRY_LINE_H__
