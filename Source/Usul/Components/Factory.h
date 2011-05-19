
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2005, Perry L Miller IV and Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Generic Component Factory class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_COMPONENTS_FACTORY_CLASS_H_
#define _USUL_COMPONENTS_FACTORY_CLASS_H_

#include "Usul/Components/Manager.h"


#ifdef _DEBUG 
#define USUL_DEBUG_MODE true
#else
#define USUL_DEBUG_MODE false
#endif


///////////////////////////////////////////////////////////////////////////////
//
//  Macro for functions needed in Cadkit plugins.
//
///////////////////////////////////////////////////////////////////////////////

#define USUL_DECLARE_COMPONENT_FACTORY(class_name) \
extern "C" bool usul_is_debug_build() \
{ \
  return USUL_DEBUG_MODE;\
} \
extern "C" void usul_plugin_initialize() \
{ \
  Usul::Components::Manager::instance().addPlugin ( Usul::Interfaces::IUnknown::QueryPtr ( new class_name ) ); \
}


#endif // _USUL_COMPONENTS_FACTORY_CLASS_H_
