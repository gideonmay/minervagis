
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
//  Class that represents a menu.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_MENU_H_
#define _HELIOS_MENUS_MENU_H_

#include "Helios/Menus/Item.h"

#include <vector>

namespace Helios { namespace Menus { class MenuBar; } }


namespace Helios {
namespace Menus {


class HELIOS_EXPORT Menu : public Helios::Menus::Item
{
public:

  typedef Helios::Menus::Item BaseClass;
  typedef std::vector < Item::RefPtr > Items;
  typedef Items::size_type Position;

  USUL_DECLARE_REF_POINTERS ( Menu );

  Menu ( const std::string &text = "" );

  virtual void      accept ( Visitor &v ) { v.visit ( *this ); }
  void              append ( Item::RefPtr item ) { this->insert ( -1, item ); }

  // Return item at position, or null if out of range.
  Item::RefPtr      at ( Position );

  void              clear();

  bool              empty() const;

  Position          find ( Item::RefPtr ) const;
  Position          find ( const std::string &text ) const;

  void              insert ( Position, Item::RefPtr );

  // Find or create menu. Can optionally specify position for when menu 
  // is created. Otherwise, it will be appended.
  RefPtr            menu ( const std::string &text, Position p = -1 );

  void              prepend ( Item::RefPtr item ) { this->insert ( 0, item ); }

  // Remove empty sub-menus.
  void              purge();

  unsigned int      size() const;

  void              remove ( Position );
  void              remove ( Item::RefPtr item )       { this->remove ( this->find ( item ) ); }
  void              remove ( const std::string &text ) { this->remove ( this->find ( text ) ); }

  void              visitItems ( Visitor & );

protected:

  virtual ~Menu();

private:

  friend class MenuBar;

  Items _items;
};


} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_MENU_H_
