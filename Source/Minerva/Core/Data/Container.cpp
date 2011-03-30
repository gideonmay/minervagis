
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/Container.h"
#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Visitor.h"

#include "Minerva/Common/IElevationDatabase.h"
#include "Minerva/Common/IPlanetCoordinates.h"

#include "Minerva/OsgTools/Group.h"

#include "Usul/Bits/Bits.h"
#include "Usul/Factory/RegisterCreator.h"
#include "Usul/Threads/Safe.h"

#include "osg/Group"

#include "boost/bind.hpp"

#include <limits>

using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Register creators.
//
///////////////////////////////////////////////////////////////////////////////

USUL_FACTORY_REGISTER_CREATOR_WITH_NAME ( "Container", Container );

USUL_IMPLEMENT_IUNKNOWN_MEMBERS ( Container, Container::BaseClass );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Container::Container() : 
  BaseClass(),
  _layers(),
  _builders(),
  _flags ( Container::ALL ),
  _root ( new osg::Group ),
  _unknownMap(),
  _comments()
{
  this->_registerMembers();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Container::Container( const Container& rhs ) : 
  BaseClass ( rhs ),
  _layers( rhs._layers ),
  _builders ( rhs._builders ),
  _flags ( rhs._flags | Container::SCENE_DIRTY ), // Make sure scene gets rebuilt.
  _root ( new osg::Group ),
  _unknownMap ( rhs._unknownMap ),
  _comments ( rhs._comments )
{
  this->_registerMembers();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Container::~Container()
{
  _layers.clear();
  _builders.clear();
  _unknownMap.clear();
  _comments.clear();
  _root = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Register members for serialization.
//
///////////////////////////////////////////////////////////////////////////////

void Container::_registerMembers()
{
  this->_addMember ( "layers", _layers );
  this->_addMember ( "comments", _comments );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query Interface.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown* Container::queryInterface ( unsigned long iid )
{
  switch ( iid )
  {
  case Usul::Interfaces::IUnknown::IID:
  case Minerva::Common::IElevationChangedListener::IID:
    return static_cast<Minerva::Common::IElevationChangedListener*> ( this );
  case Minerva::Common::ITileVectorData::IID:
    return static_cast<Minerva::Common::ITileVectorData*> ( this );
  case Minerva::Common::IBuildScene::IID:
    return static_cast<Minerva::Common::IBuildScene*> ( this );
  default:
    return 0x0;
  };
}


///////////////////////////////////////////////////////////////////////////////
//
//  Accept the visitor.
//
///////////////////////////////////////////////////////////////////////////////

void Container::accept ( Minerva::Core::Visitor& visitor )
{
  visitor.visit ( *this );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Traverse all Features.
//
///////////////////////////////////////////////////////////////////////////////

void Container::traverse ( Minerva::Core::Visitor& visitor )
{
  //Guard guard ( this );
  Features feature ( Usul::Threads::Safe::get ( this->mutex(), _layers ) );
  for ( Features::iterator iter = feature.begin(); iter != feature.end(); ++iter )
  {
    Feature::RefPtr feature ( *iter );
    if ( feature.valid() )
    {
      feature->accept ( visitor );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the show label.
//
///////////////////////////////////////////////////////////////////////////////

void Container::visibilitySet ( bool b )
{
  // Set the visibility state.
  BaseClass::visibilitySet ( b );

  // The scene needs rebuilt.
  this->dirtyScene ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add a data object.
//
///////////////////////////////////////////////////////////////////////////////

void Container::add ( Feature* feature, bool notify )
{
  if ( 0x0 == feature )
    return;

  {
    Guard guard ( this );
    _layers.push_back ( feature );
    
    _unknownMap.insert ( FeatureMap::value_type ( feature->objectId(), feature ) );
  }

  // Add the builder.
  IBuildScene::QueryPtr buildScene ( feature );
  if ( buildScene.valid() )
  {
    Guard guard ( this );
    _builders.push_back ( buildScene );
  }
  
  // Update the extents.
  this->_updateExtents ( feature );
  
  // Our scene needs rebuilt.
  this->dirtyScene ( true );
  
  // Notify any listeners that the data has changed.
  if ( notify )
    this->_notifyDataChangedListeners();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove a layer.
//
///////////////////////////////////////////////////////////////////////////////

void Container::remove ( Feature *feature )
{
  if ( 0x0 == feature )
    return;

  {
    Guard guard ( this );
    
    {
      Features::iterator doomed ( std::find ( _layers.begin(), _layers.end(), Features::value_type ( feature ) ) );
      if( doomed != _layers.end() )
        _layers.erase( doomed );
    }

    // Remove the builder.
    IBuildScene::QueryPtr buildScene ( feature );
    if ( buildScene.valid() )
    {
      Builders::iterator doomed ( std::find ( _builders.begin(), _builders.end(), Builders::value_type ( buildScene ) ) );
      if( doomed != _builders.end() )
        _builders.erase ( doomed );
    }
    
    // If we can get a GUID, remove the mapping.
    _unknownMap.erase ( feature->objectId() );
  }

  // Our scene needs rebuilt.
  this->dirtyScene ( true );
  
  // Notify any listeners that the data has changed.
  this->_notifyDataChangedListeners();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear data objects.
//
///////////////////////////////////////////////////////////////////////////////

void Container::clear()
{
  Guard guard ( this->mutex() );

  _unknownMap.clear();
  _layers.clear();
  _builders.clear();

  // Our scene needs to be rebuilt.
  this->dirtyScene ( true );
  
  // Notify any listeners that the data has changed.
  this->_notifyDataChangedListeners();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the number of data objects in this layer.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int Container::size() const
{
  Guard guard ( this->mutex() );
  return _layers.size();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the scene.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Container::getScene()
{
  Guard guard ( this->mutex() );
  return _root.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene (IBuildScene).
//
///////////////////////////////////////////////////////////////////////////////

osg::Node * Container::buildScene ( Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation )
{
  return this->getScene();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene.
//
///////////////////////////////////////////////////////////////////////////////

void Container::_buildScene ( Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation )
{
  Guard guard ( this->mutex() );
  
  if ( true == _root.valid() || this->dirtyScene() )
  {
    // For debugging...
    _root->setName ( this->name() );
    
    // Remove all children.
    OsgTools::Group::removeAllChildren ( _root.get() );

    // Add to the scene if we are shown.
    if ( BaseClass::visibility() )
    {
      for ( Builders::iterator iter = _builders.begin(); iter != _builders.end(); ++iter )
      {
        Builders::value_type dataObject ( *iter );

        // Should we build the scene?
        if ( dataObject.valid() )
        {
          // Build the scene. Handle possible null return.
          osg::ref_ptr<osg::Node> node ( dataObject->buildScene ( planet, elevation ) );
          if ( true == node.valid() )
          {
            _root->addChild ( node.get() );
          }
        }
      }
    }
    
    // Our scene is no longer dirty.
    this->dirtyScene ( false );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the flags.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int Container::flags() const
{
  Guard guard ( this->mutex() );
  return _flags;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flags.
//
///////////////////////////////////////////////////////////////////////////////

void Container::flags ( unsigned int f )
{
  Guard guard ( this->mutex() );
  _flags = f;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the data dirty flag.
//
///////////////////////////////////////////////////////////////////////////////

bool Container::dirtyData() const
{
  Guard guard ( this->mutex() );
  return Usul::Bits::has<unsigned int, unsigned int> ( _flags, Container::DATA_DIRTY );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the data dirty flag.
//
///////////////////////////////////////////////////////////////////////////////

void Container::dirtyData ( bool b )
{
  Guard guard ( this->mutex() );
  _flags = Usul::Bits::set<unsigned int, unsigned int> ( _flags, Container::DATA_DIRTY, b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extents dirty flag.
//
///////////////////////////////////////////////////////////////////////////////

bool Container::dirtyExtents() const
{
  Guard guard ( this->mutex() );
  return Usul::Bits::has<unsigned int, unsigned int> ( _flags, Container::EXTENTS_DIRTY );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the extents dirty flag.
//
///////////////////////////////////////////////////////////////////////////////

void Container::dirtyExtents ( bool b )
{
  Guard guard ( this->mutex() );
  _flags = Usul::Bits::set<unsigned int, unsigned int> ( _flags, Container::EXTENTS_DIRTY, b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get dirty scene flag.
//
///////////////////////////////////////////////////////////////////////////////

bool Container::dirtyScene() const
{
  Guard guard ( this->mutex() );
  return Usul::Bits::has<unsigned int, unsigned int> ( _flags, Container::SCENE_DIRTY );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set dirty scene flag.
//
///////////////////////////////////////////////////////////////////////////////

void Container::dirtyScene ( bool b )
{
  Guard guard ( this->mutex() );
  _flags = Usul::Bits::set<unsigned int, unsigned int> ( _flags, Container::SCENE_DIRTY, b );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extents.
//
///////////////////////////////////////////////////////////////////////////////

Container::Extents Container::calculateExtents() const
{
  Guard guard ( this->mutex() );
  
  Extents extents;
  for ( Features::const_iterator iter = _layers.begin(); iter != _layers.end(); ++iter )
  {
    Feature::RefPtr feature ( *iter );
    if ( feature )
    {
      Extents e ( feature->extents() );
      extents.expand ( e );
    }
  }
  
  return extents;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Update.
//
///////////////////////////////////////////////////////////////////////////////

void Container::updateNotify ( CameraState* camera, Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation )
{
  // Build if we need to...
  if ( this->dirtyScene()  )
  {
    this->_buildScene ( planet, elevation );
  }

  // Ask each one to update.
  {
    Guard guard ( this );
    std::for_each ( _layers.begin(), _layers.end(), boost::bind ( &Feature::updateNotify, _1, camera, planet, elevation ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the number of children (ITreeNode).
//
///////////////////////////////////////////////////////////////////////////////

unsigned int Container::getNumChildNodes() const
{
  Guard guard ( this->mutex() );
  return _layers.size();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the child node (ITreeNode).
//
///////////////////////////////////////////////////////////////////////////////

Feature::RefPtr Container::getChildNode ( unsigned int which )
{
  Guard guard ( this->mutex() );
  return _layers.at ( which );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Deserialize.
//
///////////////////////////////////////////////////////////////////////////////

void Container::deserialize ( const XmlTree::Node &node )
{
  Guard guard ( this->mutex() );

  _dataMemberMap.deserialize ( node );

  // Add layers.
  for ( Features::iterator iter = _layers.begin(); iter != _layers.end(); ++iter )
  {
    Feature::RefPtr feature ( *iter );

    // Add the builder.
    IBuildScene::QueryPtr buildScene ( feature );
    if ( buildScene.valid() )
    {
      _builders.push_back ( buildScene );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Elevation has changed within given extents (IElevationChangeListener).
//
///////////////////////////////////////////////////////////////////////////////

bool Container::elevationChangedNotify ( const Extents& extents, unsigned int level, ElevationDataPtr elevationData, Usul::Interfaces::IUnknown * caller )
{
  bool handled ( false );
  
  Features features ( Usul::Threads::Safe::get ( this->mutex(), _layers ) );
  {
    for ( Features::iterator iter = features.begin(); iter != features.end(); ++iter )
    {
      Minerva::Common::IElevationChangedListener::QueryPtr ecl ( *iter );
      if ( ecl.valid() )
      {
        if ( ecl->elevationChangedNotify ( extents, level, elevationData, caller ) )
        {
          this->dirtyScene ( true );
          handled = true;
        }
      }
    }
  }
  
  return handled;
}


///////////////////////////////////////////////////////////////////////////////
//
//  See if the given level falls within this layer's range of levels.
//
///////////////////////////////////////////////////////////////////////////////

bool Container::isInLevelRange ( unsigned int level ) const
{
  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Launch the jobs to fetch vector data.
//
///////////////////////////////////////////////////////////////////////////////

Container::TileVectorJobs Container::launchVectorJobs ( double minLon, 
                                                        double minLat, 
                                                        double maxLon, 
                                                        double maxLat, 
                                                        unsigned int level, 
                                                        Usul::Jobs::Manager *manager, 
                                                        Usul::Interfaces::IUnknown::RefPtr caller )
{
  TileVectorJobs answer;

  Features unknowns ( Usul::Threads::Safe::get ( this->mutex(), _layers ) );
  {
    for ( Features::iterator i = unknowns.begin(); i != unknowns.end(); ++i )
    {
      Minerva::Common::ITileVectorData::QueryPtr tileVectorData ( *i );
      if ( true == tileVectorData.valid() )
      {
        TileVectorJobs jobs ( tileVectorData->launchVectorJobs ( minLon, minLat, maxLon, maxLat, level, manager, caller ) );
        answer.insert ( answer.end(), jobs.begin(), jobs.end() );
      }
    }
  }

  return answer;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find unknown with given id.  The function will return null if not found.
//
///////////////////////////////////////////////////////////////////////////////

Feature::RefPtr Container::find ( const ObjectID& id ) const
{
  Guard guard ( this->mutex() );
  FeatureMap::const_iterator iter ( _unknownMap.find ( id ) );
  if ( iter != _unknownMap.end() )
  {
    return iter->second;
  }
  
  return 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the items within the extents.
//
///////////////////////////////////////////////////////////////////////////////

Feature::RefPtr Container::getItemsWithinExtents ( double minLon, double minLat, double maxLon, double maxLat, IUnknown::RefPtr caller ) const
{
  // Initialize.
  Container::RefPtr answer ( new Container );
  Extents givenExtents ( minLon, minLat, maxLon, maxLat );

  // Get a copy of the layers.
  const Features layers ( Usul::Threads::Safe::get ( this->mutex(), _layers ) );

  // Loop through the layers.
  for ( Features::const_iterator i = layers.begin(); i != layers.end(); ++i )
  {
    // Does this layer contain items?
    const Minerva::Common::IWithinExtents::QueryPtr w ( i->get() );
    if ( true == w.valid() )
    {
      Feature::RefPtr contained ( w->getItemsWithinExtents ( minLon, minLat, maxLon, maxLat, caller ) );
      if ( true == contained.valid() )
      {
        answer->add ( contained.get(), false );
      }
    }
    else
    {
      Feature::RefPtr feature ( *i );
      if ( feature )
      {
        // Calculate the center.
        const Extents layerExtents ( feature->extents() );
        const Extents::Vertex center ( layerExtents.center() );

        // Is the center in the extents?
        if ( true == givenExtents.contains ( center ) )
        {
          answer->add ( feature.get(), false );
        }
      }
    }
  }

  // Return the answer.
  return ( answer->size() > 0 ) ? answer.get() : 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Swap features.
//
///////////////////////////////////////////////////////////////////////////////

void Container::swap ( Feature* layer0, Feature* layer1 )
{
  {
    Guard guard ( this->mutex() );
    
    Features::iterator iter0 ( std::find_if ( _layers.begin(), 
                                              _layers.end(), 
                                              Feature::RefPtr::IsEqual ( layer0 ) ) );
    
    Features::iterator iter1 ( std::find_if ( _layers.begin(), 
                                              _layers.end(), 
                                              Feature::RefPtr::IsEqual ( layer1 ) ) );
    
    if ( _layers.end() != iter0 && _layers.end() != iter1 )
    {
      Feature::RefPtr temp ( *iter0 );
      *iter0 = *iter1;
      *iter1 = temp;
    }
  }
  
  this->_notifyDataChangedListeners();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the feature.
//
///////////////////////////////////////////////////////////////////////////////

Feature::RefPtr Container::feature ( unsigned int i )
{
  Guard guard ( this->mutex() );
  return _layers.at ( i );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Reserve enough room.
//
///////////////////////////////////////////////////////////////////////////////

void Container::reserve ( unsigned int size )
{
  Guard guard ( this->mutex() );
  _layers.reserve ( size );
  _builders.reserve ( size );
}
