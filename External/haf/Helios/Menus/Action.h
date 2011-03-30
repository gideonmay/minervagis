
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
//  Action class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_ACTION_H_
#define _HELIOS_MENUS_ACTION_H_

#include "Helios/Export.h"


#include "QtGui/QAction"

#include "boost/function.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"

#include <string>


namespace Helios {
namespace Menus {


class HELIOS_EXPORT Action : public QAction, public boost::noncopyable
{
  Q_OBJECT;

public:

  // Typedefs.
  typedef QAction BaseClass;
  typedef boost::function<void ()>  Command;
  typedef boost::function<void ( QAction& )> Update;
  typedef boost::shared_ptr < Action > RefPtr;

  // Constructor.
  Action ( const std::string &text, Command command, QObject *parent = 0x0 );
  Action ( const std::string &text, 
           const std::string &icon, 
           const std::string &status,
           Command command, 
           Update update,
           QObject *parent );

  // Destructor.
  virtual ~Action();

  // Clear the action of internal commands.
  void              clear();

  // Update the action now.
  void              updateState();

private slots:

  void              _triggered();

  void              _updateNotify();

private:

  Command _command;
  Update _update;
};



} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_ACTION_H_
