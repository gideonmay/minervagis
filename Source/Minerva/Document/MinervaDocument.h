
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_DOCUMENT_H__
#define __MINERVA_DOCUMENT_H__

#include "Minerva/Config.h"
#include "Minerva/Document/Export.h"
#include "Minerva/Document/CameraPath.h"

#include "Usul/Documents/Document.h"

#include "Minerva/Core/Data/TimeSpan.h"
#include "Minerva/Core/Data/Container.h"
#include "Minerva/Core/TileEngine/Body.h"
#include "Minerva/Core/Utilities/Hud.h"
#include "Minerva/Core/Navigator.h"

#include "Serialize/XML/Macros.h"

#include "Usul/File/Log.h"
#include "Usul/Jobs/Job.h"

#include "osg/Camera"
#include "osgText/Text"

#include "osgUtil/LineSegmentIntersector"

namespace Minerva { namespace Core { class Visitor; } }
namespace Minerva { namespace Core { namespace Data { class DataObject; } } }

namespace Minerva {
namespace Document {

class MINERVA_DOCUMENT_EXPORT MinervaDocument : public Usul::Documents::Document
{
public:
  
  /// Useful typedefs.
  typedef Usul::Documents::Document BaseClass;
  typedef Minerva::Core::Data::TimeSpan TimeSpan;
  typedef Minerva::Core::Data::Container::ObjectID ObjectID;
  typedef Minerva::Core::TileEngine::Body Body;
  typedef Usul::File::Log::RefPtr LogPtr;
  typedef std::vector < CameraPath::RefPtr > Paths;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( MinervaDocument );

  MinervaDocument ( 
    LogPtr log = LogPtr ( 0x0 ),
    const std::string &typeName = "Minerva Document" );

  /// Get the body.
  Body::RefPtr                             body() const;

  void                                     postRenderNotify();

  /// Build the scene.
  osg::Node *                              buildScene();

  /// Return true if this document can do it.
  virtual bool                             canExport ( const std::string &ext ) const;
  virtual bool                             canInsert ( const std::string &ext ) const;
  virtual bool                             canOpen   ( const std::string &ext ) const;
  virtual bool                             canSave   ( const std::string &ext ) const;

  /// Get the filters that correspond to what this document can do.
  virtual Filters                          filtersExport() const;
  virtual Filters                          filtersInsert() const;
  virtual Filters                          filtersOpen()   const;
  virtual Filters                          filtersSave()   const;

  /// Read the file and add it to existing document's data.
  virtual void                             read ( const std::string &filename, Unknown *caller = 0x0, Unknown *progress = 0x0 );

  /// Write the document to given file name. Does not rename this document.
  virtual void                             write ( const std::string &filename, Unknown *caller = 0x0, Unknown *progress = 0x0  ) const;

  /// Clear any existing data.
  virtual void                             clear ( Unknown *caller = 0x0 );

  /// Update.
  virtual void                             updateNotify ( Minerva::Core::Data::Camera::RefPtr camera );

  /// Get the busy state.
  bool                                     busyStateGet() const;

  /// Have visitor visit all layes.
  void                                     accept ( Minerva::Core::Visitor& visitor );

  /// Get/Set the dirty flag.
  void                                     dirty ( bool );
  bool                                     dirty() const;
  
  /// Dirty the scene.
  bool                                     dirtyScene() const;
  void                                     dirtyScene ( bool b, Minerva::Core::Data::Feature *feature );
  
  /// Toggle the skirts on and off.
  bool                                     isShowSkirts() const;
  void                                     showSkirts ( bool b );
  
  /// Toggle the borders on and off.
  bool                                     isShowBorders() const;
  void                                     showBorders ( bool b );
  
  /// Toggle the freeze split flag.
  bool                                     isAllowSplit() const;
  void                                     allowSplit( bool b );

  /// Toggle the keep detail.
  bool                                     isKeepDetail() const;
  void                                     keepDetail ( bool b );

  TimeSpan::RefPtr                         timeSpanOfData();

  void                                     visibleTimeSpan ( TimeSpan::RefPtr timeSpan );
  TimeSpan::RefPtr                         visibleTimeSpan() const;

  // Path functions.
  CameraPath::RefPtr createAndAppendNewPath();
  unsigned int       numberOfPaths() const;
  CameraPath::RefPtr getPath ( unsigned int index );

  void runningJobStats ( unsigned int &queued, Usul::Jobs::Manager::Strings& names );

protected:

  virtual ~MinervaDocument();

  MinervaDocument( const MinervaDocument& );
  MinervaDocument& operator=( const MinervaDocument& );

  /// Clear.
  void                                     _clear();

  /// Destroy.
  void                                     _destroy();

  /// Find first and last date.
  void                                     _findFirstLastDate();
  
  /// Make the planet.
  void                                     _makePlanet();

  /// Increase/Decrease split distance.
  void                                     _increaseSplitDistance();
  void                                     _decreaseSplitDistance();
  
  /// Get the job manager.
  Usul::Jobs::Manager *                    _getJobManager();

  /// Set all the children's log.
  void                                     _setLog();

private:
  
  bool _dirty;

  Body::RefPtr               _body;
  Usul::Jobs::Manager *      _manager;
  
  TimeSpan::RefPtr _timeSpanOfData;
  TimeSpan::RefPtr _visibleTimeSpan;

  bool _allowSplit;
  bool _keepDetail;
  LogPtr _log;

  Paths _paths;

  SERIALIZE_XML_DEFINE_MAP;
  SERIALIZE_XML_CLASS_NAME( MinervaDocument );
  SERIALIZE_XML_SERIALIZE_FUNCTION;
  virtual void deserialize ( const XmlTree::Node &node );
  SERIALIZE_XML_ADD_MEMBER_FUNCTION;
};


}
}

#endif // __MINERVA_DOCUMENT_H__
