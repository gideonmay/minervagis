
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author(s): Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __VRV_CORE_APPLICATION_H__
#define __VRV_CORE_APPLICATION_H__

#include "VRV/Export.h"
#include "VRV/Core/BaseApplication.h"
#include "VRV/Core/SharedData.h"
#include "VRV/Core/Navigator.h"
#include "VRV/Devices/Wingman.h"

#include "Minerva/Core/Utilities/Hud.h"
#include "Minerva/Document/MinervaDocument.h"
#include "Minerva/Document/AnimationController.h"

#include "vrj/Draw/OpenGL/ContextData.h"

#include "plugins/ApplicationDataManager/UserData.h"

#include "osg/Referenced"
#include "osg/LightSource"
#include "osg/Matrix"
#include "osg/Timer"

#include <list>
#include <string>
#include <queue>


namespace osg
{
  class FrameStamp;
  class MatrixTransform;
  class Node;
};


namespace VRV {
namespace Core {


class VRV_EXPORT Application : public VRV::Core::BaseApplication
{
public:
  // Typedefs.
  typedef VRV::Core::BaseApplication           BaseClass;
  typedef Usul::Math::Matrix44d                Matrix;
  typedef std::vector < std::string >          Filenames;
  typedef Usul::Interfaces::IUnknown           IUnknown;

  // Constructors.
  Application();

  // Destructor.
  virtual ~Application();

  // Clean up.  Call before the Application is destroyed.
  void                          cleanup();

  // Set the camera.
  virtual void                  resetView();

  /// Get the models node.
  osg::MatrixTransform*              models();
  const osg::MatrixTransform*        models() const;

  // Print the usage string.
  static void             usage ( const std::string &exe, std::ostream &out );

  // View functions.
  void                    viewScene();

  /// Reread the preference files and reinitialiaze.
  void                    reinitialize();

  void nextTimestep();
  void previousTimestep();

  void toggleAnimation();

protected:

  /// VR Juggler methods.
  virtual void                  _init();
  virtual void                  _preFrame();
  virtual void                  _latePreFrame();
  virtual void                  _postFrame();

  double                        _getCurrentTimeInMilliSeconds() const;

  // Load the file(s).
  virtual void                  _loadModelFiles  ( const Filenames& filenames );

  // Set the near and far clipping planes based on the scene.
  void                          _setNearAndFarClippingPlanes();

  /// Update notify.
  void                          _updateNotify();

  /// Navigate.
  virtual void                  _navigate();

  // Parse the command-line arguments.
  void                          _parseCommandLine();

  // Set the current "camera" position as "home".
  void                          _setHome();

  /// Get section for current document.
  std::string                   _documentSection() const;

  // Set/Get the navigation matrix.
  void                          _navigationMatrix ( const osg::Matrixd& m );
  const osg::Matrixd&           _navigationMatrix() const;

  // Notify of rendering.
  void                          _postRenderNotify();

  /// Get the duration of the last frame in seconds.
  double                        frameTime() const;

  ///  VRV::Interfaces::INavigationScene
  /// Get the navigation scene.
  virtual const osg::Group *    navigationScene() const;
  virtual osg::Group *          navigationScene();

  /// VRV::Interfaces::ModelsScene
  /// Get the models scene.
  virtual const osg::Group *    modelsScene() const;
  virtual osg::Group *          modelsScene();

  /// Set the view matrix ( Usul::Interfaces::IViewMatrix ).
  /// Note: In this implementation, the navigation matrix is set.
  virtual void                  setViewMatrix ( const osg::Matrixf& );
  virtual void                  setViewMatrix ( const osg::Matrixd& );

  /// Get the view matrix ( Usul::Interfaces::IViewMatrix ).
  /// Note: In this implementation, the navigation matrix is set.
  virtual osg::Matrixd          getViewMatrix() const;

  Minerva::Document::MinervaDocument  * document();

  /// No copying.
  Application ( const Application& );
  Application& operator = (const Application&);

private:

  // Override these functions from vrj::osg::App.
  virtual void initScene() {}
  virtual osg::Group* getScene() { return _root.get(); }
  virtual void configSceneView ( osgUtil::SceneView* newSceneViewer );

  virtual void            draw();

  // Typedefs.
  typedef osg::ref_ptr <osg::Group>                        GroupPtr;
  typedef osg::ref_ptr <osg::MatrixTransform>              MatTransPtr;
  typedef VRV::Core::SharedData<double>                    SharedDouble;
  typedef VRV::Core::SharedData<osg::Matrixd>              SharedMatrix;

  // Data members.
  GroupPtr                               _root;
  MatTransPtr                            _navBranch;
  MatTransPtr                            _models;
  osg::Timer                             _timer;
  osg::Timer_t                           _initialTime;
  osg::Timer_t                           _frameStart;
  double                                 _frameTime;
  cluster::UserData < SharedDouble >     _sharedFrameTime;
  cluster::UserData < SharedDouble >     _sharedReferenceTime;
  cluster::UserData < SharedMatrix >     _sharedMatrix;
  osg::Matrixd                           _home;
  Minerva::Document::MinervaDocument::RefPtr _document;
  Minerva::Document::AnimationController::RefPtr _animationController;
  VRV::Devices::Wingman::RefPtr _wingMan;
  Navigator::RefPtr _navigator;
  bool _isAnimating;
  double _lastTimeAnimationStep;
  osg::ref_ptr<osg::Camera> _hudCamera;
};

}
}


#endif // __VRV_CORE_APPLICATION_H__
