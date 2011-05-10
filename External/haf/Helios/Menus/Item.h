
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Derived from earlier work by Adam Kubach and Perry Miller found here:
//  http://sourceforge.net/projects/cadkit/
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Base class for menu classes.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_ITEM_H_
#define _HELIOS_MENUS_ITEM_H_

#include "Helios/Export.h"
#include "Helios/Menus/Visitor.h"

#include "Usul/Base/Object.h"

#include "boost/noncopyable.hpp"


namespace Helios {
namespace Menus {


class HELIOS_EXPORT Item : 
  public Usul::Base::Object, 
  public boost::noncopyable
{
public:

  typedef Usul::Base::Object BaseClass;
  typedef Usul::Base::Referenced GuiData;

  USUL_DECLARE_REF_POINTERS ( Item );

  virtual void      accept ( Visitor &v ) { v.visit ( *this ); }

  std::string       text() const { return BaseClass::name(); }

protected:

  Item ( const std::string &text );
  virtual ~Item();
};


} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_ITEM_H_
