
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Qt/Widgets/EditWmsLayerWidget.h"
#include "Minerva/Qt/Widgets/OptionsDialog.h"
#include "Minerva/Qt/Widgets/AlphasDialog.h"

#include "Minerva/Core/DiskCache.h"

#include "ui_EditWmsLayerWidget.h"

#include "Usul/Strings/Format.h"
#include "Usul/Scope/Caller.h"

#include "QtGui/QMessageBox"

#include <list>

using namespace Minerva::QtWidgets;

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

EditWmsLayerWidget::EditWmsLayerWidget ( RasterLayerNetwork *layer, QWidget * parent ) : BaseClass ( parent ),
  _layer ( layer ),
  _implementation ( new Ui::EditWmsLayerWidget )
{
  // Initialize code from designer.
  _implementation->setupUi ( this );
  
  if ( _layer.valid() )
  {
    _implementation->_name->setText( _layer->name().c_str() );
    _implementation->_server->setText ( _layer->urlBase().c_str() );
  }

  // Connect slots and signals.
  this->connect ( _implementation->_name, SIGNAL ( editingFinished() ), this, SLOT ( _nameFinishedEditing() ) );
  this->connect ( _implementation->_server, SIGNAL ( editingFinished() ), this, SLOT ( _serverFinishedEditing() ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

EditWmsLayerWidget::~EditWmsLayerWidget()
{
  delete _implementation; _implementation = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the new name.
//
///////////////////////////////////////////////////////////////////////////////

void EditWmsLayerWidget::_nameFinishedEditing()
{
  if ( _layer.valid() )
    _layer->name ( _implementation->_name->text().toStdString() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the new server.
//
///////////////////////////////////////////////////////////////////////////////

void EditWmsLayerWidget::_serverFinishedEditing()
{
  if ( _layer.valid() )
    _layer->urlBase ( _implementation->_server->text().toStdString() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The view options button has been clicked.
//
///////////////////////////////////////////////////////////////////////////////

void EditWmsLayerWidget::on_viewOptionsButton_clicked()
{
  if ( _layer.valid() )
  {
    Minerva::QtWidgets::OptionsDialog dialog ( _layer->options(), this );
    
    if ( QDialog::Accepted == dialog.exec() )
      _layer->options ( dialog.options() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  The view options button has been clicked.
//
///////////////////////////////////////////////////////////////////////////////

void EditWmsLayerWidget::on_viewAlphasButton_clicked()
{
  if ( _layer.valid() )
  {
    RasterLayerNetwork::Alphas alphas ( _layer->alphas() );
    
    Minerva::QtWidgets::AlphasDialog dialog ( alphas, this );
    
    if ( QDialog::Accepted == dialog.exec() )
      _layer->alphas ( dialog.alphas() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Delete the cache.
//
///////////////////////////////////////////////////////////////////////////////

void EditWmsLayerWidget::on_deleteCacheButton_clicked()
{
  if ( 0x0 == _layer )
    return;
  
  const std::string message ( "Are you sure you want to delete the cache?" );
  if ( QMessageBox::Ok != QMessageBox::question ( this, "Confirm", message.c_str(), QMessageBox::Ok | QMessageBox::Cancel ) )
    return;
  
  Minerva::Common::LayerKey::RefPtr key ( _layer->cacheKey() );
  Minerva::Core::DiskCache::instance().deleteCache ( *key );
}
