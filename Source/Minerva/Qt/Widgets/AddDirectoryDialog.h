
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Add a directory.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_ADD_DIRECTORY_DIALOG_H__
#define __MINERVA_ADD_DIRECTORY_DIALOG_H__

#include "Minerva/Qt/Widgets/Export.h"

#include "QtGui/QDialog"

#include <vector>
#include <string>

namespace Ui { class AddDirectoryDialog; }

namespace Minerva {
namespace QtWidgets {

class MINERVA_QT_WIDGETS_EXPORT AddDirectoryDialog : public QDialog
{
  Q_OBJECT;
public:
  typedef QDialog BaseClass;

  AddDirectoryDialog ( QWidget *parent = 0x0 );
  virtual ~AddDirectoryDialog();

  typedef std::vector<std::string> Filenames;

  void getFilenames ( Filenames& filenames ) const;

private slots:

  void on_browseDirectory_clicked();
  
private:

  Ui::AddDirectoryDialog *_impl;
  
};


}
}

#endif // __MINERVA_ADD_DIRECTORY_DIALOG_H__
