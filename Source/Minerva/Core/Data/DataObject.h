
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class to contain vector data.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINEVA_CORE_DATA_OBJECT_H__
#define __MINEVA_CORE_DATA_OBJECT_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Data/Object.h"
#include "Minerva/Core/Data/Geometry.h"
#include "Minerva/Core/Data/Feature.h"

#include "Minerva/Common/Extents.h"
#include "Minerva/Common/IBuildScene.h"
#include "Minerva/Common/IElevationChangedListener.h"
#include "Minerva/Common/IWithinExtents.h"

#include "Usul/Pointers/Pointers.h"

#include "osg/Node"

#include <map>

namespace OsgTools { namespace Widgets { class Item; } }

namespace Minerva { namespace Common { struct IPlanetCoordinates; struct IElevationDatabase; } }

namespace Minerva {
namespace Core {

  class Visitor;

namespace Data {
  
class DataObject;


class MINERVA_EXPORT ClickedCallback : public Usul::Base::Object
{
public:
  typedef ::OsgTools::Widgets::Item Item;

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( ClickedCallback );
  
  ClickedCallback();
  virtual ~ClickedCallback();
  
  virtual Item* operator() ( const DataObject&, Usul::Interfaces::IUnknown* ) const = 0;
};


class MINERVA_EXPORT DataObject :
  public Minerva::Core::Data::Feature,
  public Minerva::Common::IElevationChangedListener,
  public Minerva::Common::IWithinExtents,
  public Minerva::Common::IBuildScene
{
public:
  typedef Minerva::Core::Data::Feature              BaseClass;
  typedef Usul::Interfaces::IUnknown                Unknown;
  typedef BaseClass::Extents                        Extents;
  typedef Minerva::Core::Data::Geometry             Geometry;
  typedef ::OsgTools::Widgets::Item                 Item;
  typedef ClickedCallback                           ClickedCB;
  typedef Usul::Math::Vec3d                         PositionType;
  typedef Minerva::Common::IWithinExtents           IWithinExtents;

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( DataObject );
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  /// Constructor.
  DataObject();

  /// Get this as a data object.
  virtual DataObject*   asDataObject() { return this; }

  /// Accept the visitor.
  virtual void          accept ( Minerva::Core::Visitor& visitor );

  /// DataObject has been clicked.
  virtual Item*         clicked ( Usul::Interfaces::IUnknown* caller = 0x0 ) const;

  /// Set/get the clicked callback.
  void                  clickedCallback ( ClickedCB::RefPtr );
  ClickedCB::RefPtr     clickedCallback() const;
  
  /// Clone this DataObject.
  virtual Feature*      clone() const { return new DataObject ( *this ); }

  /// Build the scene branch for the data object.
  void                  preBuildScene ( Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation );
  virtual osg::Node*    buildScene ( Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation );

  /// Get/Set the dirty flag.
  bool                  dirty() const;
  void                  dirty ( bool );
  
  /// Elevation has changed within given extents (IElevationChangedListnerer).
  bool                  elevationChangedNotify ( const Extents& extents, unsigned int level, ElevationDataPtr elevationData, Unknown * caller = 0x0 );

  /// Is the data object empty?
  bool                  empty() const;

  /// Set/get the geometry.
  void                  geometry ( Geometry::RefPtr );
  Geometry::RefPtr      geometry() const;

  /// Get/Set the label
  void                  label ( const std::string& label );
  const std::string&    label() const;

  /// Get/Set the flag to show the label.
  void                  showLabel ( bool value );
  bool                  showLabel() const;
  
  /// Set/get the style.
  void                  style ( Style::RefPtr style );
  Style::RefPtr         style() const;
  Style::RefPtr         getOrCreateStyle();

  // Set the visibilty.
  virtual void          visibilitySet ( bool b );

  /// Minerva::Common::IWithinExtents
  Feature::RefPtr       getItemsWithinExtents ( double minLon, double minLat, double maxLon, double maxLat, IUnknown::RefPtr caller = IUnknown::RefPtr ( 0x0 ) ) const;
  
protected:

  /// Use reference counting.
  virtual ~DataObject ();

  osg::Node*            _buildLabel ( const PositionType& position );
  
  // Return the pointer to this (IDataObject).
  virtual DataObject*   dataObject();

private:

  bool         _dirty;
  std::string  _label;
  bool         _showLabel;
  osg::ref_ptr < osg::Node > _root;
  osg::ref_ptr < osg::Node > _preBuiltScene;
  Geometry::RefPtr _geometry;
  ClickedCallback::RefPtr _clickedCallback;
  Style::RefPtr _style;
};

}
}
}

#endif // __MINEVA_CORE_DATA_OBJECT_H__
