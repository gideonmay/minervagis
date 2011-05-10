
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

#ifndef __MINERVA_DOUCMENT_VEIW_H__
#define __MINERVA_DOUCMENT_VEIW_H__

#include "Minerva/Document/Export.h"
#include "Minerva/Document/MinervaDocument.h"

namespace Minerva {
namespace Document {


class MINERVA_DOCUMENT_EXPORT View : public Usul::Base::Referenced
{
public:

  typedef Usul::Base::Referenced BaseClass;
  typedef Minerva::Document::MinervaDocument Document;
  typedef Minerva::Core::Data::Container::ObjectID ObjectID;

  USUL_DECLARE_REF_POINTERS ( View );

  View ( Document::RefPtr document );

  Document::RefPtr document() const;

  osg::Node* scene() const;

  virtual void resize ( unsigned int width, unsigned int height );

  /// Get/Set show compass state.
  bool                                     isShowCompass() const;
  void                                     showCompass ( bool b );

  /// Toggle showing of lat lon text.
  bool                                     isShowLatLonText() const;
  void                                     showLatLonText ( bool b );

  /// Toggle showing of job feedback.
  bool                                     isShowJobFeedback() const;
  void                                     showJobFeedback ( bool b );

  /// Toggle showing of date feedback.
  bool                                     isShowDateFeedback() const;
  void                                     showDateFeedback ( bool b );

  /// Toggle showing of eye altitude feedback.
  bool                                     isShowEyeAltitude() const;
  void                                     showEyeAltitude ( bool b );

  /// Toggle showing of meta-string feedback.
  bool                                     isShowMetaString() const;
  void                                     showMetaString ( bool b );

  // Notify the observer of the intersection.
  void                                     intersectNotify ( const osgUtil::LineSegmentIntersector::Intersection &hit );

  /// Clear the balloon
  void                                     clearBalloon();

  // Display balloon for interesected object (if any).
  void                                     displayBalloon ( const osgUtil::LineSegmentIntersector::Intersection &hit );

protected:

  virtual ~View();

  void _updateHUD ( Minerva::Core::Data::Camera::RefPtr camera );

private:

  bool                                     _displayInformationBalloon ( Minerva::Core::Data::DataObject& );

  /// Find object.
  Minerva::Core::Data::Feature::RefPtr     _findObject ( const ObjectID& objectID );

  Document::RefPtr _document;
  Minerva::Core::Utilities::Hud _hud;

  osg::ref_ptr<osg::Group>      _root;
  osg::ref_ptr<osg::Camera>     _osgCamera;
  osg::ref_ptr<osg::Node>       _balloon;
};

}
}

#endif // __MINERVA_DOUCMENT_VEIW_H__
