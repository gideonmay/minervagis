
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Derived from earlier work by Adam Kubach and Perry Miller found here:
//  http://sourceforge.net/projects/cadkit/
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class that represents a wrapper around QMenu.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __HELIOS_MENUS_MENU_ADAPTER_H__
#define __HELIOS_MENUS_MENU_ADAPTER_H__

#include "Helios/Menus/Action.h"
#include "Helios/Menus/Menu.h"

#include "QtGui/QMenu"

#include <set>

namespace Helios {
namespace Menus {
    

class HELIOS_EXPORT MenuAdapter : public QMenu
{
  Q_OBJECT;
public:
  
  typedef QMenu BaseClass;
  typedef boost::shared_ptr<MenuAdapter> RefPtr;
  typedef std::set < Action::RefPtr > Actions;
  typedef boost::shared_ptr < QMenu > QMenuPtr;
  typedef std::set < QMenuPtr > Menus;
  
  MenuAdapter ( const QString& title= "", QWidget* parent = 0x0 );
  virtual ~MenuAdapter();
  
  void         menu ( Menu::RefPtr menu );
  Menu::RefPtr menu() const;
  
private slots:
  
  void _onShowMenu();
  
private:
  
  Menu::RefPtr _menu;
  Menus _menus;
  Actions _actions;
};


}
}

#endif // __HELIOS_MENUS_MENU_ADAPTER_H__
