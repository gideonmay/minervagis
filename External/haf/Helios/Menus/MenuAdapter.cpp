
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

#include "Helios/Menus/MenuAdapter.h"
#include "Helios/Menus/Button.h"
#include "Helios/Menus/Visitor.h"


using namespace Helios::Menus;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

MenuAdapter::MenuAdapter ( const QString& title, QWidget* parent ) : BaseClass ( title, parent ),
  _menu(),
  _menus(),
  _actions()
{
  QObject::connect ( this, SIGNAL ( aboutToShow() ), SLOT ( _onShowMenu() ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

MenuAdapter::~MenuAdapter()
{
  _menu = 0x0;
  _menus.clear();
  _actions.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the menu.
//
///////////////////////////////////////////////////////////////////////////////

void MenuAdapter::menu ( Menu::RefPtr menu )
{
  _menu = menu;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the menu.
//
///////////////////////////////////////////////////////////////////////////////

Menu::RefPtr MenuAdapter::menu() const
{
  return _menu;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the menu.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail 
{
  
  class QtMenuBuilder : public Visitor
  {
  public:
    typedef Visitor BaseClass;
    USUL_DECLARE_REF_POINTERS ( QtMenuBuilder );
    
    QtMenuBuilder ( QMenu* menu, MenuAdapter::Actions &actions, MenuAdapter::Menus& menus ) : BaseClass(),
      _menu ( menu ),
      _actions ( actions ),
      _menus ( menus )
    {
    }
    
    virtual void visit ( Menu &m )
    {
      // Create the menu.
      MenuAdapter::RefPtr menu ( new MenuAdapter ( m.text().c_str() ) );
      _menus.insert ( menu );
      
      // Set the Menu.
      menu->menu ( &m ); 
      
      _menu->addMenu ( menu.get() );
    }
    
    virtual void visit ( Button &b )
    {
      Action::RefPtr action ( new Helios::Menus::Action (
        b.textGet(), b.iconGet(), b.statusGet(), 
        b.commandGet(), b.updateGet(), 0x0 ) );
      
      action->updateState();

      _actions.insert ( action );
      
      _menu->addAction ( action.get() );
    }
    
    virtual void apply ( Separator &s )
    {
      _menu->addSeparator();
    }
    
  private:
    QMenu *_menu;
    MenuAdapter::Actions &_actions;
    MenuAdapter::Menus &_menus;
  };
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the menu.
//
///////////////////////////////////////////////////////////////////////////////

void MenuAdapter::_onShowMenu()
{
  // Clear what we may have.
  this->clear();
  _actions.clear();
  _menus.clear();
  
  if ( _menu.valid() )
  {
    // Build the qt menu.
    Detail::QtMenuBuilder::RefPtr visitor ( new Detail::QtMenuBuilder ( this, _actions, _menus ) );

    // Don't accept the visitor, but start by traversing the menu's items.
    _menu->visitItems ( *visitor );
  }
}
