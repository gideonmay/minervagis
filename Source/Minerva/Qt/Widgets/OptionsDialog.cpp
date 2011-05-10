
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Qt/Widgets/OptionsDialog.h"
#include "ui_OptionsDialog.h"


using namespace Minerva::QtWidgets;

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

OptionsDialog::OptionsDialog ( const Options& options, QWidget* parent ) : BaseClass ( parent ),
  _optionsDialog ( new Ui::OptionsDialog )
{
  _optionsDialog->setupUi( this );
    
  for ( Options::const_iterator iter = options.begin(); iter != options.end(); ++iter )
  {
    _optionsDialog->_optionsTreeWidget->addItem ( iter->first, iter->second );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

OptionsDialog::~OptionsDialog()
{
  delete _optionsDialog;
  _optionsDialog = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the options.
//
///////////////////////////////////////////////////////////////////////////////

OptionsDialog::Options OptionsDialog::options() const
{
  Options options;
  
  {
    typedef Minerva::QtTools::StringsView::Items Items;
    Items values ( _optionsDialog->_optionsTreeWidget->items() );
  
    for ( Items::const_iterator iter = values.begin(); iter != values.end(); ++iter )
    {
      options.insert ( *iter );
    }
  }
  
  return options;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add a row.
//
///////////////////////////////////////////////////////////////////////////////

void OptionsDialog::on_addRowButton_clicked()
{
  _optionsDialog->_optionsTreeWidget->addItem ( "Key", "Value" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove a row.
//
///////////////////////////////////////////////////////////////////////////////

void OptionsDialog::on_removeRowButton_clicked()
{
  _optionsDialog->_optionsTreeWidget->removeRow ( _optionsDialog->_optionsTreeWidget->currentRow() );
}
