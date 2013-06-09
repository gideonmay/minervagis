
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Main window.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_QT_MAIN_WINDOW_H_
#define _MINERVA_QT_MAIN_WINDOW_H_

#include "MainWindowBase.h"

#ifndef Q_MOC_RUN
#include "Minerva/Document/MinervaDocument.h"
#include "Minerva/Document/CameraPathController.h"
#endif

namespace Ui { class MainWindow; }
class Viewer;
class SnapShotWidget;

namespace Minerva { namespace QtWidgets { class AnimationControl; class Favorites; } }

class MainWindow : public MainWindowBase
{
  Q_OBJECT

public:

  // Useful typedefs.
  typedef MainWindowBase BaseClass;

  // Constructor and destructor.
  MainWindow ( const std::string &vendor, 
               const std::string &url, 
               const std::string &program,
               const std::string &icon,
               const std::string &output,
               const std::string& about,
               bool showSplash = true );
  virtual ~MainWindow();

  // Add dock widget to menu.
  void addDockWidgetMenu ( QDockWidget * dock );

  virtual void  updateTextWindow ( bool force );

public slots:
  
  void dirtyAndRedraw( Minerva::Core::Data::Feature* );
  void flyToFeature ( Minerva::Core::Data::Feature* );
  void flyToLocation ( double lon, double lat );
  void setViewMatrix ( Usul::Math::Matrix44d );

  void requestRender();

  void modifyDocument();

  void takePicture ( QString filename, double frameScale, unsigned int numSamples );

private slots:

  // File menu.
  void on_actionNew_triggered ( bool );
  void on_actionOpen_triggered ( bool );
  void on_actionSave_triggered ( bool );
  void on_actionSave_As_triggered ( bool );
  void on_actionExport_Image_triggered ( bool );
  void on_actionWork_Offline_triggered ( bool );
  void on_actionExit_triggered ( bool );
  
  // View menu.
  void on_actionEdit_triggered ( bool );
  void on_actionReset_View_triggered ( bool );
  void on_actionFrame_Dump_triggered ( bool );
  void on_actionShow_Position_triggered ( bool );
  void on_actionShow_Eye_Altitude_triggered ( bool );
  void on_actionShow_Job_Feedback_triggered ( bool );
  void on_actionShow_Date_Feedback_triggered ( bool );
  void on_actionShow_Borders_triggered ( bool );
  void on_actionAllow_Splitting_triggered ( bool );
  void on_actionKeep_Detail_triggered ( bool );

  // Camera menu.
  void on_actionNew_Path_triggered ( bool );
  void on_actionAppend_Camera_triggered ( bool );
  void on_actionPrepend_Camera_triggered ( bool );
  void on_actionInsert_Between_Closest_triggered ( bool );
  void on_actionRemove_Closest_triggered ( bool );
  void on_actionClose_Path_triggered ( bool );
  void on_actionReplace_Closest_triggered ( bool );
  void on_actionPlay_Forward_triggered ( bool );
  void on_actionPlay_Backward_triggered ( bool );
  void on_actionStopCameraAnimation_triggered ( bool );

private:
  
  void _buildTextWindow();
  void _buildPathMenu();
  void _buildCameraMenu();

  Minerva::Document::CameraPath::RefPtr _getCurrentPath();

  void _isPathCurrent ( QAction& action, Minerva::Document::CameraPath::RefPtr ) const;
  void _setCurrentPath ( Minerva::Document::CameraPath::RefPtr );

  void _goToCameraPosition ( unsigned int i );

  void _saveDocument();
  void _saveDocument ( const std::string& filename );
  void _saveAsDocument();

  virtual void _notifyFinishedLoading ( Minerva::Document::MinervaDocument* );

  static void _toggleVisiblity ( QWidget* widget );
  static void _isVisible ( QAction& action, QWidget* widget );

  // Typedefs.
  typedef std::pair<QTextEdit*,std::ifstream> TextWindow;

  // Members.
  Ui::MainWindow *_implementation;
  Minerva::QtWidgets::Favorites *_favorites;
  Viewer *_viewer;
  Minerva::Document::MinervaDocument::RefPtr _document;
  Minerva::QtWidgets::AnimationControl *_animationControl;
  SnapShotWidget *_snapShotWidget;
  Minerva::Document::CameraPathController::RefPtr _cameraPathController;

  TextWindow _textWindow;

  Helios::Menus::Menu::RefPtr _dockingWindowsMenu;
  Helios::Menus::Menu::RefPtr _pathMenu;
  Helios::Menus::Menu::RefPtr _cameraMenu;
};


#endif //_MINERVA_QT_MAIN_WINDOW_H_
