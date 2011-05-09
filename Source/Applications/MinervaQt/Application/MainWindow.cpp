
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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AddFileSystemComponent.h"
#include "SnapShotWidget.h"
#include "Viewer.h"
#include "Format.h"

#include "Minerva/Qt/Widgets/AnimationControl.h"
#include "Minerva/Qt/Widgets/Favorites.h"
#include "Minerva/Qt/Tools/FileDialog.h"

#include "Minerva/Document/MinervaDocument.h"

#include "Helios/Menus/Button.h"
#include "Helios/Menus/MenuAdapter.h"

#include "Usul/Components/Manager.h"
#include "Usul/Errors/Assert.h"
#include "Usul/Registry/Database.h"
#include "Usul/Threads/Named.h"

#include "QtGui/QTextEdit"

#include "boost/bind.hpp"
#include "boost/lexical_cast.hpp"

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow ( const std::string &vendor, 
                         const std::string &url, 
                         const std::string &program,
                         const std::string &icon,
                         const std::string &output,
                         const std::string& about,
                         bool showSplash ) : BaseClass ( vendor, url, program, icon, output, about, showSplash ),
  _implementation ( new Ui::MainWindow ),
  _favorites ( new Minerva::QtWidgets::Favorites ),
  _viewer ( 0x0 ),
  _document ( 0x0 ),
  _animationControl ( 0x0 ),
  _snapShotWidget ( 0x0 ),
  _cameraPathController ( new Minerva::Document::CameraPathController ),
  _textWindow(),
  _dockingWindowsMenu ( new Helios::Menus::Menu ),
  _pathMenu ( new Helios::Menus::Menu ),
  _cameraMenu ( new Helios::Menus::Menu )
{
  _implementation->setupUi ( this );

  this->setWindowTitle ( "Minerva" );

  // Set the favorites.
  _implementation->_layers->favorites ( _favorites );

  QObject::connect ( _implementation->_layers, SIGNAL ( addLayerFavorites ( Minerva::Core::Data::Feature* ) ), 
                     _favorites, SLOT ( addLayer ( Minerva::Core::Data::Feature* ) ) );
  QObject::connect ( _implementation->_layers, SIGNAL ( layerDirty ( Minerva::Core::Data::Feature* ) ), 
                    this, SLOT ( dirtyAndRedraw( Minerva::Core::Data::Feature * ) ) );
  QObject::connect ( _implementation->_layers, SIGNAL ( layerDoubleClicked ( Minerva::Core::Data::Feature* ) ), 
                    this, SLOT ( flyToFeature ( Minerva::Core::Data::Feature * ) ) );
  QObject::connect ( _implementation->_layers, SIGNAL ( documentModified() ), 
                    this, SLOT ( modifyDocument() ) );
  QObject::connect ( _implementation->_flyTo, SIGNAL ( flyToLocation(double,double) ),
                    this, SLOT ( flyToLocation(double,double) ) );

  {
    AddFileSystemComponent::RefPtr component ( new AddFileSystemComponent );
    Usul::Components::Manager::instance().addPlugin ( Usul::Interfaces::IUnknown::QueryPtr ( component ) );
  }

  // Build the docking window.
  {
    QDockWidget* dock ( new QDockWidget ( QObject::tr ( "Animation Control" ), this ) );
    dock->setAllowedAreas ( Qt::AllDockWidgetAreas );

    _animationControl = new Minerva::QtWidgets::AnimationControl ( dock );
    _animationControl->setObjectName ( "Animation Control" );

    QObject::connect ( _animationControl, SIGNAL ( renderRequired() ), this, SLOT ( requestRender() ) );
    QObject::connect ( _implementation->_layers, SIGNAL ( documentModified() ), _animationControl, SLOT ( documentModified() ) );

    // Add the dock to the main window.
    dock->setWidget ( _animationControl );
    this->addDockWidget ( Qt::BottomDockWidgetArea, dock );
    this->addDockWidgetMenu ( dock );
  }

  {
    QDockWidget* dock ( new QDockWidget ( QObject::tr ( "Snap Shot" ), this ) );
    dock->setAllowedAreas ( Qt::AllDockWidgetAreas );

    _snapShotWidget = new SnapShotWidget ( dock );

    QObject::connect ( _snapShotWidget, SIGNAL ( takePicture ( QString, double, unsigned int ) ),
                       this, SLOT ( takePicture ( QString, double, unsigned int )  ) );

    dock->setWidget ( _snapShotWidget );
    this->addDockWidget ( Qt::BottomDockWidgetArea, dock );
    this->addDockWidgetMenu ( dock );

    dock->hide();
  }

  this->_newDocument();

  Helios::Menus::MenuAdapter *dockingWindowsMenu ( new Helios::Menus::MenuAdapter ( "Docking Windows" ) );
  dockingWindowsMenu->menu ( _dockingWindowsMenu );
  _implementation->menuView->insertMenu ( _implementation->actionEdit, dockingWindowsMenu );

  Helios::Menus::MenuAdapter *cameraMenu ( new Helios::Menus::MenuAdapter ( "Cameras" ) );
  cameraMenu->menu ( _cameraMenu );
  _implementation->menuCamera->addMenu ( cameraMenu );

  Helios::Menus::MenuAdapter *pathMenu ( new Helios::Menus::MenuAdapter ( "Paths" ) );
  pathMenu->menu ( _pathMenu );
  _implementation->menuCamera->addMenu ( pathMenu );

  // Build the text window.
  this->_buildTextWindow();
  _textWindow.second.open ( output.c_str() );

  this->addDockWidgetMenu ( _implementation->dockWidget );
  this->addDockWidgetMenu ( _implementation->dockWidget_3 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
  // Clear the layers.
  _implementation->_layers->buildTree ( 0x0 );

  _document = 0x0;

  if ( 0x0 != _viewer )
  {
    delete _viewer;
  }

  delete _implementation;
  delete _favorites;
  delete _animationControl;
  delete _snapShotWidget;

  _textWindow.first = 0x0;
  _textWindow.second.close();

  _dockingWindowsMenu = 0x0;
  _pathMenu = 0x0;
  _cameraMenu = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the user interface.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_notifyFinishedLoading ( Minerva::Document::MinervaDocument* document )
{
  // Set the document.
  _document = document;

  if ( document )
  {
    Viewer *viewer = new Viewer ( document, defaultFormat(), this );

    if ( _viewer )
    {
      viewer->setGeometry ( _viewer->geometry() );
    }

    // Show the window.
    viewer->show();
    
    // Set the focus.
    viewer->setFocus();

    QMainWindow::setCentralWidget ( viewer );

    if ( _viewer )
    {
      delete _viewer;
      _viewer = 0x0;
    }

    _viewer = viewer;
    
    if ( 0x0 != _implementation->_layers )
      _implementation->_layers->buildTree ( document->body()->container() );
  }

  _animationControl->document ( document );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The given layer is dirty.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::dirtyAndRedraw ( Minerva::Core::Data::Feature* feature )
{
  if ( _document )
  {
    _document->dirtyScene ( true, feature );
  }
  
  if ( _viewer )
  {
    _viewer->update();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make a new document.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionNew_triggered ( bool )
{
  this->_newDocument();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Open a document.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionOpen_triggered ( bool )
{
  this->_openDocument();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Save the document.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionSave_triggered ( bool )
{
  this->_saveDocument();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Save a document to a different file.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionSave_As_triggered ( bool )
{
  this->_saveAsDocument();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Save the document.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_saveDocument()
{
  if ( _document.valid() )
  {
    // If the filename is valid...
    if ( _document->fileValid() )
    {
      this->_saveDocument( _document->fileName() );
    }

    // Otherwise...
    else
    {
      this->_saveAsDocument();
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Save a document to filename.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_saveDocument ( const std::string& filename )
{
  if ( _document.valid() )
  {
    _document->write ( filename );

    // Set the file name.
    _document->fileName ( filename );
    _document->fileValid ( true );

    // It's no longer modified.
    _document->modified ( false );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Save a document to a different file.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_saveAsDocument()
{
  if ( true == _document.valid() )
  {
    // Ask for file name.
    typedef Minerva::QtTools::FileDialog FileDialog;
    typedef FileDialog::FileResult FileResult;

    const std::string title ( "Save" );
    const FileResult result ( FileDialog::getSaveFileName ( this, title, _document->filtersSave() ) );

    if ( false == result.first.empty() )
    {
      this->_saveDocument ( result.first );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Export an image.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionExport_Image_triggered ( bool )
{
  if ( _viewer )
  {
    // Get the filters.
    typedef Minerva::QtTools::FileDialog FileDialog;
    typedef FileDialog::Filters Filters;
    typedef FileDialog::Filter Filter;
    typedef FileDialog::FileResult FileResult;
    Filters filters;
    filters.push_back ( Filter ( "JPEG Image (*.jpg)", "*.jpg"   ) );
    filters.push_back ( Filter ( "PNG Image (*.png)", "*.png"    ) );
    filters.push_back ( Filter ( "BMP Image (*.bmp)", "*.bmp"    ) );
  
    // Get the filename.
    FileResult result ( FileDialog::getSaveFileName ( this, "Export Image", filters ) );
    const std::string &filename ( result.first );
    
    // Return now if the filename is empty.
    if ( filename.empty() )
      return;

    QImage image ( _viewer->grabFrameBuffer() );
    image.save ( filename.c_str() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle work offline flag.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionWork_Offline_triggered ( bool )
{
  // Toggle the state.
	const bool workOffline ( Usul::Registry::Database::instance()["work_offline"].get<bool> ( false, true ) );
	Usul::Registry::Database::instance()["work_offline"] = !workOffline;
  _implementation->actionWork_Offline->setChecked ( workOffline );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Exit the application.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionExit_triggered ( bool )
{
  // Closing all windows will exit the program.
  QApplication::closeAllWindows();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Fly to the given feature.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::flyToFeature ( Minerva::Core::Data::Feature* feature )
{
  if ( _viewer )
  {
    _viewer->flyToFeature ( feature );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Fly to the given location.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::flyToLocation ( double lon, double lat )
{
  if ( _viewer )
  {
    _viewer->flyToLocation ( lon, lat );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the current view matrix.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::setViewMatrix ( Usul::Math::Matrix44d m )
{
  if ( _viewer )
  {
    _viewer->setViewMatrix ( m );
    _viewer->update();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the near/far ratio.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionEdit_triggered ( bool )
{
  if ( _viewer )
  {
    _viewer->editNearFarRatio();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Reset the camera.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionReset_View_triggered ( bool )
{
  if ( _viewer )
  {
    _viewer->resetCamera();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle frame dump state.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionFrame_Dump_triggered ( bool value )
{
  if ( _viewer )
  {
    _viewer->setFrameDump ( value );
    _implementation->actionFrame_Dump->setChecked ( _viewer->getFrameDump() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle visibility of mouse position feedback.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionShow_Position_triggered ( bool value )
{
  if ( _viewer )
  {
    _viewer->setShowPosition ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle visibility of eye altitude feedback.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionShow_Eye_Altitude_triggered ( bool value )
{
  if ( _viewer )
  {
    _viewer->setShowEyeAltitude ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle visibility of job feedback.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionShow_Job_Feedback_triggered ( bool value )
{
  if ( _viewer )
  {
    _viewer->setShowJobFeedback ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle visibility of date feedback.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionShow_Date_Feedback_triggered ( bool value )
{
  if ( _viewer )
  {
    _viewer->setShowDateFeedback ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle visibility borders.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionShow_Borders_triggered ( bool value )
{
  if ( _viewer )
  {
    _viewer->setShowBorders ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle allowing of more tile splitting.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionAllow_Splitting_triggered ( bool value )
{
  if ( _viewer )
  {
    _viewer->setAllowSplitting ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle keeping of detail.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionKeep_Detail_triggered ( bool value )
{
  if ( _viewer )
  {
    _viewer->setKeepDetail ( value );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Queue a render request.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::requestRender()
{
  if ( _viewer )
  {
    _viewer->update();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Mark the document as modified.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::modifyDocument()
{
  if ( _document )
  {
    _document->modified ( true );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Take a picture.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::takePicture ( QString filename, double frameScale, unsigned int numSamples )
{
  if ( _viewer )
  {
    _viewer->takePicture ( filename, frameScale, numSamples );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create a new path.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionNew_Path_triggered ( bool )
{
  if ( _document )
  {
    Minerva::Document::CameraPath::RefPtr path ( _document->createAndAppendNewPath() );
    _cameraPathController->currentPath ( path );
    this->_buildPathMenu();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Append the current camera.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionAppend_Camera_triggered ( bool )
{
  if ( _viewer )
  {
    this->_getCurrentPath();
    _cameraPathController->currentCameraAppend ( _viewer->camera() );
    this->_buildCameraMenu();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Prepend the current camera.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionPrepend_Camera_triggered ( bool )
{
  if ( _viewer )
  {
    this->_getCurrentPath();
    _cameraPathController->currentCameraPrepend ( _viewer->camera() );
    this->_buildCameraMenu();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Insert between the current camera.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionInsert_Between_Closest_triggered ( bool )
{
  if ( _viewer )
  {
    this->_getCurrentPath();
    _cameraPathController->currentCameraInsert ( _viewer->camera() );
    this->_buildCameraMenu();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove the closest camera.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionRemove_Closest_triggered ( bool )
{
  if ( _viewer )
  {
    this->_getCurrentPath();
    _cameraPathController->currentCameraRemove ( _viewer->camera() );
    this->_buildCameraMenu();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Close the path.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionClose_Path_triggered ( bool )
{
  this->_getCurrentPath();
  _cameraPathController->closePath();
  this->_buildCameraMenu();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Replace the closest camera.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionReplace_Closest_triggered ( bool )
{
  if ( _viewer )
  {
    this->_getCurrentPath();
    _cameraPathController->currentCameraReplace ( _viewer->camera() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play path forward.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionPlay_Forward_triggered ( bool )
{
  if ( _viewer )
  {
    _viewer->playPathForward ( _cameraPathController->currentPath() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Play path backward.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionPlay_Backward_triggered ( bool )
{
  if ( _viewer )
  {
    _viewer->playPathBackward ( _cameraPathController->currentPath() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Stop animation.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionStopCameraAnimation_triggered ( bool )
{
  if ( _viewer )
  {
    _viewer->stopAnimation();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get or create the current path.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Document::CameraPath::RefPtr MainWindow::_getCurrentPath()
{
  Minerva::Document::CameraPath::RefPtr path ( _cameraPathController->currentPath() );
  if ( !path )
  {
    if ( _document )
    {
      if ( 0 == _document->numberOfPaths() )
      {
        path = _document->createAndAppendNewPath();
      }
      else
      {
        path = _document->getPath ( 0 );
      }
      _cameraPathController->currentPath ( path );
      this->_buildPathMenu();
    }
  }
  
  return path;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add dock widget to menu.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::addDockWidgetMenu ( QDockWidget * dock )
{
  USUL_ASSERT ( _dockingWindowsMenu );

  std::string text ( 0x0 != dock ? dock->windowTitle().toStdString() : "" );

  typedef Helios::Menus::Button Button;
  _dockingWindowsMenu->append ( new Button ( text, "", "",
                                             boost::bind ( &MainWindow::_toggleVisiblity, dock ),
                                             boost::bind ( &MainWindow::_isVisible, _1, dock ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle the visibility of the widget.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_toggleVisiblity ( QWidget* widget )
{
  if ( widget )
  {
    widget->setVisible ( !widget->isVisible() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the widget visible?
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_isVisible ( QAction& action, QWidget* widget )
{
  action.setCheckable ( true );
  action.setChecked ( 0x0 != widget ? widget->isVisible () : false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the text window.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::updateTextWindow ( bool force )
{
  USUL_THREADS_ENSURE_GUI_THREAD ( return );

  // Handle no text window.
  if ( 0x0 == _textWindow.first )
    return;

  // Don't allow this to throw because it may create an infinite loop.
  try
  {
    bool changed ( false );

    if ( EOF != _textWindow.second.peek() )
    {
      std::string line;
      std::getline ( _textWindow.second, line );
      _textWindow.first->append ( line.c_str() );
      changed = true;
    }

    // Force a GUI update now if we should.
    if ( true == force && true == changed )
    {
      _textWindow.first->update();
    }
  }

  // Catch all exceptions.
  catch ( ... )
  {
    try
    {
      _textWindow.first->append ( "Error 1536020510: failed to append new text output" );
    }
    catch ( ... )
    {
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the settings.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_buildTextWindow()
{
  USUL_THREADS_ENSURE_GUI_THREAD_OR_THROW ( "3978847086" );

  // Handle repeated calls.
  if ( 0x0 != _textWindow.first )
    return;

  // Build the docking window.
  QDockWidget *dock = new QDockWidget ( tr ( "Text Output" ), this );
  dock->setAllowedAreas ( Qt::AllDockWidgetAreas );

  // Make the text window.
  _textWindow.first = new QTextEdit ( dock );

  // Set properties.
  _textWindow.first->setReadOnly ( true );

  // Keep text window from growing too big.
  if ( 0x0 != _textWindow.first->document() )
  {
    // Set maximum block count.
    _textWindow.first->document()->setMaximumBlockCount ( 1000 );
  }

  // Dock it.
  dock->setWidget ( _textWindow.first );
  this->addDockWidget ( Qt::BottomDockWidgetArea, dock );

  // Set the object name.
  dock->setObjectName ( "TextWindow" );

  // Add toggle to the menu.
  this->addDockWidgetMenu ( dock );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the path menu.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_buildPathMenu()
{
  USUL_ASSERT ( _pathMenu );
  _pathMenu->clear();

  if ( _document )
  {
    for ( unsigned int i = 0; i < _document->numberOfPaths(); ++i )
    {
      Minerva::Document::CameraPath::RefPtr camera ( _document->getPath ( i ) );
      if ( camera )
      {
        typedef Helios::Menus::Button Button;
        _pathMenu->append ( new Button ( "Path " + boost::lexical_cast<std::string> ( i ), "", "",
                                         boost::bind ( &MainWindow::_setCurrentPath, this, camera ),
                                         boost::bind ( &MainWindow::_isPathCurrent, this, _1, camera ) ) );
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the path current?
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_isPathCurrent ( QAction& action, Minerva::Document::CameraPath::RefPtr path ) const
{
  USUL_ASSERT ( _cameraPathController );
  action.setCheckable ( true );
  action.setChecked ( path == _cameraPathController->currentPath() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the current path.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_setCurrentPath ( Minerva::Document::CameraPath::RefPtr path )
{
  USUL_ASSERT ( _cameraPathController );
  _cameraPathController->currentPath ( path );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the camera menu.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_buildCameraMenu()
{
  USUL_ASSERT ( _cameraPathController );
  USUL_ASSERT ( _cameraMenu );
  _cameraMenu->clear();

  Minerva::Document::CameraPath::RefPtr path ( _cameraPathController->currentPath() );
  if ( path )
  {
    for ( unsigned int i = 0; i < path->size(); ++i )
    {
      typedef Helios::Menus::Button Button;
      _cameraMenu->append ( new Button ( "Camera " + boost::lexical_cast<std::string> ( i ), "", "",
                                       boost::bind ( &MainWindow::_goToCameraPosition, this, i ) ) );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Go to the camera position.
//
///////////////////////////////////////////////////////////////////////////////

void MainWindow::_goToCameraPosition ( unsigned int i )
{
  USUL_ASSERT ( _cameraPathController );

  if ( _viewer )
  {
    _viewer->goToCameraPostion ( _cameraPathController->currentPath(), i );
  }
}
