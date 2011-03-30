
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

#ifndef _POSTGIS_LAYER_QT_H_
#define _POSTGIS_LAYER_QT_H_

#include "MinervaQt/Plugins/PostGISLayerQt/CompileGuard.h"
#include "MinervaQt/Plugins/PostGISLayerQt/AddPostGISLayerWidget.h"

#include "Minerva/Interfaces/ILayerAddGUIQt.h"
#include "Minerva/Interfaces/ILayerModifyGUIQt.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Headers/Qt.h"
#include "Usul/Interfaces/IPlugin.h"

class PostGISLayerQtComponent : public Usul::Base::Referenced,
                                public Usul::Interfaces::IPlugin,
                                public Minerva::Interfaces::ILayerAddGUIQt,
                                public Minerva::Interfaces::ILayerModifyGUIQt

{
public:

  /// Typedefs.
  typedef Usul::Base::Referenced     BaseClass;
  typedef Usul::Interfaces::IUnknown Unknown;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( PostGISLayerQtComponent );

  /// Usul::Interfaces::IUnknown members.
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  /// Constructor
  PostGISLayerQtComponent();
  
protected:

  /// ILayerAddGUIQt.
  virtual QWidget*            layerAddGUI();
  virtual std::string         name () const;
  virtual void                apply ( Minerva::Core::Data::Feature*, DataLoadedCallback );

  /// Return name of plugin.
  virtual std::string         getPluginName() const;

  /// ILayerModifyQtGUI
  virtual bool                handle ( Minerva::Core::Data::Feature* ) const;
  virtual void                showModifyGUI ( Minerva::Core::Data::Feature* feature, Minerva::Core::Data::Feature* parent, Usul::Interfaces::IUnknown* caller = 0x0 );

  // Do not copy.
  PostGISLayerQtComponent ( const PostGISLayerQtComponent & );
  PostGISLayerQtComponent &operator = ( const PostGISLayerQtComponent & );

  /// Use reference counting.
  virtual ~PostGISLayerQtComponent();

private:

  AddPostGISLayerWidget *_widget;
};


#endif // _POSTGIS_LAYER_QT_H_
