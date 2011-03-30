
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

#include "QPasswordPromptWidget.h"

#include "Usul/System/Sleep.h"
#include "Usul/Threads/Safe.h"

#include "QtGui/QInputDialog"
#include "QtCore/QMutex"
#include "QtCore/QMutexLocker"

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

QPasswordPromptWidget::QPasswordPromptWidget() : BaseClass(),
	_done ( false ),
	_text()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

QPasswordPromptWidget::~QPasswordPromptWidget()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Prompt the user for a password.
//
///////////////////////////////////////////////////////////////////////////////

std::string QPasswordPromptWidget::promptForPassword ( const std::string& text )
{
	// Only prompt for one thread at a time.
  QMutex mutex;
  QMutexLocker guard ( &mutex );

	Usul::Threads::Safe::set ( _text.mutex(), "", _text.value() );
	Usul::Threads::Safe::set ( _done.mutex(), false, _done.value() );

	QMetaObject::invokeMethod ( this, "_promptForPassword", Qt::QueuedConnection, Q_ARG ( QString, text.c_str() ) );

	while ( false == Usul::Threads::Safe::get ( _done.mutex(), _done.value() ) )
	{
		Usul::System::Sleep::milliseconds ( 500 );
	}

	return Usul::Threads::Safe::get ( _text.mutex(), _text.value() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Prompt the user for a password.
//
///////////////////////////////////////////////////////////////////////////////

void QPasswordPromptWidget::_promptForPassword ( QString text )
{
	QString result ( QInputDialog::getText ( 0x0, "Enter Password", text, QLineEdit::Password ) );

	Usul::Threads::Safe::set ( _text.mutex(), result.toStdString(), _text.value() );
	Usul::Threads::Safe::set ( _done.mutex(), true, _done.value() );
}
