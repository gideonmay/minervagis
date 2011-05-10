
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Qt/Widgets/Favorites.h"

#include "Minerva/Network/Download.h"
#include "Minerva/Core/Data/Container.h"

#include "ui_Favorites.h"

#include "Helios/Menus/Menu.h"
#include "Helios/Menus/Button.h"

#include "Usul/App/Application.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Jobs/Job.h"
#include "Usul/Jobs/Manager.h"
#include "Usul/Registry/Database.h"
#include "Usul/Scope/RemoveFile.h"
#include "Usul/User/Directory.h"

#include "Serialize/XML/Serialize.h"
#include "Serialize/XML/Deserialize.h"

#include "QtGui/QHeaderView"
#include "QtGui/QMenu"

#include "boost/bind.hpp"

using namespace Minerva::QtWidgets;

///////////////////////////////////////////////////////////////////////////////
//
//  Typedefs with file scope.
//
///////////////////////////////////////////////////////////////////////////////

typedef Usul::Threads::Guard<Usul::Threads::Mutex> Guard;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Favorites::Favorites ( QWidget* parent ) : BaseClass( parent ),
  _serverFavorites(),
  _favoritesMap(),
  _downloadJob ( 0x0 ),
  _mutex(),
  _implementation ( new Ui::Favorites ),
  SERIALIZE_XML_INITIALIZER_LIST
{
  _implementation->setupUi ( this );
  
  _implementation->_favoritesTree->header()->setHidden ( 1 );
  
  this->_addMember ( "favorites", _favoritesMap );
  
  // Restore favorites.
  Usul::Functions::safeCall ( boost::bind ( &Favorites::_restoreState, this ) );
  
  // We want a custom context menu.
  _implementation->_favoritesTree->setContextMenuPolicy ( Qt::CustomContextMenu );

  // Notify us when a context menu is requested.
  connect ( _implementation->_favoritesTree, SIGNAL ( customContextMenuRequested ( const QPoint& ) ),
            this,  SLOT   ( _onContextMenuShow ( const QPoint& ) ) );

  // We want extended selection.
  //_favoritesTree->selectionMode ( QAbstractItemView::ExtendedSelection );

  // Read from server.
  _downloadJob = Usul::Jobs::create ( boost::bind ( &Favorites::_readFavoritesFromServer, this ) );
  _downloadJob->name ( "Favorites Download Job" );

  // Add the job to the manager.
  Usul::Jobs::Manager::instance().addJob ( _downloadJob );

  this->_buildTree();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Favorites::~Favorites()
{
  Usul::Functions::safeCall ( boost::bind ( &Favorites::_clear, this ) );
  delete _implementation; _implementation = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::_clear()
{
  // Copy the job pointer.
  Usul::Jobs::Job::RefPtr job ( 0x0 );
  Guard guard ( _mutex );
  {
    job = _downloadJob;
  }

  // Done with the job member.
  _downloadJob = 0x0;

  // Wait for the job.
  if ( true == job.valid() )
  {
    Usul::Jobs::Manager::instance().cancel ( job );
    while ( false == job->isDone() ){}
    job = 0x0;
  }

  // Save favorites.
  Usul::Functions::safeCall ( boost::bind ( &Favorites::_saveState, this ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to check the cancelled state.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  bool isCancelled ( Usul::Jobs::Job::RefPtr &job, Usul::Threads::Mutex &mutex )
  {
    Guard guard ( mutex );

    // Has the job been cancelled in one way or another?
    const bool invalid ( false == job.valid() );
    const bool cancelled ( ( true == job.valid() ) ? job->canceled() : false );
    return ( invalid || cancelled );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to throw if cancelled.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  void checkCancelled ( Usul::Jobs::Job::RefPtr &job, Usul::Threads::Mutex &mutex )
  {
    if ( true == Helper::isCancelled ( job, mutex ) )
    {
      throw Usul::Exceptions::Canceled();
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add layer to the favorites.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::addLayer ( Minerva::Core::Data::Feature* feature )
{
  if( feature )
    _favoritesMap[feature->name()] = feature;
  
  // Build the tree.
  this->_buildTree();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add a layer.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::_addLayer ( Minerva::Core::Data::Feature *parent, Minerva::Core::Data::Feature* unknown )
{
  Minerva::Core::Data::Container::RefPtr container ( parent ? parent->asContainer() : 0x0 );
  
  // Return now if unknown is null.
  if ( 0x0 == unknown || !container )
    return;

  {
    // Clone the "template"
    Minerva::Core::Data::Feature::RefPtr layer ( unknown->clone() );
    
    if( layer.valid() )
    {
      // Make sure that the favorite is shown.
      layer->visibilitySet ( true );
    
      // Add the layer.
      container->add ( layer.get() );
      
      // Emit the layer added signal.
      //emit layerAdded ( unknown );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove the favorite.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::_removeFavoriteButtonClicked()
{
  // Get the current item.
  QTreeWidgetItem * item ( _implementation->_favoritesTree->currentItem() );
  
  // Return if no layer.
  if ( 0x0 == item )
    return;
  
  // Get the name of the favorite.
  std::string name ( item->text( 0 ).toStdString() );
  
  // Remove the item from our map.
  FavoritesMap::iterator iter = _favoritesMap.find( name );
  if( iter != _favoritesMap.end() )
    _favoritesMap.erase ( iter );
  
  // Rebuild the tree.
  this->_buildTree();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add items from map.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  template < class Map >
  inline void addItems ( const Map& map, QTreeWidgetItem* parent )
  {
    for ( typename Map::const_iterator iter = map.begin(); iter != map.end(); ++iter )
    {
      // Make the item.
      QTreeWidgetItem *item ( new QTreeWidgetItem ( parent ) );
      item->setText ( 0, iter->first.c_str() );
      parent->addChild ( item );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the tree.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::_buildTree()
{
  // Clear anything we may have.
  _implementation->_favoritesTree->clear();

  // Make the item.
  QTreeWidgetItem *server ( new QTreeWidgetItem ( _implementation->_favoritesTree ) );
  QTreeWidgetItem *user   ( new QTreeWidgetItem ( _implementation->_favoritesTree ) );

  // Set the text.
  server->setText ( 0, "Server" );
  user->setText   ( 0, "User" );

  // Add the items.
  Helper::addItems ( _serverFavorites, server );
  Helper::addItems ( _favoritesMap, user );
  
  // Add items to tree.
  _implementation->_favoritesTree->addTopLevelItem ( server );
  _implementation->_favoritesTree->addTopLevelItem ( user );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Save State.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::_saveState()
{
   Serialize::XML::serialize ( *this, this->_filename() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Restore State.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::_restoreState()
{
  const std::string file ( this->_filename() );
  if ( true == boost::filesystem::exists ( file ) )
  {
    XmlTree::Document::ValidRefPtr document ( new XmlTree::Document );
    document->load ( file );
    this->deserialize ( *document );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the filename.
//
///////////////////////////////////////////////////////////////////////////////

std::string Favorites::_filename() const
{
  // Get the vendor and program names.
  const std::string vendor ( Usul::App::Application::instance().vendor() );
  const std::string program ( Usul::App::Application::instance().program() );
  
  // Get persistant directory and make sure it exists.
  const std::string persistantDir ( Usul::User::Directory::vendor ( vendor, true ) + program + "/" );
  boost::filesystem::create_directories ( persistantDir );
  
  // Build the filename.
  const std::string filename ( persistantDir + "gis_favorites.xml" );
  
  // Return the file name.
  return filename;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make the menu.
//
///////////////////////////////////////////////////////////////////////////////

Favorites::Menu* Favorites::menu ( Minerva::Core::Data::Feature *parent )
{
  // Make the menu.
  Menu::RefPtr menu ( new Menu );
  menu->append ( this->_buildMenu ( _serverFavorites, "Server", parent ) );
  menu->append ( this->_buildMenu ( _favoritesMap, "User", parent ) );
  return menu.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add to the menu.
//
///////////////////////////////////////////////////////////////////////////////

Helios::Menus::Menu* Favorites::_buildMenu ( const FavoritesMap& map, const std::string& name, Minerva::Core::Data::Feature *parent )
{
  // Shorten the lines.
  typedef Helios::Menus::Menu Menu;
  typedef Helios::Menus::Button Button;

  // Make the menu.
  Menu::RefPtr menu ( new Menu ( name ) );

  // Add buttons to the menu.
  for ( FavoritesMap::const_iterator iter = map.begin(); iter != map.end(); ++iter )
  {
    menu->append ( new Button ( iter->first, "", "", boost::bind ( &Favorites::_addLayer, this, parent, iter->second.get() ) ) );
  }

  return menu.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show the context menu.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::_onContextMenuShow ( const QPoint& pos )
{
  // Get the current item.
  QTreeWidgetItem * item ( _implementation->_favoritesTree->currentItem() );
  
  // Return if no layer.
  if ( 0x0 == item )
    return;

  QMenu menu;

  QAction action ( "Remove", 0x0 );
  connect ( &action, SIGNAL ( triggered() ), this, SLOT ( _removeFavoriteButtonClicked() ) );
  menu.addAction ( &action );

  menu.exec ( _implementation->_favoritesTree->mapToGlobal ( pos ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read from server.
//
///////////////////////////////////////////////////////////////////////////////

void Favorites::_readFavoritesFromServer()
{
  const std::string server ( "www.minerva-gis.org" );
  const std::string file ( "gis_favorites.xml" );
  const std::string url ( "http://" + server + "/" + file );

  // Check for the cancelled state. This matters when the server is slow 
  // and the user shuts down right away (before this job has finished).
  Helper::checkCancelled ( _downloadJob, _mutex );

  // File to download to.
  std::string name ( Usul::File::Temp::file() );
  Usul::Scope::RemoveFile remove ( name );

  Helper::checkCancelled ( _downloadJob, _mutex );

  // Attempt to download the file.
  if ( false == Minerva::Network::downloadToFile ( url, name ) )
    return;

  Helper::checkCancelled ( _downloadJob, _mutex );

  Serialize::XML::DataMemberMap map;
  map.addMember ( "favorites", _serverFavorites );

  XmlTree::Document::ValidRefPtr document ( new XmlTree::Document );
  document->load ( name );

  Helper::checkCancelled ( _downloadJob, _mutex );

  map.deserialize ( *document );
  
  Helper::checkCancelled ( _downloadJob, _mutex );

  // Make sure the tree is rebuilt.
  QMetaObject::invokeMethod ( this, "_buildTree", Qt::QueuedConnection );

  // We're done with this member.
  {
    Guard guard ( _mutex );
    _downloadJob = 0x0;
  }
}
