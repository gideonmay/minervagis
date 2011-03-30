
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Base class for all data objects.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/UserData.h"
#include "Minerva/Core/Visitor.h"

#include "Minerva/OsgTools/ConvertVector.h"
#include "Minerva/OsgTools/Font.h"
#include "Minerva/OsgTools/Legend.h"
#include "Minerva/OsgTools/LegendObject.h"
#include "Minerva/OsgTools/LegendText.h"

#include "Minerva/Common/IElevationDatabase.h"
#include "Minerva/Common/IPlanetCoordinates.h"

#include "Usul/Math/Vector3.h"
#include "Usul/Threads/Safe.h"

#include "osg/Depth"
#include "osg/Geode"
#include "osg/Group"
#include "osg/MatrixTransform"
#include "osgText/Text"

#include <limits>

const Usul::Math::Vec4f DEFAULT_LABEL_COLOR ( 1.0, 1.0, 1.0, 1.0 );
const float DEFAULT_LABEL_SIZE ( 25.0f );


///////////////////////////////////////////////////////////////////////////////
//
//  Callback to cull text that is not visible to the eye.
//
///////////////////////////////////////////////////////////////////////////////

struct TextCullCallback : public osg::Drawable::CullCallback
{
  typedef osg::Drawable::CullCallback BaseClass;

  TextCullCallback ( const osg::Vec3d& normal ) : BaseClass(), _normal ( normal )
  {
    _normal.normalize();
  }

  virtual bool cull ( osg::NodeVisitor* nv, osg::Drawable* /*drawable*/, osg::RenderInfo* /*renderInfo*/ ) const
  {
    // Calculate the dot product of the eye and the normal.
    // If less than zero, the normal is pointing away from the eye.
    return nv && nv->getEyePoint() * _normal <= 0;
  }

  private:
    osg::Vec3d _normal;
};


using namespace Minerva::Core::Data;

USUL_IMPLEMENT_IUNKNOWN_MEMBERS ( DataObject, DataObject::BaseClass );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

DataObject::DataObject() :
  BaseClass(),
  _dirty ( true ),
  _label(),
  _showLabel ( false ),
  _geometry ( 0x0 ),
  _clickedCallback ( 0x0 ),
  _style ( 0x0 )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

DataObject::~DataObject()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query for interface.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown* DataObject::queryInterface ( unsigned long iid )
{
  switch ( iid )
  {
  case Usul::Interfaces::IUnknown::IID:
  case Minerva::Common::IElevationChangedListener::IID:
    return static_cast<Minerva::Common::IElevationChangedListener*> ( this );
  case Minerva::Common::IWithinExtents::IID:
    return static_cast<Minerva::Common::IWithinExtents*> ( this );
  case Minerva::Common::IBuildScene::IID:
    return static_cast<Minerva::Common::IBuildScene*> ( this );
  default:
    return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Accept the visitor.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::accept ( Minerva::Core::Visitor& visitor )
{
  visitor.visit ( *this );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add a geometry.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::geometry ( Geometry::RefPtr geometry )
{
  Guard guard ( this );
  _geometry = geometry;

  if ( true == geometry.valid() )
  {
    this->extents ( geometry->extents() );
    
    this->dirty ( true );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the dirty flag.
//
///////////////////////////////////////////////////////////////////////////////

bool DataObject::dirty() const
{
  Guard guard ( this );
  return _dirty;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the dirty flag.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::dirty ( bool b )
{
  Guard guard ( this );
  _dirty = b;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the label.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::label ( const std::string& label )
{
  Guard guard ( this );
  _label = label;
  this->dirty( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the label.
//
///////////////////////////////////////////////////////////////////////////////

const std::string& DataObject::label() const
{
  Guard guard ( this );
  return _label;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the geometry.
//
///////////////////////////////////////////////////////////////////////////////

Geometry::RefPtr DataObject::geometry() const
{
  Guard guard ( this );
  return _geometry;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the node for the label.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* DataObject::_buildLabel ( const PositionType& position )
{
  osg::ref_ptr<osg::MatrixTransform> mt ( new osg::MatrixTransform );
  
  const std::string label ( this->label() );

  if ( this->showLabel() && false == label.empty() )
  {
    osg::ref_ptr < osg::Geode > geode ( new osg::Geode );

    Usul::Math::Vec4f color ( DEFAULT_LABEL_COLOR );
    float scale ( 1.0 );

    Minerva::Core::Data::Style::RefPtr style ( this->style() );
    if ( style && style->labelstyle() )
    {
      color = style->labelstyle()->color();
      scale = style->labelstyle()->scale();
    }

    osg::ref_ptr < osgText::Text > text ( new osgText::Text );
    text->setFont ( OsgTools::Font::defaultFont() );
    text->setColor ( Usul::Convert::Type<Usul::Math::Vec4f,osg::Vec4f>::convert ( color ) );
    text->setPosition ( osg::Vec3f ( 0.0, 0.0, 0.0 ) );
    text->setAutoRotateToScreen( true );
    text->setCharacterSizeMode( osgText::Text::SCREEN_COORDS );
    text->setCharacterSize( DEFAULT_LABEL_SIZE * scale );

    text->setAlignment ( osgText::TextBase::CENTER_CENTER );

    // Always pass the depth test.
    text->getOrCreateStateSet()->setAttributeAndModes ( new osg::Depth(osg::Depth::ALWAYS), osg::StateAttribute::ON );

    // Draw last.
    text->getOrCreateStateSet()->setRenderBinDetails ( std::numeric_limits<int>::max(), "RenderBin" );

    text->setText ( this->label() );

    geode->addDrawable( text.get() );

    mt->setMatrix ( osg::Matrixd::translate ( Usul::Convert::Type<PositionType,osg::Vec3d>::convert ( position ) ) );
    mt->addChild ( geode.get() );

    osg::Vec3d normal ( position[0], position[1], position[2] );
    normal.normalize();
    text->setCullCallback ( new TextCullCallback ( normal ) );
  }

  return mt.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag to show the label.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::showLabel ( bool value )
{
  Guard guard ( this );
  _showLabel = value;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the flag to show the label.
//
///////////////////////////////////////////////////////////////////////////////

bool DataObject::showLabel () const
{
  Guard guard ( this );
  return _showLabel;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::preBuildScene ( Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation )
{
  osg::ref_ptr<osg::Group> group ( new osg::Group );
  
  Extents extents;

  Geometry::RefPtr geometry ( this->geometry() );
  if ( true == geometry.valid() )
  {
    // Build the scene for the geometry.
    osg::ref_ptr<osg::Node> node ( geometry->buildScene ( this->style(), planet, elevation ) );

    // Set the extents to the geometry's extents.
    // This needs to be called after the scene is built because the extents may be updated in Geometry::buildScene.
    extents = geometry->extents();

    if ( node.valid() )
    {
      // Add user data.
      node->setUserData ( new Minerva::Core::Data::UserData ( this->objectId() ) );

      group->addChild ( node.get() );
    }
  }

  // Set our extents.
  this->extents ( extents );

  // Do we have a label?
  if ( this->showLabel() && !this->label().empty() )
  {
    Usul::Math::Vec3d p ( extents.center()[0], extents.center()[1], 0.0 );

    if ( planet )
    {
      planet->convertToPlanet ( Usul::Math::Vec3d ( p ), p );
    }

    group->addChild ( this->_buildLabel ( PositionType ( p[0], p[1], p[2] ) ) );
  }

  Guard guard ( this );
  _preBuiltScene = group.get();

  this->dirty ( false );  
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* DataObject::buildScene ( Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation )
{
  // Build the scene if we need to.
  if ( this->dirty() )
  {
    this->preBuildScene ( planet, elevation );
  }

  Guard guard ( this );

  // Switch the pre built scene for what we hand back to osg.
  // This is safer in a multi-threaded environment.
  if( _preBuiltScene.valid() )
  {
    // Get the visibilty state.
    const bool visible ( BaseClass::visibility() );
    
    _root = _preBuiltScene;
    _preBuiltScene = 0x0;

    // Set the visibility state.
    this->visibilitySet ( visible );
  }

  return _root.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the visibilty flag.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::visibilitySet ( bool b )
{
  BaseClass::visibilitySet ( b );
 
  Guard guard ( this );

  if ( _root.valid () )
  {
    const unsigned int nodeMask ( b ? 0xffffffff : 0x0 );
    _root->setNodeMask ( nodeMask );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  DataObject has been clicked.
//
///////////////////////////////////////////////////////////////////////////////

OsgTools::Widgets::Item* DataObject::clicked ( Usul::Interfaces::IUnknown* caller ) const
{
  ClickedCallback::RefPtr cb ( this->clickedCallback() );
  
  // Use the callback if we have one.
  if ( cb.valid() )
    return (*cb)( *this, caller );
  
  OsgTools::Widgets::Legend::RefPtr legend ( new OsgTools::Widgets::Legend );
  legend->maximiumSize ( 300, 300 );
  legend->position ( 10, 10 );
  legend->growDirection ( OsgTools::Widgets::Legend::GROW_DIRECTION_UP );
  
  OsgTools::Widgets::LegendObject::RefPtr row0 ( new OsgTools::Widgets::LegendObject );
  
  // Make some text.
  OsgTools::Widgets::Text::RefPtr text0 ( new OsgTools::Widgets::Text );
  text0->text ( this->name() );
  text0->wrapLine ( false );
  text0->autoSize ( false );
  text0->alignmentVertical ( OsgTools::Widgets::Text::TOP );
  text0->fontSize ( 15 );
  
  // Add the items.
  row0->addItem ( text0.get() );
  
  // Set the percentage of the row.
  row0->percentage ( 0 ) = 1.00;
  
  const std::string description ( this->description() );
  if ( false == description.empty() )
  {
    OsgTools::Widgets::LegendObject::RefPtr row1 ( new OsgTools::Widgets::LegendObject );
    
    // Make some text.
    OsgTools::Widgets::Text::RefPtr text ( new OsgTools::Widgets::Text );
    text->text ( description );
    text->wrapLine ( true );
    text->alignmentVertical ( OsgTools::Widgets::Text::TOP );
    text->fontSize ( 15 );
    
    // Add the items.
    row1->addItem ( text.get() );
    
    // Set the percentage of the row.
    row1->percentage ( 0 ) = 1.00;
    
    legend->addRow ( row1.get() );
  }
  
  legend->addRow ( row0.get() );
  
  return legend.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Elevation has changed within given extents (IElevationChangedListnerer).
//
///////////////////////////////////////////////////////////////////////////////

bool DataObject::elevationChangedNotify ( const Extents& extents, unsigned int level, ElevationDataPtr elevationData, Unknown * caller )
{
  Extents e ( this->extents() );

  if ( e.intersects ( extents ) )
  {
    Geometry::RefPtr geometry ( this->geometry() );
    if ( geometry )
    {
      Minerva::Common::IElevationDatabase::QueryPtr elevation ( caller );
      Minerva::Common::IPlanetCoordinates::QueryPtr planet ( caller );
      geometry->elevationChangedNotify ( extents, level, elevationData, planet.get(), elevation.get() );
    }
  }

  return false;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the clicked callback.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::clickedCallback ( ClickedCallback::RefPtr cb )
{
  Guard guard ( this->mutex() );
  _clickedCallback = cb;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the clicked callback.
//
///////////////////////////////////////////////////////////////////////////////

ClickedCallback::RefPtr DataObject::clickedCallback() const
{
  Guard guard ( this->mutex() );
  return _clickedCallback;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

ClickedCallback::ClickedCallback()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

ClickedCallback::~ClickedCallback()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the pointer to this (IDataObject).
//
///////////////////////////////////////////////////////////////////////////////

DataObject* DataObject::dataObject()
{
  return this;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the data object empty?
//
///////////////////////////////////////////////////////////////////////////////

bool DataObject::empty() const
{
  Guard guard ( this );
  return false == _geometry.valid();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the items within the extents.
//
///////////////////////////////////////////////////////////////////////////////

Feature::RefPtr DataObject::getItemsWithinExtents ( double minLon, double minLat, double maxLon, double maxLat, IUnknown::RefPtr caller ) const
{
  // Initialize.
  DataObject::RefPtr answer ( new DataObject );
  Extents givenExtents ( minLon, minLat, maxLon, maxLat );

  Geometry::RefPtr geometry ( this->geometry() );
  if ( geometry )
  {
    // Calculate the center.
    const Extents layerExtents ( geometry->extents() );
    const Extents::Vertex center ( layerExtents.center() );

    // Is the center in the extents?
    if ( true == givenExtents.contains ( center ) )
    {
      answer->geometry ( geometry );
    }
  }

  // Return the answer.
  return ( ( false == answer->empty() ) ? answer.get() : 0x0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the style.
//
///////////////////////////////////////////////////////////////////////////////

void DataObject::style ( Style::RefPtr style )
{
  Guard guard ( this->mutex() );
  _style = style;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the style.
//
///////////////////////////////////////////////////////////////////////////////

Style::RefPtr DataObject::style() const
{
  Guard guard ( this->mutex() );
  return _style;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get or create the style.
//
///////////////////////////////////////////////////////////////////////////////

Style::RefPtr DataObject::getOrCreateStyle()
{
  Guard guard ( this->mutex() );
  if ( false == _style.valid() )
    _style = new Style;

  return _style;
}
