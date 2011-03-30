
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
//  Visitor for building the menus.
//
///////////////////////////////////////////////////////////////////////////////

#include "Helios/Menus/Builder.h"
#include "Helios/Menus/Action.h"
#include "Helios/Menus/MenuBar.h"
#include "Helios/Menus/Separator.h"
#include "Helios/Menus/ToolBar.h"

#include "Usul/Functions/SafeCall.h"
#include "Usul/Scope/Reset.h"

#include "QtGui/QMenuBar"
#include "QtGui/QMenu"
#include "QtGui/QToolBar"

using namespace Helios::Menus;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
///////////////////////////////////////////////////////////////////////////////

Builder::Builder ( QMenuBar *mb ) : BaseClass(),
  _menuBar ( mb ),
  _toolBar ( 0x0 ),
  _currentMenu ( 0x0 ),
  _actions(),
  _menus()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
///////////////////////////////////////////////////////////////////////////////

Builder::Builder ( QToolBar *tb ) : BaseClass(),
  _menuBar ( 0x0 ),
  _toolBar ( tb ),
  _currentMenu ( 0x0 ),
  _actions(),
  _menus()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor
//
///////////////////////////////////////////////////////////////////////////////

Builder::~Builder()
{
  USUL_TRY_BLOCK
  {
    this->_reset();
  }
  USUL_DEFINE_CATCH_BLOCKS ( "4068778617" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Reset the internal state.
//
///////////////////////////////////////////////////////////////////////////////

void Builder::reset()
{
  this->_reset();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Reset the internal state.
//
///////////////////////////////////////////////////////////////////////////////

void Builder::_reset()
{
  // Initialize the current menu.
  _currentMenu = 0x0;

  // Delete the actions and menus, in that order.
  _actions.clear();
  _menus.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Visit the item.
//
///////////////////////////////////////////////////////////////////////////////

void Builder::visit ( Button &b )
{
  // Make the action with a null parent.
  Action::RefPtr action ( new Helios::Menus::Action (
    b.textGet(), b.iconGet(), b.statusGet(), 
    b.commandGet(), b.updateGet(), 0x0 ) );

  // Save the action.
  _actions.insert ( action );

  // If we are adding to a menu...
  if ( 0x0 != _currentMenu )
  {
    // Add the action to the menu.
    _currentMenu->addAction ( action.get() );

    // Update button when menu is shown.
    QObject::connect ( _currentMenu, SIGNAL ( aboutToShow() ), 
                       action.get(), SLOT ( _updateNotify() ) );
  }

  // If we are adding to a toolbar...
  if ( 0x0 != _toolBar )
  {
    _toolBar->addAction ( action.get() );
  }

  // Redirect to base class.
  BaseClass::visit ( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Visit the item.
//
///////////////////////////////////////////////////////////////////////////////

void Builder::visit ( Menu &m )
{
  typedef Usul::Scope::Reset < QMenu * > ResetMenu;

  const std::string s ( m.text() );
  QString text ( ( true == s.empty() ) ? QString() : QString ( s.c_str() ) );

  // Make the new menu with null parent.
  QMenuPtr menu ( new QMenu ( text, 0x0 ) );

  // Save the menu.
  _menus.insert ( menu );

  // Need local scope.
  {
    // Grab current menu and set to new one.
    QMenu *current ( _currentMenu );
    ResetMenu rm ( _currentMenu, menu.get(), current );

    // If there is a current menu...
    if ( 0x0 != current )
    {
      current->addMenu ( menu.get() );
    }

    // Otherwise, add new menu to menubar.
    else
    {
      if ( 0x0 != _menuBar )
      {
        _menuBar->addMenu ( menu.get() );
      }
    }

    // Visit items in this scope because we set a new "current" menu.
    BaseClass::visit ( m );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Visit the item.
//
///////////////////////////////////////////////////////////////////////////////

void Builder::visit ( Separator &s )
{
  if ( 0x0 != _currentMenu )
  {
    _currentMenu->addSeparator();
  }

  BaseClass::visit ( s );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Visit the item.
//
///////////////////////////////////////////////////////////////////////////////

void Builder::visit ( MenuBar &mb )
{
  BaseClass::visit ( mb );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Visit the item.
//
///////////////////////////////////////////////////////////////////////////////

void Builder::visit ( ToolBar &tb )
{
  BaseClass::visit ( tb );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the actions.
//
///////////////////////////////////////////////////////////////////////////////

void Builder::updateActions()
{
  for ( Actions::iterator i = _actions.begin(); i != _actions.end(); ++i )
  {
    Action::RefPtr action ( *i );
    if ( 0x0 != action.get() )
    {
      action->updateState();
    }
  }
}
