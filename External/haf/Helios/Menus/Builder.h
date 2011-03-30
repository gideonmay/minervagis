
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

#ifndef _HELIOS_MENUS_BUILDER_VISITOR_H_
#define _HELIOS_MENUS_BUILDER_VISITOR_H_

#include "Helios/Menus/Visitor.h"
#include "Helios/Menus/Action.h"

#include "boost/shared_ptr.hpp"

#include <set>

class QMenuBar;
class QMenu;
class QObject;
class QToolBar;


namespace Helios {
namespace Menus {


class HELIOS_EXPORT Builder : public Helios::Menus::Visitor
{
public:

  typedef Helios::Menus::Visitor BaseClass;
  typedef std::set < Action::RefPtr > Actions;
  typedef boost::shared_ptr < QMenu > QMenuPtr;
  typedef std::set < QMenuPtr > Menus;

  USUL_DECLARE_REF_POINTERS ( Builder );

  Builder ( QMenuBar * );
  Builder ( QToolBar * );

  virtual void        reset();

  void                updateActions();

  virtual void        visit ( Button & );
  virtual void        visit ( Menu & );
  virtual void        visit ( MenuBar & );
  virtual void        visit ( Separator & );
  virtual void        visit ( ToolBar & );

protected:

  virtual ~Builder();

  void                _reset();

private:

  QMenuBar *_menuBar;
  QToolBar *_toolBar;
  QMenu *_currentMenu;
  Actions _actions;
  Menus _menus;
};


} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_BUILDER_VISITOR_H_
