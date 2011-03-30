
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_INTERFACES_ILAYER_ADD_GUI_QT_H__
#define __MINERVA_INTERFACES_ILAYER_ADD_GUI_QT_H__

#include "Usul/Interfaces/IUnknown.h"

namespace Minerva { namespace Core { namespace Data { class Feature; } } }
class QWidget;

#include "boost/function.hpp"

#include <string>

namespace Minerva {
namespace Common {


struct ILayerAddGUIQt : public Usul::Interfaces::IUnknown
{
  typedef boost::function<void ()> DataLoadedCallback;

  /// Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( ILayerAddGUIQt );

  /// Id for this interface.
  enum { IID = 2946322505u };

  virtual QWidget*            layerAddGUI() = 0;

  virtual std::string         name() const = 0;

  virtual void                apply ( Minerva::Core::Data::Feature *feature, DataLoadedCallback dataLoadedCallback ) = 0;

}; // struct ILayerAddGUIQt


} // End namespace Interfaces
} // End namespace Usul

#endif // __USUL_INTERFACES_ILAYER_ADD_GUI_QT_H__
