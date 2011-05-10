
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Document/MinervaDocument.h"
#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Factory/Readers.h"
#include "Minerva/Core/Functions/MakeBody.h"
#include "Minerva/Core/Visitors/TemporalAnimation.h"
#include "Minerva/Core/Visitors/FindMinMaxDates.h"
#include "Minerva/Core/Visitors/StackPoints.h"
#include "Minerva/Common/Extents.h"

#include "Minerva/OsgTools/ConvertVector.h"
#include "Minerva/OsgTools/ConvertMatrix.h"
#include "Minerva/OsgTools/Group.h"

#include "Serialize/XML/Serialize.h"
#include "Serialize/XML/Deserialize.h"

#include "Usul/Bits/Bits.h"
#include "Usul/File/Path.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Jobs/Manager.h"
#include "Usul/Registry/Constants.h"
#include "Usul/Registry/Database.h"
#include "Usul/Strings/Case.h"
#include "Usul/Scope/CurrentDirectory.h"
#include "Usul/Threads/Safe.h"

#include "osg/Geode"

#include "osgUtil/IntersectVisitor"

#include "boost/bind.hpp"

#include <algorithm>
#include <limits>
#include <sstream>
#include <vector>

using namespace Minerva::Document;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

MinervaDocument::MinervaDocument ( LogPtr log, const std::string &typeName ) :
  BaseClass ( typeName ),
  _dirty ( false ),
  _body ( 0x0 ),
  _manager ( 0x0 ),
  _timeSpanOfData ( new TimeSpan ),
  _visibleTimeSpan ( new TimeSpan ),
  _allowSplit ( true ),
  _keepDetail ( false ),
  _log ( log ),
  _paths(),
  SERIALIZE_XML_INITIALIZER_LIST
{
  // Serialization glue.
  this->_addMember ( "body", _body );
  this->_addMember ( "paths", _paths );

  // Make the body.
  this->_makePlanet();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

MinervaDocument::~MinervaDocument()
{
  Usul::Functions::safeCall ( boost::bind ( &MinervaDocument::_destroy, this ), "1582439358" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return true if this document can do it.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::canExport ( const std::string &file ) const
{
  const std::string ext ( Usul::Strings::lowerCase ( Usul::File::extension ( file ) ) );
  return ( ext == ".minerva" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return true if this document can do it.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::canInsert ( const std::string &file ) const
{
  return false;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return true if this document can do it.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::canOpen ( const std::string &file ) const
{
  const std::string ext ( Usul::Strings::lowerCase ( Usul::File::extension ( file ) ) );
  return ( ext == ".minerva" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return true if this document can do it.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::canSave ( const std::string &file ) const
{
  const std::string ext ( Usul::Strings::lowerCase ( Usul::File::extension ( file ) ) );
  return ( ext == ".minerva" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the filters that correspond to what this document can export.
//
///////////////////////////////////////////////////////////////////////////////

MinervaDocument::Filters MinervaDocument::filtersExport() const
{
  Filters filters;
  filters.push_back ( Filter ( "Minerva (*.minerva)", "*.minerva" ) );
  return filters;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the filters that correspond to what this document can insert.
//
///////////////////////////////////////////////////////////////////////////////

MinervaDocument::Filters MinervaDocument::filtersInsert() const
{
  Filters filters;
  return filters;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the filters that correspond to what this document can open.
//
//////////////////////////////////////////////////////////////////////////////

MinervaDocument::Filters MinervaDocument::filtersOpen()   const
{
  Filters filters;
  filters.push_back ( Filter ( "Minerva (*.minerva)", "*.minerva" ) );
  return filters;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the filters that correspond to what this document can save.
//
///////////////////////////////////////////////////////////////////////////////

MinervaDocument::Filters MinervaDocument::filtersSave()   const
{
  Filters filters;
  filters.push_back ( Filter ( "Minerva (*.minerva)", "*.minerva" ) );
  return filters;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read the file and add it to existing document's data.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::read ( const std::string &filename, Unknown *caller, Unknown *progress )
{
  const std::string ext ( Usul::Strings::lowerCase ( Usul::File::extension ( filename ) ) );

  if ( ".minerva" == ext )
  {
    // Deserialize the xml tree.
    XmlTree::Document::ValidRefPtr document ( new XmlTree::Document );
    document->load ( filename );

    // Change the current working directory to where the file lives.
    {
      Usul::Scope::CurrentDirectory cwd ( Usul::File::directory ( filename ) );
      this->deserialize ( *document );
    }

    if ( _body.valid() )
    {
      // Set the job manager for each body.
      _body->jobManager ( this->_getJobManager() );
    }

  }

  // Reset all the log pointers.
  this->_setLog();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Write the document to given file name. Does not rename this document.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::write ( const std::string &filename, Unknown *caller, Unknown *progress ) const
{
  const std::string ext ( Usul::Strings::lowerCase ( Usul::File::extension ( filename ) ) );

  if( ".minerva" == ext )
  {
    Serialize::XML::serialize ( *this, filename );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear any existing data.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::clear ( Unknown *caller )
{
  Usul::Functions::safeCall ( boost::bind ( &MinervaDocument::_clear, this ), "2171542707" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::_clear()
{
  Guard guard ( this );

  // Clear the bodies.
  if ( _body )
  {
    _body->clear();
  }

  // Clean up job manager.
  if ( 0x0 != _manager )
  {
    // Remove all queued jobs and cancel running jobs.
    _manager->cancel();

    // Wait for remaining jobs to finish.
    _manager->wait();

    // Delete the manager.
    delete _manager; _manager = 0x0;
  }

  // Clear the bodies.
  _body = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destroy this instance.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::_destroy()
{
  Usul::Functions::safeCall ( boost::bind ( &MinervaDocument::_clear, this ), "3972867384" );

  // Set other members.
  _body = 0x0;
  _timeSpanOfData = 0x0;
  _log = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node * MinervaDocument::buildScene()
{
  Guard guard ( this );

  // Make sure we have at least one body.
  this->_makePlanet();

  Body::RefPtr body ( this->body() );
  return body.valid() ? body->scene() : 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Deserialze.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::deserialize ( const XmlTree::Node &node )
{
  _dataMemberMap.deserialize ( node );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Notification that a renderer just rendered.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::postRenderNotify()
{
  Body::RefPtr body ( this->body() );
  if ( body.valid() )
  {
    // Remove all tiles that are ready.
    body->purgeTiles();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::updateNotify ( Minerva::Core::Data::Camera::RefPtr camera )
{
  Guard guard ( this );

  if ( this->dirty() )
  {
    Minerva::Core::Visitors::StackPoints::RefPtr visitor ( new Minerva::Core::Visitors::StackPoints );
    this->accept ( *visitor );
  }

  Body::RefPtr body ( this->body() );
  if ( body )
  {
    body->updateNotify ( camera.get() );
  }

  this->dirty ( false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the dirty scene flag.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::dirtyScene() const
{
  Guard guard ( this );
  return _dirty;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Dirty the scene.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::dirtyScene ( bool b, Minerva::Core::Data::Feature *feature )
{
  Guard guard ( this );

  _dirty = b;

  // Get the active body.
  Body::RefPtr body ( this->body() );
  
  if ( !body )
    return;

  Minerva::Core::Layers::RasterLayer::RefPtr raster ( dynamic_cast<Minerva::Core::Layers::RasterLayer*> ( feature ) );
  
  if ( raster.valid() )
  {
    body->rasterChanged ( raster.get() );
  }
  else
  {
    body->dirtyTextures ( Minerva::Common::Extents ( -180, -90, 180, 90 ) );
  }

  // This is causing the document to prompt about saving even though 
  // no changes have been made.
  //this->modified ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Have visitor visit all layes.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::accept ( Minerva::Core::Visitor& visitor )
{
  Body::RefPtr body ( this->body() );
  if ( body.valid() )
  {
    body->accept ( visitor );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find first and last date.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::_findFirstLastDate()
{
  Guard guard ( this );

  Minerva::Core::Visitors::FindMinMaxDates::RefPtr findMinMax ( new Minerva::Core::Visitors::FindMinMaxDates );
  this->accept ( *findMinMax );

  TimeSpan::RefPtr global ( new TimeSpan );

  global->begin ( findMinMax->first() );
  global->end ( findMinMax->last() );

  // Set the current to the new global.
  _timeSpanOfData = global;

  this->visibleTimeSpan ( new TimeSpan ( global->begin(), global->begin() ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Increase split distance.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::_increaseSplitDistance()
{
  Body::RefPtr body ( this->body() );
  if ( body.valid() )
    body->splitDistance ( body->splitDistance() * 1.1 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Decrease split distance.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::_decreaseSplitDistance()
{
  Body::RefPtr body ( this->body() );
  if ( body.valid() )
    body->splitDistance ( body->splitDistance() / 1.1 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make the planet.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::_makePlanet()
{
  // Only make it once.
  {
    Guard guard ( this );
    if ( _body.valid() )
      return;
  }

  // Make the earth.
  Body::RefPtr body ( Minerva::Core::Functions::makeEarth ( this->_getJobManager() ) );

  {
    Guard guard ( this );
    _body = body;
  }

  // Set the log.
  this->_setLog();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the scene dirty?
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::dirty() const
{
  Guard guard ( this );
  return _dirty;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the dirty flag
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::dirty ( bool b )
{
  Guard guard ( this );
  _dirty = b;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set use skirts state.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::isShowSkirts() const
{
  Body::RefPtr body ( this->body() );
  return ( body.valid() ? body->useSkirts() : false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle the skirts on and off.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::showSkirts ( bool b )
{
  Body::RefPtr body ( this->body() );
  if ( body.valid() )
    body->useSkirts ( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set use borders state.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::isShowBorders() const
{
  Body::RefPtr body ( this->body() );
  return ( body.valid() ? body->useBorders() : false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Toggle the borders on and off.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::showBorders ( bool b )
{
  Body::RefPtr body ( this->body() );
  if ( body.valid() )
    body->useBorders ( b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the active body.
//
///////////////////////////////////////////////////////////////////////////////

MinervaDocument::Body::RefPtr MinervaDocument::body() const
{
  Guard guard ( this );
  return _body;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the job manager.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Jobs::Manager * MinervaDocument::_getJobManager()
{
  Guard guard ( this );

  // Only make it once.
  if ( 0x0 == _manager )
  {
    typedef Usul::Registry::Database Reg;
    namespace Sections = Usul::Registry::Sections;

    const std::string type ( Reg::instance().convertToTag ( this->typeName() ) );
    Usul::Registry::Node &node ( Reg::instance()[Sections::DOCUMENT_SETTINGS][type]["job_manager_thread_pool_size"] );
    const unsigned int poolSize ( node.get<unsigned int> ( 5, true ) );

    const std::string name ( Usul::Strings::format ( "Minerva ", this ) );
    std::cout << Usul::Strings::format ( name, " thread pool size = ", poolSize, '\n' ) << std::flush;

    _manager = new Usul::Jobs::Manager ( name, poolSize );
    _manager->logSet ( Usul::Jobs::Manager::instance().logGet() );
  }

  return _manager;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the freeze split flag.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::isAllowSplit() const
{
  Guard guard ( this );
  return _allowSplit;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the freeze split flag.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::allowSplit( bool b )
{
  Guard guard ( this );
  _allowSplit = b;

  {
    Body::RefPtr body ( this->body() );
    if ( body.valid() )
      body->allowSplit ( _allowSplit );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the keep detail flag.
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::isKeepDetail() const
{
  Guard guard ( this );
  return _keepDetail;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the keep detail flag.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::keepDetail( bool b )
{
  Guard guard ( this );
  _keepDetail = b;

  {
    Body::RefPtr body ( this->body() );
    if ( body.valid() )
      body->keepDetail ( _keepDetail );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Reset all the logs.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::_setLog()
{
  // Get the log.
  LogPtr logFile ( Usul::Threads::Safe::get ( this->mutex(), _log ) );

  Body::RefPtr body ( this->body() );
  if ( body.valid() )
  {
    // Set the log.
    body->logSet ( Usul::Interfaces::ILog::QueryPtr ( logFile ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Are we busy?
//
///////////////////////////////////////////////////////////////////////////////

bool MinervaDocument::busyStateGet() const
{
  Guard guard ( this );

  MinervaDocument *me ( const_cast < MinervaDocument * > ( this ) );
  if ( 0x0 == me )
    return false;

  const unsigned int queued    ( me->_getJobManager()->numJobsQueued() );
  const unsigned int executing ( me->_getJobManager()->numJobsExecuting() );
  const bool idle ( ( 0 == queued ) && ( 0 == executing ) );

  return ( false == idle );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the timespan of the data.
//
///////////////////////////////////////////////////////////////////////////////

MinervaDocument::TimeSpan::RefPtr MinervaDocument::timeSpanOfData()
{
  this->_findFirstLastDate();
  return _timeSpanOfData;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the visible time span.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::visibleTimeSpan ( TimeSpan::RefPtr timeSpan )
{
  _visibleTimeSpan = timeSpan;

  if ( _visibleTimeSpan.valid() )
  {
    // Set the dates to show.
    Minerva::Core::Visitors::TemporalAnimation::RefPtr visitor ( new Minerva::Core::Visitors::TemporalAnimation ( _visibleTimeSpan->begin(), _visibleTimeSpan->end() ) );
    this->accept ( *visitor );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  SGt the visible time span.
//
///////////////////////////////////////////////////////////////////////////////

MinervaDocument::TimeSpan::RefPtr MinervaDocument::visibleTimeSpan() const
{
  return _visibleTimeSpan;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create and append a path.
//
///////////////////////////////////////////////////////////////////////////////

CameraPath::RefPtr MinervaDocument::createAndAppendNewPath()
{
  CameraPath::RefPtr path ( new CameraPath );

  Guard guard ( this );
  path->name ( Usul::Strings::format ( "Path ", _paths.size() ) );
  _paths.push_back ( path );
  return path;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the number of paths.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int MinervaDocument::numberOfPaths() const
{
  Guard guard ( this );
  return _paths.size();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the given path.
//
///////////////////////////////////////////////////////////////////////////////

CameraPath::RefPtr MinervaDocument::getPath ( unsigned int index )
{
  Guard guard ( this );
  return ( index < _paths.size() ? _paths[index] : 0x0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get stats on the running jobs.
//
///////////////////////////////////////////////////////////////////////////////

void MinervaDocument::runningJobStats ( unsigned int &queued, Usul::Jobs::Manager::Strings& names )
{
  Usul::Jobs::Manager *manager ( this->_getJobManager() );
  queued = ( 0x0 == manager ) ? 0 : manager->numJobsQueued();

  if ( 0x0 != manager )
    manager->executingNames ( names );
}
