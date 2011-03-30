
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_PLUGINS_LAYERS_TREE_H__
#define __MINERVA_PLUGINS_LAYERS_TREE_H__

#include "Minerva/Qt/Widgets/Export.h"
#include "Minerva/Core/Data/Feature.h"

#include "Usul/Interfaces/IUnknown.h"

#include "QtGui/QWidget"

#include <map>

class QSlider;

namespace Minerva {
namespace QtWidgets {
  
  class TreeControl;
  class TreeNode;
  class Favorites;
  
class MINERVA_QT_WIDGETS_EXPORT LayersTree : public QWidget
{
  Q_OBJECT;
public:
  typedef QWidget                        BaseClass;
  typedef Usul::Interfaces::IUnknown     IUnknown;
  typedef Minerva::Core::Data::Feature   Feature;

  LayersTree ( QWidget *parent = 0x0 );
  virtual ~LayersTree ();

  void        buildTree ( Minerva::Core::Data::Feature * feature );

  void        clear();

  /// Set/get the favorites.
  void        favorites ( Favorites* );
  Favorites*  favorites() const;

signals:

  void        documentModified();
  void        addLayerFavorites ( Minerva::Core::Data::Feature *feature );
  void        layerDoubleClicked ( Minerva::Core::Data::Feature *feature );
  void        layerDirty ( Minerva::Core::Data::Feature *feature );

protected:
  bool        _canMoveLayerUp ( TreeNode *item ) const;
  bool        _canMoveLayerDown ( TreeNode *item ) const;
  bool        _canRefreshLayer ( Feature *unknown ) const;
  void        _connectTreeViewSlots ();
  void        _addLayer ( Feature *parent );
  void        _editLayerProperties ( Feature *unknown, Feature* parent, IUnknown *editor );
  void        _editDataObjectProperties ( Feature* feature, Feature* parent );
  void        _dirtyAndRedraw ( Feature *unknown );
  IUnknown*   _findEditor ( Feature* unknown );
  void        _moveLayerUp ( TreeNode *item );
  void        _moveLayerDown ( TreeNode *item );
  void        _refreshLayer ( Feature *unknown );
  void        _removeSelectedLayers();
  void        _swapLayers ( TreeNode *item0, TreeNode *item1, TreeNode *parent );

protected slots:
  void        _onTreeNodeChanged ( TreeNode * node );
  void        _onDoubleClick ( TreeNode * node );
  void        _onContextMenuShow ( const QPoint& pos );
  void        _onAddLayerFavorites();
  void        _onSliderReleased();
  void        _onItemSelectionChanged();

private:

  void _documentIsModified();
  
  TreeControl *_tree;
  QSlider *_slider;
  Minerva::Core::Data::Feature::RefPtr _feature;
  Favorites *_favorites;
};


}
}

#endif // __MINERVA_PLUGINS_LAYERS_TREE_H__
