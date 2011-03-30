
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_INTERFACES_ILAYER_MODIFY_GUI_QT_H__
#define __MINERVA_INTERFACES_ILAYER_MODIFY_GUI_QT_H__

#include "Usul/Interfaces/IUnknown.h"

namespace Minerva { namespace Core { namespace Data { class Feature; } } }

namespace Minerva {
namespace Common {

struct ILayerModifyGUIQt : public Usul::Interfaces::IUnknown
{
  /// Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( ILayerModifyGUIQt );

  /// Id for this interface.
  enum { IID = 1262894538u };

  virtual bool handle ( Minerva::Core::Data::Feature *feature ) const = 0;
  virtual void showModifyGUI ( Minerva::Core::Data::Feature *feature, Minerva::Core::Data::Feature* parent, Usul::Interfaces::IUnknown* caller = 0x0 ) = 0;

}; // struct ILayerModifyGUIQt


} // End namespace Interfaces
} // End namespace Usul

#endif // __MINERVA_INTERFACES_ILAYER_MODIFY_GUI_QT_H__
