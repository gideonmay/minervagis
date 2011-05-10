
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Dialog to edit DataObject
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_QT_WIDGETS_EDITDATAOBJECTDIALOG_H__
#define __MINERVA_QT_WIDGETS_EDITDATAOBJECTDIALOG_H__

#include <QDialog>

namespace Minerva { namespace Core { namespace Data { class DataObject; } } }
namespace Ui { class EditDataObjectDialog; }

namespace Minerva {
namespace QtWidgets {


class EditDataObjectDialog : public QDialog {
    Q_OBJECT
public:
  EditDataObjectDialog(QWidget *parent = 0);
  virtual ~EditDataObjectDialog();

  void populate ( Minerva::Core::Data::DataObject* );

  void applyChanges ( Minerva::Core::Data::DataObject* );

private:
  Ui::EditDataObjectDialog *ui;
};

}
}

#endif // __MINERVA_QT_WIDGETS_EDITDATAOBJECTDIALOG_H__
