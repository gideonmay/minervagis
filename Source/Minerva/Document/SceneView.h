
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  Copyright (c) 2005, Perry L. Miller IV and Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  View class that wraps osgUtil::SceneView.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OSG_TOOLS_RENDER_VIEWER_H__
#define __OSG_TOOLS_RENDER_VIEWER_H__

#include "Minerva/Document/Export.h"
#include "Minerva/Document/View.h"
#include "Minerva/OsgTools/FrameDump.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"

#include "osgUtil/SceneView"
#include "osgUtil/LineSegmentIntersector"

#include "osg/ref_ptr"
#include "osg/PolygonMode"
#include "osg/Timer"

#include <string>


namespace Minerva {
namespace Document {


class MINERVA_DOCUMENT_EXPORT SceneView : public View
{
public:

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( SceneView );

  // Useful typedefs.
  typedef View BaseClass;
  typedef View::Document Document;
  typedef osg::Node Node;
  typedef osg::PolygonMode PolygonMode;
  typedef Minerva::OsgTools::FrameDump FrameDump;

  // Construction
  SceneView ( Document::RefPtr document );

  /// Set/get back face culling.
  void                  showBackFaces ( bool b );
  bool                  isBackFacesShowing() const;

  /// Set/get near far ratio.
  void                  nearFarRatio ( double value );
  double                nearFarRatio() const;

  // Clear
  void                  clear();

  // Intersect with the scene (ISceneIntersect).
  bool                  intersect ( float x, float y, osgUtil::LineSegmentIntersector::Intersections & );

  /// Set properties for dumping frames.
  void                  frameDumpProperties ( const std::string &dir,
                                              const std::string &base,
                                              const std::string &ext,
                                              unsigned int start = 0,
                                              unsigned int digits = 10 );

  /// Set frame-dump state.
  void                  setFrameDumpState ( FrameDump::DumpState );

  /// Get frame-dump state
  FrameDump::DumpState  getFrameDumpState() const;

  // Set/query/remove the polygon mode.
  void                  setPolygonMode    ( osg::PolygonMode::Mode mode );
  void                  togglePolygonMode ( osg::PolygonMode::Mode mode );
  bool                  hasPolygonMode    ( osg::PolygonMode::Mode mode ) const;
  void                  removePolygonMode();

  // Render the scene.
  void                  render ( Minerva::Core::Data::Camera::RefPtr );

  // Resize the viewer
  void                  resize ( unsigned int width, unsigned int height );

  // Set/Get two sided lighting
  void                  twoSidedLightingSet ( bool twoSided );
  bool                  twoSidedLightingGet() const;

  // Write the current frame to an image file.
  bool                  writeImageFile ( const std::string &filename ) const;

  /// Set/get the view matrix.
  void                  setViewMatrix ( const osg::Matrixd& );
  osg::Matrixd          getViewMatrix() const;

  osg::Matrixd          getProjectionMatrix() const;

  // Capture the screen.
  osg::Image*           screenCapture ( float frameSizeScale, unsigned int numSamples );
  
protected:

  // Use reference counting.
  virtual ~SceneView();

  void                  _dumpFrame();

  // Set/get the frame-dump data.
  void                  frameDump ( const FrameDump &fd ) { _frameDump = fd; }
  const FrameDump &     frameDump() const { return _frameDump; }
  FrameDump &           frameDump()       { return _frameDump; }

  bool                  _intersect ( float x, float y, osg::Node *scene, osgUtil::LineSegmentIntersector::Intersections &intersections );

  bool                  _lineSegment ( float mouseX, float mouseY, osg::Vec3d &pt0, osg::Vec3d &pt1 );

  double                _width() const;
  double                _height() const;

  // Get the viewer.
  const osgUtil::SceneView *     viewer() const;
  osgUtil::SceneView *           viewer();
  
private:

  void _initSceneView();

  // Do not use.
  SceneView ( const SceneView & );
  SceneView &operator = ( const SceneView & );

  osg::ref_ptr<osgUtil::SceneView> _sceneView;
  osg::ref_ptr<osg::FrameStamp> _framestamp;
  osg::Timer _timer;
  osg::Timer_t _start_tick;
  FrameDump _frameDump;
  unsigned int _contextId;
};

}
}

#endif // __OSG_TOOLS_RENDER_VIEWER_H__
