
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

#ifndef __ADD_FILE_SYSTEM_COMPONENT_H_
#define __ADD_FILE_SYSTEM_COMPONENT_H_

#include "Minerva/Qt/Widgets/AddFileSystemWidget.h"

#include "Minerva/Common/ILayerAddGUIQt.h"

#include "Usul/Base/Referenced.h"

class AddFileSystemComponent : public Usul::Base::Referenced,
                               public Minerva::Common::ILayerAddGUIQt

{
public:

  /// Typedefs.
  typedef Usul::Base::Referenced                                   BaseClass;
  typedef Usul::Interfaces::IUnknown                               Unknown;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( AddFileSystemComponent );

  /// Usul::Interfaces::IUnknown members.
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  /// Constructor
  AddFileSystemComponent();
  
protected:

  virtual QWidget*            layerAddGUI();
  virtual std::string         name () const;
  virtual void                apply ( Minerva::Core::Data::Feature* parent, DataLoadedCallback dataLoadedCallback );

  // Do not copy.
  AddFileSystemComponent ( const AddFileSystemComponent & );
  AddFileSystemComponent &operator = ( const AddFileSystemComponent & );

  /// Use reference counting.
  virtual ~AddFileSystemComponent();

private:
  Minerva::QtWidgets::AddFileSystemWidget *_widget;
};


#endif // __ADD_FILE_SYSTEM_COMPONENT_H_
