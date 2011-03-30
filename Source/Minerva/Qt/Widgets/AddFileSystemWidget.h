
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author(s): Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __ADD_OSSIM_LAYER_WIDGET_H__
#define __ADD_OSSIM_LAYER_WIDGET_H__

#include "Minerva/Qt/Widgets/Export.h"

#include "Minerva/Core/Data/Container.h"

#include "Minerva/Common/ILayerAddGUIQt.h"

#include "QtGui/QWidget"

#include <vector>
#include <string>

namespace Ui { class AddFileSystemWidget; }

namespace Minerva {
namespace QtWidgets {

class MINERVA_QT_WIDGETS_EXPORT AddFileSystemWidget : public QWidget
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;
  typedef Minerva::Common::ILayerAddGUIQt::DataLoadedCallback DataLoadedCallback;

  AddFileSystemWidget ( QWidget *parent = 0x0 );
  virtual ~AddFileSystemWidget();

  void             apply ( Minerva::Core::Data::Feature* parent, DataLoadedCallback callback );

private slots:

  void on_addFilesButton_clicked();
  void on_addDirectoryButton_clicked();
  void on_searchDirectoryButton_clicked();
  void on_removeSelectedFilesButton_clicked();
  
private:

  typedef std::vector<std::string> Filenames;
  
  // Pass the filesnames by copy so these functions can be threaded.
  static void      _loadData ( Filenames filenames, Minerva::Core::Data::Feature* parent, DataLoadedCallback callback );
  static void      _showDataExtents ( Filenames filenames, Minerva::Core::Data::Feature* parent );

  Ui::AddFileSystemWidget *_impl;
};

}
}

#endif // __ADD_OSSIM_LAYER_WIDGET_H__
