
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Perry L Miller IV
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Main window.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _CADKIT_HELIOS_QT_CORE_MAIN_WINDOW_H_
#define _CADKIT_HELIOS_QT_CORE_MAIN_WINDOW_H_

#include "SplashScreen.h"
#include "TimerFactory.h"

#include "Usul/Interfaces/Qt/IMainWindow.h"
#include "Usul/Interfaces/IPasswordPrompt.h"
#include "Usul/Threads/Guard.h"
#include "Usul/Threads/RecursiveMutex.h"
#include "Usul/Threads/Queue.h"

#include "Helios/Menus/Menu.h"
#include "Helios/Menus/Action.h"

#include "boost/shared_ptr.hpp"

#include <QtGui/QMainWindow>

#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

class QWorkspace;
class QTextEdit;
class QMenu;
class QStringList;
class QTimer;

namespace Usul { namespace Threads { class Thread; } }
namespace Minerva { namespace Document { class MinervaDocument; } }


class MainWindowBase : 
  public QMainWindow,
  public Usul::Interfaces::Qt::IMainWindow,
  public Usul::Interfaces::IPasswordPrompt
{
  Q_OBJECT

public:

  // Useful typedefs.
  typedef QMainWindow                           BaseClass;
  typedef Usul::Threads::RecursiveMutex         Mutex;
  typedef Usul::Threads::Guard<Mutex>           Guard;
  typedef Usul::Interfaces::IUnknown            Unknown;

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( MainWindowBase );

  // Usul::Interfaces::IUnknown members.
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  // Constructor and destructor.
  MainWindowBase ( const std::string &vendor, 
               const std::string &url, 
               const std::string &program, 
               const std::string &icon,
               const std::string &output,
               const std::string &about,
               bool showSplash = true );
  virtual ~MainWindowBase();

  // Get the mutex.
  Mutex &                           mutex() const { return *_mutex; }

  // Functions for loading, releasing, etc., the plugins.
  std::string                       pluginFile() const;
  void                              loadPlugins ( const std::string &configFile );
  void                              printPlugins() const;
  void                              releasePlugins();

  // Load the file.
  void                              loadFile ( const std::string &file );

  // Functions for getting information about this binary.
  std::string                       directory() const;
  std::string                       icon() const;
  std::string                       programFile() const;
  std::string                       programName() const;
  std::string                       vendor() const;
  std::string                       about() const;

  // Parse the command-line.
  void                              parseCommandLine ( int argc, char **argv );

  // Show/hide the splash screen.
  void                              hideSplashScreen();
  void                              showSplashScreen();

  // Get the name of the settings file.
  std::string                       settingsFileName() const;

  // Restore dock window positions.
  void                              restoreDockWindows();

  // Text window operations.
  virtual void                      updateTextWindow ( bool force );

public slots:
  
  void notifyDocumentFinishedLoading ( void* document );
  
protected:

  void _newDocument();
  void _openDocument();
  void _openDocument ( const std::string& filename );

  void                              _initRecentFilesMenu();
  
  void                              _clearDocuments();
  void                              _clearRecentFiles();
  virtual void                      closeEvent ( QCloseEvent *event );
  virtual void                      _closeEvent ( QCloseEvent* event );

  /// Drag events.
  virtual void                      dragEnterEvent ( QDragEnterEvent *event );
  virtual void                      dropEvent      ( QDropEvent      *event );

  void                              _loadSettings();

  void                              _parseCommandLine ( int argc, char **argv );

  void                              _saveSettings();
  
  static void                       _waitForJobs();
 
  // Usul::Interfaces::Qt::IMainWindow
  virtual QMainWindow *             mainWindow();
  virtual const QMainWindow*        mainWindow() const;

  /// Prompt the user for a password.
  virtual std::string               promptForPassword ( const std::string& text );
  
  virtual void                      _notifyFinishedLoading ( Minerva::Document::MinervaDocument* );
  
private slots:

  void                              _idleProcess();

  void                              _notifyDocumentFinishedLoading ( void* document );

private:

  typedef std::list<std::string> StringList;

  // No copying or assignment.
  MainWindowBase ( const MainWindowBase & );
  MainWindowBase &operator = ( const MainWindowBase & );
  
  void                              _destroy();

  mutable Mutex *_mutex;
  unsigned long _refCount;
  std::string _vendor;
  std::string _program;
  std::string _icon;
  std::string _output;
  std::string _about;
  SplashScreen::RefPtr _splash;
  QTimer *_idleTimer;
  StringList _recentFiles;
  Helios::Menus::Menu::RefPtr _recentFilesMenu;
  TimerServer::RefPtr _timerServer;
};


#endif //_CADKIT_HELIOS_QT_CORE_MAIN_WINDOW_H_
