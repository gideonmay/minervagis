
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __EDIT_WMS_LAYER_WIDGET_H__
#define __EDIT_WMS_LAYER_WIDGET_H__

#include "Minerva/Qt/Widgets/Export.h"

#include "Minerva/Core/Layers/RasterLayerWms.h"

#include "QtGui/QWidget"

namespace Ui { class EditWmsLayerWidget; }

namespace Minerva {
namespace QtWidgets {


class MINERVA_QT_WIDGETS_EXPORT EditWmsLayerWidget : public QWidget
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;
  typedef Minerva::Core::Layers::RasterLayerNetwork RasterLayerNetwork;
  
  EditWmsLayerWidget ( RasterLayerNetwork *layer, QWidget * parent = 0x0 );
  virtual ~EditWmsLayerWidget();
  
protected slots:
  void _nameFinishedEditing();
  void _serverFinishedEditing();
  
  void on_viewOptionsButton_clicked();
  void on_viewAlphasButton_clicked();
  void on_deleteCacheButton_clicked();
  
private:
  RasterLayerNetwork::RefPtr _layer;
  Ui::EditWmsLayerWidget *_implementation;
};


}
}

#endif // __EDIT_WMS_LAYER_WIDGET_H__
