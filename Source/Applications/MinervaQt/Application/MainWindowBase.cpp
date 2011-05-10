
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

#include "MainWindowBase.h"
#include "OpenFileThread.h"
#include "QPasswordPromptWidget.h"

#include "Minerva/Document/MinervaDocument.h"

#include "Constants.h"

#include "XmlTree/Document.h"
#include "XmlTree/RegistryIO.h"

#include "Helios/Menus/Button.h"

#include "Minerva/Qt/Tools/FileDialog.h"
#include "Minerva/Qt/Tools/Image.h"

#include "Usul/App/Application.h"
#include "Usul/CommandLine/Arguments.h"
#include "Usul/Components/Manager.h"
#include "Usul/Components/Loader.h"
#include "Usul/Config/Config.h"
#include "Usul/Convert/Convert.h"
#include "Usul/Exceptions/Exception.h"
#include "Usul/Factory/ObjectFactory.h"
#include "Usul/File/Path.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Jobs/Manager.h"
#include "Usul/Registry/Convert.h"
#include "Usul/Registry/Database.h"
#include "Usul/Registry/Qt.h"
#include "Usul/Strings/Case.h"
#include "Usul/Strings/Format.h"
#include "Usul/Strings/Qt.h"
#include "Usul/System/Environment.h"
#include "Usul/Threads/Named.h"
#include "Usul/Threads/Safe.h"
#include "Usul/Threads/ThreadId.h"
#include "Usul/User/Directory.h"

#include "QtCore/QStringList"
#include "QtCore/QTimer"
#include "QtGui/QApplication"
#include "QtGui/QDockWidget"
#include "QtGui/QLabel"
#include "QtGui/QMenuBar"
#include "QtGui/QProgressDialog"
#include "QtGui/QPushButton"
#include "QtGui/QStatusBar"
#include "QtGui/QToolBar"
#include "QtGui/QVBoxLayout"
#include "QtGui/QWorkspace"
#include "QtGui/QDragEnterEvent"
#include "QtGui/QDropEvent"
#include "QtCore/QMetaType"
#include "QtCore/QUrl"

#include "boost/bind.hpp"
#include "boost/filesystem.hpp"
#include "boost/scoped_ptr.hpp"

#include <algorithm>
#include <fstream>

namespace Sections = CadKit::Helios::Core::Registry::Sections;
namespace Keys = CadKit::Helios::Core::Registry::Keys;
namespace Defaults = CadKit::Helios::Core::Registry::Defaults;
typedef Usul::Registry::Database Reg;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

MainWindowBase::MainWindowBase ( const std::string &vendor, 
                         const std::string &url, 
                         const std::string &program,
                         const std::string &icon,
                         const std::string &output,
                         const std::string &about,
                         bool showSplash ) : BaseClass(),
  _mutex        ( new MainWindowBase::Mutex ),
  _refCount     ( 0 ),
  _vendor       ( vendor ),
  _program      ( program ),
  _icon         ( icon ),
  _output       ( output ),
  _about        ( about ),
  _splash       ( 0x0 ),
  _idleTimer    ( 0x0 ),
  _recentFiles  (),
  _recentFilesMenu ( 0x0 ),
  _timerServer  ( new TimerServer )
{
  // Name this thread.
  Usul::Threads::Named::instance().set ( Usul::Threads::Names::GUI );

  // Keep names the same.
  Usul::App::Application::instance().program ( _program );

  // Program-wide settings.
  QCoreApplication::setOrganizationName ( vendor.c_str() );
  QCoreApplication::setOrganizationDomain ( url.c_str() );
  QCoreApplication::setApplicationName ( program.c_str() );

  // Show the splash screen if we should.
  if ( true == showSplash )
    this->showSplashScreen();

  // Set the icon.
  Minerva::QtTools::Image::icon ( icon, this );

  // Enable toolbar docking.
  this->setEnabled ( true );

  // Make sure we can size the status bar.
  this->statusBar()->setSizeGripEnabled ( true );
  this->statusBar()->showMessage ( tr ( "Ready" ) );

  // Load the settings.
  Usul::Functions::safeCall ( boost::bind ( &MainWindowBase::_loadSettings, this ), "9740089350" );

  // Set the title.
  this->setWindowTitle ( program.c_str() );

  // Start the idle timer.
  _idleTimer = new QTimer ( this );
  QObject::connect ( _idleTimer, SIGNAL ( timeout() ), SLOT ( _idleProcess() ) );
  _idleTimer->start ( 1000 ); // Once every second.

  this->setAcceptDrops ( true );

  // Add our self as a plugin.
  Usul::Interfaces::IUnknown::QueryPtr me ( this );
  Usul::Components::Manager::instance().addPlugin ( me );
  
  // Add the timer server.
  Usul::Components::Manager::instance().addPlugin ( Usul::Interfaces::IUnknown::QueryPtr ( _timerServer ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

MainWindowBase::~MainWindowBase()
{
  Usul::Functions::safeCall ( boost::bind ( &MainWindowBase::_clearDocuments, this ), "4010634300" );
  Usul::Functions::safeCall ( boost::bind ( &MainWindowBase::_saveSettings, this ),   "1772821423" );
  Usul::Functions::safeCall ( boost::bind ( &MainWindowBase::_destroy, this ),        "1934297230" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor contents.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_destroy()
{
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "2933090027" );

  // Stop the idle timer.
  if ( 0x0 != _idleTimer )
    _idleTimer->stop();

  // Stop all the timers.
  if ( true == _timerServer.valid() )
  {
    _timerServer = 0x0;
  }

  // Clear the factory before releasing the plugins (some of them may 
  // populate it but not clean up).
  Usul::Factory::ObjectFactory::instance().clear();

  // Clear the menu bar. Do this before plugins are released.
  _recentFilesMenu = 0x0;

  // Wait for jobs before plugins are released.
  // At least on OS X, using a pointer created in a shared object after it is released causes a crash.
  MainWindowBase::_waitForJobs();
  
  // Release all the plugins.
  this->releasePlugins();
  
  // Clear all loaded libraries.
  Usul::DLL::LibraryPool::instance().clear ( &std::cout );

  // Wait here until all jobs are done.
  // It is unlikely that a plugin will create a job in its destructor, but leaving this here to make sure.
  MainWindowBase::_waitForJobs();
  Usul::Jobs::Manager::destroy();

  // Should be true.
  //USUL_ASSERT ( 0 == _refCount );

  // Delete remaining members.
  _vendor.clear();
  _program.clear();
  _icon.clear();
  _output.clear();
  _splash = 0x0;
  _idleTimer = 0x0;

  // Delete the mutex last.
  delete _mutex; _mutex = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Wait for all jobs.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_waitForJobs()
{
  std::cout << "Waiting for all jobs to finish..." << std::endl;
  Usul::Jobs::Manager::instance().cancel();
  Usul::Jobs::Manager::instance().wait();
  std::cout << "All jobs have finished" << std::endl;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Load the settings.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_loadSettings()
{
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "2863006339" );

  // Read the file and populate the registry. Not a big deal if this fails.
  const std::string settingsFile ( this->settingsFileName() );
  Usul::Functions::safeCall ( boost::bind ( XmlTree::RegistryIO::read, settingsFile, boost::ref ( Usul::Registry::Database::instance() ) ), "1123442106" );

  // Set the window's properties.
  Usul::Registry::Node &mw ( Reg::instance()[Sections::MAIN_WINDOW] );
  this->restoreGeometry ( mw[Keys::GEOMETRY].get<QByteArray> ( QByteArray() ) );

  // Get recent-files list and remove files that do not exist.
  _recentFiles = mw[Keys::RECENT_FILES]["files"].get<StringList> ( StringList() );
  
  // TODO: Fix this
  //_recentFiles.remove_if ( boost::bind ( boost::filesystem::exists, _1 ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Save the settings.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_saveSettings()
{
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "9722347090" );

  // Set the window's properties.
  Usul::Registry::Node &mw ( Reg::instance()[Sections::MAIN_WINDOW] );
  mw[Keys::GEOMETRY] = this->saveGeometry();

  // Save dock window positions.
  mw[Keys::DOCK_WINDOW_POSITIONS] = this->saveState ( CadKit::Helios::Core::Constants::VERSION );

  // Save recent files.
  mw[Keys::RECENT_FILES]["files"] = _recentFiles;

  // Write to disk.
  const std::string settingsFile ( this->settingsFileName() );
  Usul::Functions::safeCall ( boost::bind ( XmlTree::RegistryIO::write, settingsFile, boost::ref ( Usul::Registry::Database::instance() ) ), "4136994389" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make recent files menu.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_initRecentFilesMenu()
{
#if 0
  // Make a menu if we need to.
  if ( 0x0 == _recentFilesMenu )
  {
    _recentFilesMenu = new Helios::Menus::Menu ( "&Recent Files" );
  }

  // Clear what we have.
  _recentFilesMenu->clear();

  // For convienence.
  Usul::Interfaces::IUnknown::QueryPtr me ( this );

  // Add to the recent file menu if the file exists.
  unsigned int count ( 0 );
  for ( StringList::const_iterator iter = _recentFiles.begin(); iter != _recentFiles.end(); ++iter )
  {
    const StringList::value_type file ( *iter );
    if ( true == boost::filesystem::exists ( file ) )
    {
      std::string label ( Usul::Convert::Type<unsigned int,std::string>::convert ( ++count ) );
      label.insert ( ( 1 == label.size() ) ? 0 : label.size() - 1, "&" );
      _recentFilesMenu->append ( MenuKit::Button::create ( Usul::Strings::format ( label, ' ', file ), boost::bind ( &MainWindowBase::_openDocument, this, file ) ) );
    }
  }

  _recentFilesMenu->addSeparator();

  // Add a clear button.
  _recentFilesMenu->append ( MenuKit::Button::create ( "Clear", boost::bind ( &MainWindowBase::_clearRecentFiles, this ) ) );

  // Enable the menu only if there are recent files.
  //_recentFilesMenu->enabled ( false == _recentFiles.empty() );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear recent files.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_clearRecentFiles()
{
  _recentFiles.clear();
  this->_initRecentFilesMenu();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Increment the reference count.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::ref()
{
  Guard guard ( this->mutex() );
  ++_refCount;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Decrement the reference count.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::unref ( bool )
{
  Guard guard ( this->mutex() );

  if ( 0 == _refCount )
  {
    USUL_ASSERT ( 0 );
    throw Usul::Exceptions::Exception ( "Error 4107780854: Reference count is already 0" );
  }

  --_refCount;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query for the interface.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown *MainWindowBase::queryInterface ( unsigned long iid )
{
  // No need to guard, should be re-entrant.

  switch ( iid )
  {
  case Usul::Interfaces::IUnknown::IID:
  case Usul::Interfaces::Qt::IMainWindow::IID:
    return static_cast < Usul::Interfaces::Qt::IMainWindow* > ( this );
  case Usul::Interfaces::IPasswordPrompt::IID:
    return static_cast < Usul::Interfaces::IPasswordPrompt* > ( this );
  default:
    return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the directory where this binary lives.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::directory() const
{
  // No guard needed.
  return Usul::CommandLine::Arguments::instance().directory();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the icon file name.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::icon() const
{
  Guard guard ( this->mutex() );
  return std::string ( _icon.begin(), _icon.end() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the program file name.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::programFile() const
{
  //Guard guard ( this->mutex() );

  const std::string path ( Usul::CommandLine::Arguments::instance().program() );
  std::ostringstream file;
  file << Usul::File::base ( path ) << '.' << Usul::File::extension ( path );
  return file.str();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the program name.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::programName() const
{
  Guard guard ( this->mutex() );
  return std::string ( _program.begin(), _program.end() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the vendor name.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::vendor() const
{
  Guard guard ( this->mutex() );
  return std::string ( _vendor.begin(), _vendor.end() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the about.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::about() const
{
  Guard guard ( this->mutex() );
  return std::string ( _about.begin(), _about.end() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the default plugin file name.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::pluginFile() const
{
  // If we get to here then figure out the file.
  #ifdef __APPLE__
  // Relative path for an application bundle on OS X.
  std::string relativePath ( "/../Plugins/" );
  #else
  std::string relativePath ( "/../configs/" );
  #endif

  std::string file ( Usul::Strings::format ( this->directory(), relativePath, this->programName(), ".plugins" ) );
  std::replace ( file.begin(), file.end(), '\\', '/' );

  return file;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Print the loaded plugin names.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::printPlugins() const
{
  Usul::Components::Manager::instance().print ( std::cout );
}


///////////////////////////////////////////////////////////////////////////////ƒ
//
//  Load the plugins.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::loadPlugins ( const std::string &config )
{
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "4106161463" );

  if ( false == boost::filesystem::exists ( config ) )
  {
    std::cout << "Warning 2088092978: Plugin file '" << config << "' does not exist" << std::endl;
    return;
  }

  Usul::Interfaces::IUnknown::QueryPtr unknown ( this );
  Usul::Interfaces::IUnknown::QueryPtr splash ( _splash );

  typedef Usul::Components::Loader < XmlTree::Document > Loader;
  Loader loader;

  // Set the directory on Apple.  Allow user to over ride by calling this before parse.
#ifdef __APPLE__
  const std::string directory ( Usul::CommandLine::Arguments::instance().directory() + "/../Plugins/" );
  loader.directory ( directory );
#endif
  
  std::cout << Usul::Strings::format ( "Loading plugin file: ", config ) << std::endl;

	loader.parse ( config );
  loader.load ( ( splash.valid() ) ? splash : unknown );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Release all the plugins.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::releasePlugins()
{
  Usul::Components::Manager::instance().clear ( &std::cout );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show the splash screen.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::showSplashScreen()
{
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "3041826876" );

  // Make the splash screen if we have to.
  if ( false == _splash.valid() )
  {
    const std::string splashImage ( Usul::App::Application::instance().splashImage() );
    _splash = new SplashScreen ( splashImage );
  }

  if ( true == _splash.valid() )
  {
    _splash->show();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Hide the splash screen.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::hideSplashScreen()
{
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "3041826876" );
  if ( true == _splash.valid() )
  {
    _splash->hide();
    _splash = 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the text window.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::updateTextWindow ( bool force )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Use when profiling.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef USUL_USING_PROFILER

namespace Helper
{
  void autoQuit ( unsigned int maxCount )
  {
    static unsigned int count ( 0 );
    bool print ( true );
    
    ++count;

    if ( count >= maxCount )
    {
      QApplication::quit();
    }
    if ( true == print )
    {
      std::cout << Usul::Strings::format ( "Idle count: ", count, " of ", maxCount, '\n' ) << std::flush;
    }
  }
}

#endif // USUL_USING_PROFILER


///////////////////////////////////////////////////////////////////////////////
//
//  Called by the idle timer.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_idleProcess()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  // Tell window to refresh.
  Usul::Functions::safeCall ( boost::bind ( &MainWindowBase::updateTextWindow, this, true ) );

  #ifdef USUL_USING_PROFILER
  Helper::autoQuit ( 10 );
  #endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear all the documents.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_clearDocuments()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the MainWindowBase.
//
///////////////////////////////////////////////////////////////////////////////

QMainWindow * MainWindowBase::mainWindow()
{
  return this;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the MainWindowBase.
//
///////////////////////////////////////////////////////////////////////////////

const QMainWindow* MainWindowBase::mainWindow() const
{
  return this;
}


///////////////////////////////////////////////////////////////////////////////
//
//  The document has finished loading.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::notifyDocumentFinishedLoading ( void* document )
{
  if ( 0x0 != document )
  {
    // This will add an event to the applications event loop.  It will be executed in the proper thread.
    QMetaObject::invokeMethod ( this, "_notifyDocumentFinishedLoading", Qt::QueuedConnection, 
                                Q_ARG ( void*, document ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  The document has finished loading. This function cannot throw!
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_notifyDocumentFinishedLoading ( void* document  )
{
  // If we get this far it should be the gui thread.
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  // Safely call the notification function.
  Usul::Functions::safeCall ( 
    boost::bind ( &MainWindowBase::_notifyFinishedLoading, this, reinterpret_cast<Minerva::Document::MinervaDocument*> ( document ) ), "3741587952" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The document has finished loading.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_notifyFinishedLoading ( Minerva::Document::MinervaDocument* document )
{
  // If we get this far it should be the gui thread.
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  // Handle no document.
  if ( 0x0 == document )
    return;

  // The filename.
  std::string name ( document->fileName() );

  // Replace back-slashes. Otherwise, on Windows we can get the same file twice.
  std::replace ( name.begin(), name.end(), '\\', '/' );

  // Add the document to the recent file list if it isn't already in there.
  if ( _recentFiles.end() == std::find ( _recentFiles.begin(), _recentFiles.end(), name ) )
    _recentFiles.push_front ( name );

  // If the list is too long then remove the oldest (the front).
  const unsigned int maxRecentFiles ( Reg::instance()[Sections::MAIN_WINDOW][Keys::RECENT_FILES]["max_num"].get<unsigned int> ( 15 ) );
  while ( _recentFiles.size() > maxRecentFiles )
    _recentFiles.pop_back();
}


///////////////////////////////////////////////////////////////////////////////
//
//  A close event has been recieved.  Close any windows that the workspace 
//  has open. Calling in the destructor is too late because the event loop 
//  has exited.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::closeEvent ( QCloseEvent* event )
{
  Usul::Functions::safeCall ( boost::bind ( &MainWindowBase::_closeEvent, this, event ), "1179334380" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  A close event has been recieved.  Close any windows that the workspace 
//  has open. Calling in the destructor is too late because the event loop 
//  has exited.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_closeEvent ( QCloseEvent* event )
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  // Close child windows first.
  // TODO: How should we handle this in a SDI application?
  //_workSpace->closeAllWindows();

  // Pass along to the base class.
  BaseClass::closeEvent ( event );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Restore dock window positions.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::restoreDockWindows()
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  QByteArray defaults ( this->saveState ( CadKit::Helios::Core::Constants::VERSION ) );
  QByteArray positions ( Reg::instance()[Sections::MAIN_WINDOW][Keys::DOCK_WINDOW_POSITIONS].get<QByteArray> ( defaults ) );
  this->restoreState ( positions, CadKit::Helios::Core::Constants::VERSION );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Dragging has entering window.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::dragEnterEvent ( QDragEnterEvent *event )
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

#if 0
  typedef QList < QUrl > Urls;
  typedef Urls::const_iterator ConstIterator;

  Urls urls ( event->mimeData()->urls() );

  for ( ConstIterator i = urls.begin(); i != urls.end(); ++ i )
  {
    std::string file ( i->toLocalFile().toStdString() );

    if ( Usul::Documents::Manager::instance().canOpen ( file ) )
      event->acceptProposedAction();
  }
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Files have been dropped.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::dropEvent ( QDropEvent *event )
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  typedef QList < QUrl > Urls;
  typedef Urls::const_iterator ConstIterator;

  Urls urls ( event->mimeData()->urls() );

  Usul::Interfaces::IUnknown::QueryPtr me ( this );

  for ( ConstIterator i = urls.begin(); i != urls.end(); ++ i )
  {
    std::string file ( i->toLocalFile().toStdString() );
    this->_openDocument ( file );
  }

  event->acceptProposedAction();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Parse the command-line.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::parseCommandLine ( int argc, char **argv )
{
  Usul::Functions::safeCall ( boost::bind ( &MainWindowBase::_parseCommandLine, this, argc, argv ), "4427951490" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Parse the command-line.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_parseCommandLine ( int argc, char **argv )
{
  for ( int i = 1; i < argc; ++i )
  {
    this->loadFile ( argv[i] );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Load the file.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::loadFile ( const std::string &file )
{
  if ( true == boost::filesystem::exists ( file ) )
  {
    this->_openDocument ( file );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return name of registry file.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::settingsFileName() const
{
  // Get persistant directory.
  const std::string dir ( Usul::User::Directory::vendor ( this->vendor(), true ) + this->programName() + "/" );

  // Make sure it exists.
  boost::filesystem::create_directories ( dir );

  // Make the file name.
  std::string name ( dir + this->programName() + ".settings" );

  // Return file name.
  return name;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create a new document.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_newDocument()
{
  // Create a document.
  Minerva::Document::MinervaDocument::RefPtr document ( new Minerva::Document::MinervaDocument );
  
  // Assign a default name.
  document->defaultFilename();
  
  // Create the gui.
  this->_notifyFinishedLoading ( document );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Open a document.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_openDocument()
{
  typedef Minerva::QtTools::FileDialog FileDialog;
  typedef FileDialog::Filters Filters;
  typedef FileDialog::Filter Filter;
  typedef FileDialog::FileResult FileResult;

  // Get appropriate filters.
  Minerva::Document::MinervaDocument::RefPtr temp ( new Minerva::Document::MinervaDocument );
  Filters filters ( temp->filtersOpen() );

  // Get the file name.
  const std::string title ( "Open Document" );
  FileResult result ( FileDialog::getLoadFileName ( this, title, filters ) );
  const std::string filename ( result.first );

  this->_openDocument ( filename );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Open a document.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindowBase::_openDocument ( const std::string& filename )
{
  if ( !filename.empty() )
  {
    OpenFileThread *thread ( new OpenFileThread ( filename ) );
    
    QObject::connect ( thread, SIGNAL ( documentLoaded ( void* ) ),
                      this, SLOT ( notifyDocumentFinishedLoading ( void* ) ) );
    
    thread->start();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Prompt the user for a password.
//
///////////////////////////////////////////////////////////////////////////////

std::string MainWindowBase::promptForPassword ( const std::string& text )
{
  boost::scoped_ptr<QPasswordPromptWidget> prompt ( new QPasswordPromptWidget );
  return prompt->promptForPassword ( text );
}
