
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "MinervaQt/Plugins/PostGISLayerQt/PropertyPage.h"
#include "MinervaQt/Plugins/PostGISLayerQt/AddPostGISLayerWidget.h"
#include "MinervaQt/Plugins/PostGISLayerQt/LineWidget.h"
#include "MinervaQt/Plugins/PostGISLayerQt/PolygonWidget.h"

#include "Usul/Strings/Qt.h"

#include "Minerva/Qt/Tools/Color.h"

#include "QtSql/QSqlDatabase"
#include "QtSql/QSqlTableModel"

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

PropertyPage::PropertyPage( Layer *layer, QWidget *parent ) : BaseClass ( parent ),
  _layer ( layer ),
  _colorWidget ( 0x0 ),
  _primitiveWidget ( 0x0 )
{
  // Initialize code from Designer.
  this->setupUi ( this );

  // Initialize all the tabs.
  this->_initDrawingTab();
  this->_initGeneralTab();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

PropertyPage::~PropertyPage ()
{
  if ( 0x0 != _colorWidget )
    delete _colorWidget;

  if ( 0x0 != _primitiveWidget )
    delete _primitiveWidget;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize the general tab.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_initGeneralTab()
{
  if ( _layer.valid() )
  {
    _nameEdit->setText ( _layer->name().c_str() );
  }

  QObject::connect ( _nameEdit, SIGNAL ( editingFinished () ),      this, SLOT ( _nameChanged () ) );

  this->_initLabelProperties();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize the label properties.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_initLabelProperties()
{
  if ( _layer.valid() )
  {
    _labelShown->setChecked ( _layer->showLabel () );
    _labelColor->color ( Minerva::QtTools::Color< Usul::Math::Vec4f >::convert ( _layer->labelColor() ) );
    
    // Get all columns.
    Connection::RefPtr connection ( _layer->connection() );
    if ( connection.valid() )
    {
      QSqlDatabase database;
      database.setDatabaseName ( connection->database().c_str() );
      database.setHostName ( connection->hostname().c_str() );
      database.setUserName ( connection->username().c_str() );
      database.setPassword( connection->password().c_str() );
      database.setPort ( 5432 );

      boost::shared_ptr<QSqlTableModel> model ( new QSqlTableModel ( this ) );
      model->setTable ( _layer->tablename().c_str() );

      // Populate the combo box.
      _labelColumn->clear();
      for ( int i = 0; i < model->rowCount(); ++i )
      {
        _labelColumn->addItem ( model->headerData ( i, Qt::Horizontal ).toString() );
      }

      _labelColumn->setCurrentIndex ( _labelColumn->findText ( _layer->labelColumn().c_str() ) );
    }

    _labelSize->setValue ( static_cast<int> ( _layer->labelSize () ) );
    _labelZOffset->setValue ( static_cast<int> ( _layer->labelZOffset () ) );
  }

  // Slots and signals for labels.
  QObject::connect ( _labelShown,   SIGNAL ( stateChanged ( int )        ), this, SLOT ( _labelShownChanged   ( int ) ) );
  QObject::connect ( _labelColor,   SIGNAL ( colorChanged ( )            ), this, SLOT ( _labelColorChanged   ( )     ) );
  QObject::connect ( _labelColumn,  SIGNAL ( currentIndexChanged ( int ) ), this, SLOT ( _labelColumnChanged  ( int ) ) );
  QObject::connect ( _labelSize,    SIGNAL ( valueChanged ( int )        ), this, SLOT ( _labelSizeChanged    ( int ) ) );
  QObject::connect ( _labelZOffset, SIGNAL ( valueChanged ( int )        ), this, SLOT ( _labelZOffsetChanged ( int ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize the drawing tab.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_initDrawingTab()
{
  if ( false == _layer.valid() )
    return;

  QObject::connect ( _colorTypeComboBox, SIGNAL ( currentIndexChanged ( int ) ), this, SLOT ( _colorTypeChanged ( int ) ) );

  _colorTypeComboBox->addItem ( tr ( "Single Color" ) );
  _colorTypeComboBox->addItem ( tr ( "Gradient Color" ) );

  _primitiveWidget = new PolygonWidget ( _layer->style(), this );
  _primitiveStack->addWidget ( _primitiveWidget );
  _primitiveStack->setCurrentWidget ( _primitiveWidget );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The label shown state has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_labelShownChanged( int state )
{
  if ( _layer.valid() )
    _layer->showLabel ( state != 0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The label color has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_labelColorChanged()
{
  if ( _layer.valid() )
  {
    QColor c ( _labelColor->color() );
    _layer->labelColor ( Minerva::QtTools::Color< Usul::Math::Vec4f >::convert ( c ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  The label column has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_labelColumnChanged ( int index )
{
  if ( _layer.valid () )
    _layer->labelColumn ( _labelColumn->currentText().toStdString() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The label size has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_labelSizeChanged ( int value )
{
  if ( _layer.valid() )
    _layer->labelSize ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The label z offset has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_labelZOffsetChanged ( int value )
{
  if ( _layer.valid () )
    _layer->labelZOffset ( value );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The name has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PropertyPage::_nameChanged()
{
  if ( _layer.valid() )
    _layer->name ( _nameEdit->text().toStdString() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Rasterize.
//
///////////////////////////////////////////////////////////////////////////////

bool PropertyPage::rasterize() const
{
  bool rasterize ( false );
  
  if ( const PolygonWidget *pw = dynamic_cast<const PolygonWidget*> ( _primitiveWidget ) )
    rasterize = pw->rasterize();
  
  return rasterize;
}
