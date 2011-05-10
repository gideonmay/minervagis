
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "MinervaQt/Plugins/PostGISLayerQt/DatabasePage.h"
#include "MinervaQt/Plugins/PostGISLayerQt/AddPostGISLayerWidget.h"
#include "MinervaQt/Plugins/PostGISLayerQt/ConnectToDatabase.h"

#include "QtGui/QListWidget"
#include "QtGui/QVBoxLayout"
#include "QtGui/QHBoxLayout"
#include "QtGui/QPushButton"
#include "QtGui/QLabel"

#include "QtSql/QSqlDatabase"

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

DatabasePage::DatabasePage ( QWidget * parent ) : BaseClass ( parent ),
  _layer ( 0x0 ),
  _listView ( 0x0 ),
  _connection ( 0x0 )
{
  QVBoxLayout *topLayout ( new QVBoxLayout );
  this->setLayout ( topLayout );

  // Connect to database.
  {
    QPushButton *button ( new QPushButton ( "Connect..." ) );
    connect ( button, SIGNAL ( clicked () ), this, SLOT ( _connectToDatabase () ) );
    topLayout->addWidget ( button );
  }

  _listView = new QListWidget;
  topLayout->addWidget ( _listView );

  // Single select.
  _listView->setSelectionMode ( QListWidget::SingleSelection );

  connect ( _listView, SIGNAL ( itemSelectionChanged () ), this, SLOT ( _selectionChanged () ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

DatabasePage::~DatabasePage()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  List all database tables.
//
///////////////////////////////////////////////////////////////////////////////

void DatabasePage::_listTables()
{
  if ( false == _connection.valid() )
    return;

  QSqlDatabase database;
  database.setDatabaseName ( _connection->database().c_str() );
  database.setHostName ( _connection->hostname().c_str() );
  database.setUserName ( _connection->username().c_str() );
  database.setPassword( _connection->password().c_str() );
  database.setPort ( 5432 );

  QStringList tables ( database.tables() );
  for ( QStringList::const_iterator iter = tables.begin (); iter != tables.end (); ++iter )
    _listView->addItem ( *iter );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Connect to a database.
//
///////////////////////////////////////////////////////////////////////////////

void DatabasePage::_connectToDatabase()
{
  ConnectToDatabase dialog ( this );

  _connection = dialog.exec();

  if ( _connection.valid() )
    this->_listTables ();
}


///////////////////////////////////////////////////////////////////////////////
//
//  The selection has changed.
//
///////////////////////////////////////////////////////////////////////////////

void DatabasePage::_selectionChanged ()
{
  if ( false == _connection.valid() )
    return;

  Minerva::Layers::GDAL::PostGISLayer::RefPtr layer ( new Minerva::Layers::GDAL::PostGISLayer );
  layer->connection ( _connection.get() );

  std::string table ( _listView->selectedItems().front()->text().toStdString () );
  layer->tablename ( table );

  _layer = layer;

  emit layerChanged( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the layer.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Layers::GDAL::PostGISLayer* DatabasePage::layer ()
{
  return _layer.get();
}
