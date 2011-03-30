
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
//  Class that represents a menubar.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_MENUBAR_H_
#define _HELIOS_MENUS_MENUBAR_H_

#include "Helios/Menus/Menu.h"

#include <vector>


namespace Helios {
namespace Menus {


class HELIOS_EXPORT MenuBar : public Helios::Menus::Item
{
public:

  typedef Helios::Menus::Item BaseClass;
  typedef std::vector < Menu::RefPtr > MenuList;
  typedef MenuList::size_type Position;

  USUL_DECLARE_REF_POINTERS ( MenuBar );

  MenuBar();

  virtual void      accept ( Visitor &v ) { v.visit ( *this ); }
  void              append ( Menu::RefPtr menu ) { this->insert ( -1, menu ); }

  // Return menu at position, or null if out of range.
  Menu::RefPtr      at ( Position );

  void              dirty ( bool state );

  bool              empty() const;

  Position          find ( Menu::RefPtr ) const;
  Position          find ( const std::string &text ) const;

  bool              isDirty() const;
  void              insert ( Position, Menu::RefPtr );

  // Find or create menu. Can optionally specify position for when menu 
  // is created. Otherwise, it will be appended.
  Menu::RefPtr      menu ( const std::string &text, Position p = -1 );

  void              prepend ( Menu::RefPtr menu ) { this->insert ( 0, menu ); }

  // Remove empty menus.
  void              purge();

  void              remove ( Position );
  void              remove ( Menu::RefPtr menu )       { this->remove ( this->find ( menu ) ); }
  void              remove ( const std::string &text ) { this->remove ( this->find ( text ) ); }

  void              visitItems ( Visitor & );

protected:

  virtual ~MenuBar();

private:

  MenuList _menus;
  bool _dirty;
};


} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_MENUBAR_H_
