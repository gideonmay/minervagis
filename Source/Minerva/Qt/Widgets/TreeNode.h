
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class to encapsulate ITreeNode.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __QT_TOOLS_TREE_NODE_H__
#define __QT_TOOLS_TREE_NODE_H__

#include "Minerva/Qt/Widgets/Export.h"

#include "Minerva/Core/Data/Feature.h"

#include "QtCore/QList"
#include "QtCore/QString"
#include "QtCore/QVariant"

namespace Minerva {
namespace QtWidgets {

  class TreeModel;
  
class MINERVA_QT_WIDGETS_EXPORT TreeNode : 
  public QObject
{
  Q_OBJECT;
public:
  typedef QObject BaseClass;
  typedef QList<TreeNode*> TreeNodeList;
  typedef Minerva::Core::Data::Feature Feature;
  
  TreeNode ( Feature *feature, TreeNode* parent = 0x0 );
  ~TreeNode();
  
  // Get the i'th child.
  TreeNode*         child ( int i );
  const TreeNode*   child ( int i ) const;
  
  // Get the count.
  int               count() const;
  int               childCount() const;
  
  // Set/get the check state.
  void              checkState ( Qt::CheckState state );
  Qt::CheckState    checkState() const;
  
  // Get the index of node.
  int               index ( TreeNode* ) const;
  int               indexOfChild ( TreeNode* ) const;
  
  // Can the item be checked?
  bool              isCheckable() const;
  
  // Get the name.
  QString           name() const;
  
  // Get the node.
  Feature::RefPtr   node() const;
  
  // Get the parent.
  TreeNode*         parent() const;
  
  // Removes the child at index and returns it.
  TreeNode*         takeChild ( int index );
  
  // Swap locations of children.
  void              swap ( int index0, int index1 );
  
protected slots:
  
  void              _onDataChanged();
  
private:
  
  // Add children to this node.
  void              _addChildren();
  
  // Clear this node.
  void              _clear();
  
  // Rebuild the tree.
  void              _rebuildTree();
  
  // Called when data has changed.
  void              dataChangedNotify();
  
  friend class TreeModel;
  
  /// Set/get the data.
  bool              setData ( const QVariant& value, int role );
  QVariant          data    ( int role ) const;
  
  /// Set/get the model.
  void              model ( TreeModel* );
  TreeModel*        model();
  
  TreeModel* _model;
  TreeNode*  _parent;
  Feature::RefPtr _feature;
  Feature::Connection _connection;
  TreeNodeList _children;
};


}
}

#endif  // __QT_TOOLS_TREE_NODE_H__
