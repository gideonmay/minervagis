
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Qt/Widgets/LayersTree.h"
#include "Minerva/Qt/Widgets/Favorites.h"
#include "Minerva/Qt/Widgets/TreeControl.h"
#include "Minerva/Qt/Widgets/TreeNode.h"
#include "Minerva/Qt/Widgets/EditDataObjectDialog.h"

#include "Minerva/Qt/Tools/ScopedSignals.h"

#include "Minerva/Core/Data/Container.h"
#include "Minerva/Core/Layers/RasterLayer.h"

#include "Minerva/Common/IRefreshData.h"
#include "Minerva/Common/ILayerAddGUIQt.h"
#include "Minerva/Common/ILayerModifyGUIQt.h"

#include "Helios/Menus/Action.h"
#include "Helios/Menus/Menu.h"
#include "Helios/Menus/MenuAdapter.h"

#include "Usul/Components/Manager.h"
#include "Usul/Interfaces/Qt/IMainWindow.h"

#include "QtGui/QVBoxLayout"
#include "QtGui/QHBoxLayout"
#include "QtGui/QPushButton"
#include "QtGui/QTabWidget"
#include "QtGui/QDialog"
#include "QtGui/QMainWindow"
#include "QtGui/QMenu"
#include "QtGui/QMessageBox"
#include "QtGui/QSlider"

#include "boost/bind.hpp"

using namespace Minerva::QtWidgets;

namespace Detail
{
  const unsigned int SLIDER_STEPS ( 1000 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Typedefs.
//
///////////////////////////////////////////////////////////////////////////////

typedef Usul::Components::Manager PluginManager;
typedef PluginManager::UnknownSet Unknowns;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

LayersTree::LayersTree ( QWidget * parent ) : 
  BaseClass ( parent ),
  _tree ( new TreeControl ( parent ) ),
  _slider ( new QSlider ( Qt::Horizontal ) ),
  _feature(),
  _favorites ( 0x0 )
{
  QVBoxLayout *topLayout ( new QVBoxLayout ( parent ) );
  this->setLayout ( topLayout );

  QHBoxLayout *buttonLayout ( new QHBoxLayout );
  QVBoxLayout *treeLayout ( new QVBoxLayout );
  
  // We want a custom context menu.
  _tree->setContextMenuPolicy ( Qt::CustomContextMenu );
  
  // We want exteneded selection.
  _tree->setSelectionMode ( QAbstractItemView::ExtendedSelection );
  
  // Add the tree to the layout.
  treeLayout->addWidget ( _tree );
  
  topLayout->addLayout ( buttonLayout );
  topLayout->addLayout ( treeLayout );
  topLayout->addWidget ( _slider );

  _slider->setRange ( 0, Detail::SLIDER_STEPS );
  _slider->setEnabled ( false );

  // Connect signals and slots for TreeControl.
  QObject::connect ( _tree, SIGNAL ( onTreeNodeChanged ( TreeNode * ) ), this, SLOT ( _onTreeNodeChanged ( TreeNode * ) ) );
  QObject::connect ( _tree, SIGNAL ( onSelectionChanged() ),  this, SLOT ( _onItemSelectionChanged() ) );
  QObject::connect ( _tree, SIGNAL ( onTreeNodeDoubleClicked( TreeNode * ) ), this, SLOT ( _onDoubleClick ( TreeNode * ) ) );
  QObject::connect ( _tree, SIGNAL ( customContextMenuRequested ( const QPoint& ) ), this,  SLOT   ( _onContextMenuShow ( const QPoint& ) ) );
  
  // Connect the slider.
  QObject::connect ( _slider,     SIGNAL ( sliderReleased() ), SLOT ( _onSliderReleased() ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

LayersTree::~LayersTree()
{
  this->clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear the members.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::clear()
{
  _feature = static_cast<Minerva::Core::Data::Feature*> ( 0x0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Item has been double clicked.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_onDoubleClick ( TreeNode * node )
{
  emit layerDoubleClicked ( 0x0 != node ? node->node().get() : 0x0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Item has been changed.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_onTreeNodeChanged ( TreeNode * node )
{
  Feature::RefPtr feature ( 0x0 != node ? node->node().get() : 0x0 );
  this->_dirtyAndRedraw ( feature );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the tree.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::buildTree ( Minerva::Core::Data::Feature * feature )
{
  if ( 0x0 != feature )
    _tree->setRootNode ( new TreeNode ( feature ) );
  
  // Save the document;
  _feature = feature;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the favorites.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::favorites ( Favorites* favorites )
{
  _favorites = favorites;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the favorites.
//
///////////////////////////////////////////////////////////////////////////////

Favorites* LayersTree::favorites() const
{
  return _favorites;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Dirty and request a redraw.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_dirtyAndRedraw ( Feature *feature )
{
  emit layerDirty ( feature );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add a layer.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_addLayer ( Feature *parent )
{
  Unknowns unknowns ( PluginManager::instance().getInterfaces ( Minerva::Common::ILayerAddGUIQt::IID ) );
  
  QDialog dialog ( this );
  QTabWidget *tabs ( new QTabWidget ( &dialog ) );
  QPushButton *ok ( new QPushButton ( "Ok" ) );
  QPushButton *cancel ( new QPushButton ( "Cancel" ) );
  
  connect ( ok,     SIGNAL ( clicked () ), &dialog, SLOT ( accept () ) );
  connect ( cancel, SIGNAL ( clicked () ), &dialog, SLOT ( reject () ) );
  
  QVBoxLayout *topLayout ( new QVBoxLayout );
  dialog.setLayout ( topLayout );
  
  QVBoxLayout *vLayout ( new QVBoxLayout );
  QHBoxLayout *hLayout ( new QHBoxLayout );
  
  topLayout->addLayout ( vLayout );
  topLayout->addLayout ( hLayout );
  
  vLayout->addWidget ( tabs );
  hLayout->addStretch();
  hLayout->addWidget ( ok );
  hLayout->addWidget ( cancel );
  
  for ( Unknowns::iterator iter = unknowns.begin (); iter != unknowns.end(); ++iter )
  {
    Minerva::Common::ILayerAddGUIQt::QueryPtr gui ( (*iter).get() );
    tabs->addTab ( gui->layerAddGUI(), gui->name ().c_str() );
  }
  
  dialog.setModal ( true );
  
  if ( QDialog::Accepted == dialog.exec() )
  {
    for ( Unknowns::iterator iter = unknowns.begin(); iter != unknowns.end(); ++iter )
    {
      Minerva::Common::ILayerAddGUIQt::QueryPtr gui ( (*iter).get() );
      gui->apply ( parent, boost::bind ( &LayersTree::_documentIsModified, this ) );
    }

    this->_documentIsModified();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove selected layers.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_removeSelectedLayers()
{
  if ( 0x0 == _tree )
    return;
    
  // Get all selected items.
  typedef TreeControl::TreeNodeList Items;
  Items items ( _tree->selectedItems() );
  
  if ( items.size() > 0 )
  {
    std::string message ( Usul::Strings::format ( "Delete ", items.size(), " items?" ) );
    if ( QMessageBox::Ok != QMessageBox::question ( this, "Confirm", message.c_str(), QMessageBox::Ok | QMessageBox::Cancel ) )
      return;
  }
  
  // Loop through all items.
  for ( Items::iterator iter = items.begin(); iter != items.end(); ++iter )
  {
    // Get the item.
    TreeNode *node ( *iter );
    
    if ( 0x0 != node )
    {
      // Get the unknown for the item.
      Feature::RefPtr feature ( node->node().get() );
      Feature::RefPtr parent ( 0x0 != node->parent() ? node->parent()->node().get() : 0x0 );
      Minerva::Core::Data::Container::RefPtr container ( parent ? parent->asContainer() : 0x0 );
    
      if ( feature.valid () && container.valid() )
      {
        container->remove ( feature.get() );
      
        _tree->removeNode ( node );
        this->_dirtyAndRedraw ( feature.get() );
      }
    }
  }

  this->_documentIsModified();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show the context menu.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_onContextMenuShow ( const QPoint& pos )
{
  if ( 0x0 == _tree )
    return;
  
  TreeNode *currentItem ( _tree->currentNode() );
  
  if ( 0x0 == currentItem )
    return;
  
  TreeNode *parentItem ( currentItem->parent() );
  
  Feature::RefPtr unknown ( currentItem->node().get() );
  Feature::RefPtr parent ( 0x0 != parentItem ? parentItem->node().get() : 0x0 );
  bool hasParent ( parent && parent->asContainer() );

  QMenu menu;
  
  // Add button.
  Helios::Menus::Action add ( "Add...", boost::bind ( &LayersTree::_addLayer, this, unknown.get() ) );
  add.setEnabled ( hasParent );
  
  // Remove button.
  Helios::Menus::Action remove ( "Remove", boost::bind ( &LayersTree::_removeSelectedLayers, this ) );
  remove.setText ( "Remove" );
  remove.setToolTip ( "Remove selected layers." );
  
  QAction favorites ( 0x0 );
  favorites.setText( "Add to favorites" );
  favorites.setToolTip( "Add layer to favorites" );
  favorites.setEnabled ( unknown.valid() );
  QObject::connect ( &favorites, SIGNAL ( triggered() ), this, SLOT ( _onAddLayerFavorites() ) );
  
  // Move up and down actions.
  Helios::Menus::Action moveUp   ( "Move up", boost::bind ( &LayersTree::_moveLayerUp, this, currentItem ) );
  Helios::Menus::Action moveDown ( "Move down", boost::bind ( &LayersTree::_moveLayerDown, this, currentItem ) );
  moveUp.setEnabled   ( this->_canMoveLayerUp   ( currentItem ) );
  moveDown.setEnabled ( this->_canMoveLayerDown ( currentItem ) );
  
  // Add refresh button.
  Helios::Menus::Action refresh ( "Refresh", boost::bind ( &LayersTree::_refreshLayer, this, unknown.get() ) );
  
  // Editor for this layer.
  Usul::Interfaces::IUnknown::QueryPtr editor ( this->_findEditor ( unknown.get() ) );
  
  // Properties button.
  Helios::Menus::Action properties ( "Properties...", boost::bind ( &LayersTree::_editLayerProperties, this, unknown.get(), parent.get(), editor.get() ) );
  properties.setToolTip ( tr ( "Show the property dialog for this layer" ) );
  properties.setEnabled ( unknown.valid() );
  
  // Add the actions to the menu.
  menu.addAction ( &add );
  menu.addAction ( &remove );
  menu.addAction ( &moveUp );
  menu.addAction ( &moveDown );
  
  // Add the refresh button if we can.
  if ( this->_canRefreshLayer ( unknown.get() ) )
    menu.addAction( &refresh );
  
  menu.addAction ( &favorites );
  
  Helios::Menus::MenuAdapter addFromFavorites ( "Add From Favorites" );
  
  if ( 0x0 != this->favorites() && hasParent )
  {
    Helios::Menus::Menu::RefPtr subMenu ( this->favorites()->menu ( unknown.get() ) );
    addFromFavorites.menu ( subMenu );
    menu.addMenu ( &addFromFavorites );
  }
  
  menu.addAction ( &properties );
  
  menu.exec ( _tree->mapToGlobal ( pos ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find an editor for the unknown.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown* LayersTree::_findEditor ( Feature* unknown )
{
  // Attempt to find an editor.
  Unknowns unknowns ( PluginManager::instance().getInterfaces ( Minerva::Common::ILayerModifyGUIQt::IID ) );
  for ( Unknowns::iterator iter = unknowns.begin (); iter != unknowns.end(); ++iter )
  {
    Minerva::Common::ILayerModifyGUIQt::QueryPtr gui ( (*iter).get() );
    if ( gui->handle ( unknown ) )
    {
      return gui.get();
    }
  }
  
  return 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add the current layer to the favorites.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_onAddLayerFavorites()
{
  TreeNode* node ( 0x0 != _tree ? _tree->currentNode() : 0x0 );
  Feature::RefPtr unknown ( 0x0 != node ? node->node().get() : 0x0 );
  emit addLayerFavorites ( unknown.get() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Edit layer properties.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_editLayerProperties ( Feature *feature, Feature *parent, IUnknown *editor )
{  
  Minerva::Common::ILayerModifyGUIQt::QueryPtr gui ( editor );
  
  // Make sure we have a valid editor.
  if ( gui.valid() && gui->handle ( feature ) )
  {
    gui->showModifyGUI ( feature, parent, 0x0 ); // 3rd argument was _document.
    
#if 0
    //TreeNode *item ( _tree->currentNode() );
    //item->setText( 0, layer->name().c_str() );
#endif
  }
  else if ( Minerva::Core::Data::DataObject* object = feature->asDataObject() )
  {
    this->_editDataObjectProperties ( feature, parent );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Edit data object properties.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_editDataObjectProperties ( Feature* feature, Feature* parent )
{
  Minerva::Core::Data::DataObject::RefPtr object ( 0x0 != feature ? feature->asDataObject() : 0x0 );

  if ( object )
  {
    EditDataObjectDialog dialog ( this );
    dialog.populate ( object );

    if ( dialog.exec() )
    {
      dialog.applyChanges ( object.get() );

      Minerva::Core::Data::Container::RefPtr container ( 0x0 != parent ? parent->asContainer() : 0x0 );
      if ( container )
      {
        container->dirtyScene();
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  The slider has been released.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_onSliderReleased()
{
  TreeNode* node ( 0x0 != _tree ? _tree->currentNode() : 0x0 );
  Minerva::Core::Data::Feature::RefPtr feature ( 0x0 != node ? node->node().get() : 0x0 );
  Minerva::Core::Layers::RasterLayer::RefPtr raster ( dynamic_cast<Minerva::Core::Layers::RasterLayer*> ( feature.get() ) );
  
  if ( raster.valid() )
  {
    const float alpha ( static_cast<float> ( _slider->value() ) / Detail::SLIDER_STEPS );
    raster->alpha ( alpha );
    this->_dirtyAndRedraw ( feature.get() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  The selection has changed.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_onItemSelectionChanged()
{
  TreeNode* node ( 0x0 != _tree ? _tree->currentNode() : 0x0 );
  Minerva::Core::Data::Feature::RefPtr feature ( 0x0 != node ? node->node().get() : 0x0 );
  Minerva::Core::Layers::RasterLayer::RefPtr raster ( dynamic_cast<Minerva::Core::Layers::RasterLayer*> ( feature.get() ) );

  _slider->setEnabled ( raster.valid() );
  
  if ( raster.valid() )
    _slider->setValue ( static_cast<int> ( raster->alpha() * Detail::SLIDER_STEPS ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Move the given layer up.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_moveLayerUp ( TreeNode *item )
{
  if ( 0x0 == item || 0x0 == _tree )
    return;
  
  // Get the parent and the sibling.
  TreeNode *parent ( item->parent() );
  
  if ( 0x0 == parent )
    return;
  
  int index ( parent->indexOfChild ( item ) ); --index;
  TreeNode *sibling ( parent->child ( index ) );
  
  this->_swapLayers( item, sibling, parent );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Move the given layer down.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_moveLayerDown ( TreeNode *item )
{
  if ( 0x0 == item || 0x0 == _tree )
    return;
  
  // Get the parent and the sibling.
  TreeNode *parent ( item->parent() );
  
  if ( 0x0 == parent )
    return;
  
  int index ( parent->indexOfChild ( item ) ); ++index;
  TreeNode *sibling ( parent->child ( index ) );
  
  this->_swapLayers( item, sibling, parent );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Swap two layers.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_swapLayers ( TreeNode *item0, TreeNode *item1, TreeNode *parent )
{
  // Check input.
  if ( 0x0 == item0 || 0x0 == item1 || 0x0 == parent || 0x0 == _tree )
    return;

  Minerva::Core::Data::Container::RefPtr container ( parent->node() ? parent->node()->asContainer() : 0x0 );
  Feature::RefPtr unknown0 ( item0->node().get() );
  Feature::RefPtr unknown1 ( item1->node().get() );

  if ( container.valid() && unknown0.valid() && unknown1.valid() )
  {
    container->swap ( unknown0, unknown1 );

    const int index0 ( parent->indexOfChild ( item0 ) );
    const int index1 ( parent->indexOfChild ( item1 ) );

    parent->swap ( index0, index1 );

    this->_dirtyAndRedraw ( unknown0 );
    this->_dirtyAndRedraw ( unknown1 );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Can the layer be moved up?
//
///////////////////////////////////////////////////////////////////////////////

bool LayersTree::_canMoveLayerUp ( TreeNode *item ) const
{
  if ( 0x0 == item || 0x0 == _tree )
    return false;
  
  // Get the parent.
  TreeNode *parent ( item->parent() );
  
  Feature::RefPtr feature ( parent ? parent->node() : 0x0 );
  const bool isContainer ( feature ? 0x0 != feature->asContainer() : false );
  const int index ( 0x0 != parent ? parent->indexOfChild ( item ) : -1 );
  return isContainer && index > 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Can the layer be moved down?
//
///////////////////////////////////////////////////////////////////////////////

bool LayersTree::_canMoveLayerDown ( TreeNode *item ) const
{
  if ( 0x0 == item || 0x0 == _tree )
    return false;
  
  // Get the parent.
  TreeNode *parent ( item->parent() );
  
  Feature::RefPtr feature ( parent ? parent->node() : 0x0 );
  const bool isContainer ( feature ? 0x0 != feature->asContainer() : false );
  const int index ( 0x0 != parent ? parent->indexOfChild ( item ) : -1 );
  return isContainer && ( index + 1 ) < parent->childCount();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Can the layer be refreshed?
//
///////////////////////////////////////////////////////////////////////////////

bool LayersTree::_canRefreshLayer ( Feature *unknown ) const
{
  Minerva::Common::IRefreshData::QueryPtr rd ( unknown );
  return rd.valid();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Refresh the layer.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_refreshLayer ( Feature *unknown )
{
  Minerva::Common::IRefreshData::QueryPtr rd ( unknown );
  if ( rd.valid() )
    rd->refreshData();
  
  // Force a render.
  this->_dirtyAndRedraw ( unknown );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The document should be modified.
//
///////////////////////////////////////////////////////////////////////////////

void LayersTree::_documentIsModified()
{
  emit documentModified();
}
