
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Password prompt.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PASSWORD_PROMPT_COMPONENT_CLASS_H_
#define _PASSWORD_PROMPT_COMPONENT_CLASS_H_

#include "Usul/Threads/Variable.h"

#include "QtCore/QObject"
#include "QtGui/QWidget"

#include <string>


class QPasswordPromptWidget : public QObject
{
	Q_OBJECT;
  
public:

  /// Typedefs.
  typedef QObject BaseClass;
	typedef Usul::Threads::Variable<bool> Boolean;
	typedef Usul::Threads::Variable<std::string> String;

  /// Default construction.
  QPasswordPromptWidget();
  virtual ~QPasswordPromptWidget();
  
	/// Prompt the user for a password.
  std::string promptForPassword ( const std::string& text );

protected: 

  // Do not copy.
  QPasswordPromptWidget ( const QPasswordPromptWidget & );
  QPasswordPromptWidget &operator = ( const QPasswordPromptWidget & );

private slots:
	void _promptForPassword ( QString text );

private:
  
	Boolean _done;
	String  _text;
};


#endif // _PASSWORD_PROMPT_COMPONENT_CLASS_H_
