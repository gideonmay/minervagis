
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __ADD_POSTGIS_LAYER_WIDGET_H__
#define __ADD_POSTGIS_LAYER_WIDGET_H__

#include "MinervaQt/Plugins/PostGISLayerQt/CompileGuard.h"

#include "QtGui/QWidget"

#include <vector>
#include <string>


class QPushButton;
class QStackedWidget;

class DatabasePage;
class PropertyPage;

namespace Minerva { namespace Core { namespace Data { class Feature; } } }

class AddPostGISLayerWidget : public QWidget
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;

  AddPostGISLayerWidget( QWidget *parent = 0x0 );
  virtual ~AddPostGISLayerWidget();

  void apply ( Minerva::Core::Data::Feature* parent );

protected slots:
  void _editLayerProperties ();

private:
  DatabasePage   *_databasePage;
  PropertyPage   *_propertyPage;
  QPushButton    *_editButton;
  QStackedWidget *_stackedWidget;
};


#endif // __ADD_POSTGIS_LAYER_WIDGET_H__
