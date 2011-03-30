
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_CUSTOM_GEOMETRY_H_
#define _MINERVA_CUSTOM_GEOMETRY_H_

#include "MinervaQt/Plugins/QtCustomGeometry/CompileGuard.h"

#include "Minerva/Common/ILayerAddGUIQt.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Interfaces/IPlugin.h"

class AddGeometryWidget;

class QtCustomGeometryComponent : public Usul::Base::Referenced,
                                  public Usul::Interfaces::IPlugin,
                                  public Minerva::Common::ILayerAddGUIQt

{
public:

  /// Typedefs.
  typedef Usul::Base::Referenced                                   BaseClass;
  typedef Usul::Interfaces::IUnknown                               Unknown;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( QtCustomGeometryComponent );

  /// Usul::Interfaces::IUnknown members.
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  /// Constructor
  QtCustomGeometryComponent();
  
protected:

  virtual QWidget*            layerAddGUI();
  virtual std::string         name () const;
  virtual void                apply ( Minerva::Core::Data::Feature* parent, DataLoadedCallback dataLoadedCallback );

  /// Return name of plugin.
  virtual std::string           getPluginName() const;

  // Do not copy.
  QtCustomGeometryComponent ( const QtCustomGeometryComponent & );
  QtCustomGeometryComponent &operator = ( const QtCustomGeometryComponent & );

  /// Use reference counting.
  virtual ~QtCustomGeometryComponent();

private:
  AddGeometryWidget *_widget;
};


#endif // _OSSIM_LAYER_QT_H_
