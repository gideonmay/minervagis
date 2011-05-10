
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Base view class.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Document/View.h"
#include "Minerva/Core/Data/UserData.h"
#include "Minerva/Core/Visitors/FindObject.h"
#include "Minerva/OsgTools/LegendItem.h"

#include "osg/Multisample"

using namespace Minerva::Document;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

View::View ( Document::RefPtr document ) : BaseClass(),
  _document ( document ),
  _hud(),
  _root ( new osg::Group ),
  _osgCamera ( new osg::Camera ),
  _balloon ( 0x0 )
{

  _osgCamera->setName ( "Minerva_osgCamera" );
  _osgCamera->setRenderOrder ( osg::Camera::POST_RENDER );
  _osgCamera->setReferenceFrame ( osg::Camera::ABSOLUTE_RF );
  _osgCamera->setClearMask( GL_DEPTH_BUFFER_BIT );
  _osgCamera->setViewMatrix( osg::Matrix::identity() );
  _osgCamera->setComputeNearFarMode ( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
  _osgCamera->setCullingMode ( osg::CullSettings::NO_CULLING );

  // Turn on multi-sampling.
  osg::ref_ptr<osg::StateSet> ss ( _root->getOrCreateStateSet() );
  osg::ref_ptr<osg::Multisample> multiSample ( new osg::Multisample );

  ss->setAttribute ( multiSample.get(), osg::StateAttribute::ON );

  if ( document.valid() )
  {
    _root->addChild ( document->buildScene() );
  }

  _root->addChild ( _hud.buildScene() );
  _root->addChild ( _osgCamera.get() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

View::~View()
{
  _document = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the document.
//
///////////////////////////////////////////////////////////////////////////////

View::Document::RefPtr View::document() const
{
  return _document;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the scene root.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* View::scene() const
{
  return _root.get();
}

///////////////////////////////////////////////////////////////////////////////
//
//  Set show compass state.
//
///////////////////////////////////////////////////////////////////////////////

void View::showCompass( bool b )
{
  _hud.showCompass ( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get show compass state.
//
///////////////////////////////////////////////////////////////////////////////

bool View::isShowCompass() const
{
  return _hud.showCompass();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get showing of eye altitude feedback state.
//
///////////////////////////////////////////////////////////////////////////////

bool View::isShowEyeAltitude() const
{
  return _hud.showEyeAltitude();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set showing of eye altitude feedback state.
//
///////////////////////////////////////////////////////////////////////////////

void View::showEyeAltitude ( bool b )
{
  _hud.showEyeAltitude ( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get showing of meta-string feedback state.
//
///////////////////////////////////////////////////////////////////////////////

bool View::isShowMetaString() const
{
  return _hud.showMetaString();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set showing of meta-string feedback state.
//
///////////////////////////////////////////////////////////////////////////////

void View::showMetaString ( bool b )
{
  _hud.showMetaString ( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the lat lon text shown?
//
///////////////////////////////////////////////////////////////////////////////

bool View::isShowLatLonText() const
{
  return _hud.showPointerPosition();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle showing of lat lon text.
//
///////////////////////////////////////////////////////////////////////////////

void View::showLatLonText ( bool b )
{
  return _hud.showPointerPosition( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the job feedback text shown?
//
///////////////////////////////////////////////////////////////////////////////

bool View::isShowJobFeedback() const
{
  return _hud.showJobFeedback();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle showing of job feedback text.
//
///////////////////////////////////////////////////////////////////////////////

void View::showJobFeedback ( bool b )
{
  _hud.showJobFeedback( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the date feedback text shown?
//
///////////////////////////////////////////////////////////////////////////////

bool View::isShowDateFeedback() const
{
  return _hud.showDateFeedback();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle showing of date feedback text.
//
///////////////////////////////////////////////////////////////////////////////

void View::showDateFeedback ( bool b )
{
  _hud.showDateFeedback( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Notify the observer of the intersection (IIntersectListener).
//
///////////////////////////////////////////////////////////////////////////////

void View::intersectNotify ( const osgUtil::LineSegmentIntersector::Intersection &hit )
{
  Document::RefPtr document ( this->document() );
  if ( false == document.valid() )
    return;

  Minerva::Core::TileEngine::Body::RefPtr body ( document->body() );
  if ( false == body.valid() )
    return;

  // Call this every time.
  _hud.metaString ( "" );

  // Get the point in lon-lat-elev.
  osg::Vec3 world ( hit.getWorldIntersectPoint() );
  Usul::Math::Vec3d point ( world[0], world[1], world[2] );
  Usul::Math::Vec3d lonLatPoint;
  body->convertFromPlanet ( point, lonLatPoint );

  // Set the heads-up display.
  _hud.position ( lonLatPoint[1], lonLatPoint[0], lonLatPoint[2] );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to get the data object from the node path.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  View::ObjectID findObjectID ( const osg::NodePath& path )
  {
    osg::ref_ptr < Minerva::Core::Data::UserData > userdata ( 0x0 );

    // See if there is user data.
    for( osg::NodePath::const_reverse_iterator iter = path.rbegin(); iter != path.rend(); ++iter )
    {
      osg::ref_ptr<osg::Node> node ( *iter );
      if ( true == node.valid() )
      {
        osg::ref_ptr<osg::Referenced> data ( node->getUserData() );
        if ( true == data.valid() )
        {
          osg::ref_ptr<Minerva::Core::Data::UserData> ud ( dynamic_cast < Minerva::Core::Data::UserData * > ( data.get() ) );
          if ( true == ud.valid() )
          {
            userdata = ud;
            break;
          }
        }
      }
    }

    if ( userdata.valid() )
    {
      return userdata->objectID();
    }

    return "";
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Called when mouse event occurs.
//
///////////////////////////////////////////////////////////////////////////////

void View::displayBalloon ( const osgUtil::LineSegmentIntersector::Intersection &hit )
{
  // This is now causing a crash on a fairly regular basis.  The hit contains a vector of raw Node pointers.
  // My best guess right now is that the osg::Nodes are being deleted in a different thread.
  // How we handle the scene needs to be refactored so that all osg objects are only deleted in the main thread.
#if 0
  // Find the id for the object we intersected.
  ObjectID objectID ( Helper::findObjectID ( hit.nodePath ) );

  if ( false == objectID.empty() )
  {
    // Find the unknown
    Minerva::Core::Data::Feature::RefPtr feature ( this->_findObject ( objectID ) );
    if ( feature.valid() )
    {
      // This should be true, but check to make sure the data object pointer is valid.
      Minerva::Core::Data::DataObject::RefPtr dataObject ( feature->asDataObject() );
      if( dataObject.valid() )
      {
        this->_displayInformationBalloon ( *dataObject );
      }
    }
  }
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find object.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Feature::RefPtr View::_findObject ( const ObjectID& objectID )
{
  if ( _document )
  {
    Minerva::Core::Visitors::FindObject::RefPtr visitor ( new Minerva::Core::Visitors::FindObject ( objectID ) );
    _document->accept ( *visitor );
    return visitor->object();
  }

  return 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear the balloon
//
///////////////////////////////////////////////////////////////////////////////

bool View::_displayInformationBalloon ( Minerva::Core::Data::DataObject& dataObject )
{
  // Remove what we have.
  this->clearBalloon();

  osg::ref_ptr<osg::Camera> camera ( _osgCamera );
  OsgTools::Widgets::Item::RefPtr item ( dataObject.clicked() );

  if ( camera.valid() && item.valid() )
  {
    osg::ref_ptr<osg::Node> balloon ( item->buildScene() );
    if ( true == balloon.valid() )
    {
      camera->addChild ( balloon.get() );
    }

    _balloon = balloon;
    return true;
  }

  return false;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear the balloon
//
///////////////////////////////////////////////////////////////////////////////

void View::clearBalloon()
{
  if ( _osgCamera.valid() && _balloon.valid() )
    _osgCamera->removeChild ( _balloon.get() );

  _balloon = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update the hud.
//
///////////////////////////////////////////////////////////////////////////////

void View::_updateHUD ( Minerva::Core::Data::Camera::RefPtr camera )
{
  if ( camera.valid() )
  {
    _hud.hpr ( camera->heading(), camera->tilt(), camera->roll() );
    _hud.eyeAltitude ( camera->altitude() );
  }

  Document::RefPtr document ( this->document() );
  if ( document )
  {
    unsigned int queued ( 0 );
    Usul::Jobs::Manager::Strings names;
    document->runningJobStats ( queued, names );

    _hud.requests ( queued );
    _hud.running ( names );

    Minerva::Core::Data::TimeSpan::RefPtr visibleTimeSpan ( document->visibleTimeSpan() );
    if ( visibleTimeSpan )
    {
      Minerva::Core::Data::Date firstDate ( visibleTimeSpan->begin() );
      Minerva::Core::Data::Date lastDate ( visibleTimeSpan->end() );

      if ( !firstDate.date().is_special() && !lastDate.date().is_special() )
      {
        _hud.dateFeedback ( lastDate.toString() );
      }
      else
      {
        _hud.dateFeedback ( "" );
      }
    }
    else
    {
      _hud.dateFeedback ( "" );
    }
  }

  _hud.update();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Resize.
//
///////////////////////////////////////////////////////////////////////////////

void View::resize ( unsigned int width, unsigned int height )
{
  _osgCamera->setViewport ( 0, 0, width, height );
  _osgCamera->setProjectionMatrixAsOrtho ( 0, width, 0, height, -100.0, 100.0 );

  _hud.resize ( width, height );
}
