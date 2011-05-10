
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Base class for all geometries
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_GEOMETRY_H__
#define __MINERVA_CORE_DATA_GEOMETRY_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Data/AltitudeMode.h"
#include "Minerva/Core/Data/Object.h"
#include "Minerva/Core/Data/Style.h"

#include "Minerva/Common/Extents.h"
#include "Minerva/Common/IElevationData.h"

#include "boost/signals2/signal.hpp"

#include <vector>

namespace osg { class Node; }
namespace Minerva { namespace Common { struct IPlanetCoordinates; struct IElevationDatabase; } }

namespace Minerva {
namespace Core {
namespace Data {

class MINERVA_EXPORT Geometry : public Minerva::Core::Data::Object
{
public:
  typedef Minerva::Core::Data::Object               BaseClass;
  typedef Minerva::Common::Extents                  Extents;
  typedef Minerva::Common::IElevationDatabase       IElevationDatabase;
  typedef Minerva::Common::IPlanetCoordinates       IPlanetCoordinates;
  typedef Minerva::Common::IElevationData::QueryPtr ElevationDataPtr;

  USUL_DECLARE_REF_POINTERS( Geometry );
  
  /// Get/Set the altitude mode.
  void                  altitudeMode ( AltitudeMode mode );
  AltitudeMode          altitudeMode() const;

  /// Build the scene branch.
  osg::Node*            buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation );
  
  /// Elevation has changed within given extents.
  virtual void          elevationChangedNotify ( const Extents& extents, 
                                                 unsigned int level, 
                                                 ElevationDataPtr elevationData, 
                                                 IPlanetCoordinates *planet, 
                                                 IElevationDatabase* elevation );

  template<class F>
  void addElevationChangedListener ( F fun )
  {
    _elevationChangedListeners.connect ( fun );
  }

  template<class F>
  void removeElevationChangedListener ( F fun )
  {
    _elevationChangedListeners.disconnect ( fun );
  }

  /// Get/Set the dirty flag.
  void                  dirty ( bool b );
  bool                  dirty() const;
  
  /// Set/get the extents.
  void                  extents ( const Extents& e );
  Extents               extents() const;
  
  /// Get/Set extrude flag.
  void                  extrude ( bool b );
  bool                  extrude() const;
  
  /// Get/Set the render bin
  unsigned int          renderBin() const;
  void                  renderBin ( unsigned int );

  /// Is this geometry transparent?
  static bool           isSemiTransparent (  Style::RefPtr style );

protected:
	
  /// Construction/Destruction.
  Geometry();
  virtual ~Geometry();

  /// Build the scene branch.
  virtual osg::Node*    _buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation ) = 0;
    
private:

  typedef boost::signals2::signal<void ( const Extents&, unsigned int, ElevationDataPtr, IPlanetCoordinates*, IElevationDatabase* ) > ElevationChangedListeners;

  AltitudeMode _altitudeMode;
  bool         _dirty;
  bool         _extrude;
  unsigned int _renderBin;
  Extents      _extents;
  ElevationChangedListeners _elevationChangedListeners;
};

}
}
}

#endif // __MINERVA_CORE_GEOMETRY_H__
