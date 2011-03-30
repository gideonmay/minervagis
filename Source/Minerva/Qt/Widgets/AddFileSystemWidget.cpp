
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author(s): Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Qt/Widgets/AddFileSystemWidget.h"
#include "Minerva/Qt/Widgets/AddDirectoryDialog.h"
#include "ui_AddFileSystemWidget.h"

#include "Minerva/Qt/Tools/FileDialog.h"

#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/Polygon.h"
#include "Minerva/Core/Factory/Readers.h"
#include "Minerva/Core/Functions/ReadFile.h"
#include "Minerva/Core/Functions/SearchDirectory.h"

#include "Usul/Adaptors/Random.h"
#include "Usul/Components/Manager.h"
#include "Usul/File/Path.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Jobs/Job.h"
#include "Usul/Jobs/Manager.h"
#include "Usul/Registry/Database.h"
#include "Usul/Strings/Split.h"
#include "Usul/User/Directory.h"

#include "QtGui/QCheckBox"
#include "QtGui/QFileDialog"
#include "QtGui/QListWidget"
#include "QtGui/QPushButton"
#include "QtGui/QVBoxLayout"

#include "boost/bind.hpp"
#include "boost/filesystem/operations.hpp"

#include <iostream>

using namespace Minerva::QtWidgets;

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

AddFileSystemWidget::AddFileSystemWidget ( QWidget *parent ) : BaseClass ( parent ),
  _impl ( new Ui::AddFileSystemWidget )
{
  _impl->setupUi ( this );

  // Set initial check state.
  _impl->loadDataCheckBox->setChecked ( true );
  _impl->showDataExtentsCheckBox->setChecked ( false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

AddFileSystemWidget::~AddFileSystemWidget()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Browse.
//
///////////////////////////////////////////////////////////////////////////////

void AddFileSystemWidget::on_addFilesButton_clicked()
{
  // Useful typedefs.
  typedef QtTools::FileDialog              FileDialog;
  typedef FileDialog::FilesResult          FilesResult;
  typedef FileDialog::Filter               Filter;
  typedef FileDialog::Filters              Filters;
  typedef FileDialog::FileNames            FileNames;
  
  Filters f ( Minerva::Core::Factory::Readers::instance().filters() );
  
  Filters filters ( f.begin(), f.end() );
  filters.push_back ( Filter ( "All Files (*.*)", "*.*" ) );
  
  // Prompt the user.
  FilesResult results ( FileDialog::getLoadFileNames ( this, "Open Geospatial Data", filters ) );
  
  // Get the filenames.
  FileNames filenames ( results.first );
  
  // Add the filenames to our list.
  for ( FileNames::iterator iter = filenames.begin(); iter != filenames.end (); ++iter )
    _impl->listWidget->addItem ( iter->c_str() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add a directory.
//
///////////////////////////////////////////////////////////////////////////////

void AddFileSystemWidget::on_addDirectoryButton_clicked()
{
  QString dir ( QFileDialog::getExistingDirectory ( this, tr ( "Search Directory" ),
                                                  "",
                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ) );
  
  if ( false == dir.isEmpty() )
  {
    _impl->listWidget->addItem ( dir );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Search a directory.
//
///////////////////////////////////////////////////////////////////////////////

void AddFileSystemWidget::on_searchDirectoryButton_clicked()
{
  AddDirectoryDialog dialog ( this );
  if ( QDialog::Accepted == dialog.exec() )
  {
    AddDirectoryDialog::Filenames filenames;
    dialog.getFilenames ( filenames );
    for ( unsigned int i = 0; i < filenames.size(); ++i )
    {
      _impl->listWidget->addItem ( filenames.at ( i ).c_str() );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add images to caller.
//
///////////////////////////////////////////////////////////////////////////////

void AddFileSystemWidget::apply ( Minerva::Core::Data::Feature* parent, DataLoadedCallback callback )
{
  if ( 0x0 == _impl->listWidget )
    return;

  // How many items.
  const unsigned int size ( _impl->listWidget->count() );
  
  // Vector to contain filenames (or directories).
  Filenames names;
  names.reserve ( size );

  for ( unsigned int i = 0; i < size; ++i )
  {
    QListWidgetItem *item ( _impl->listWidget->item ( i ) );

    if( 0x0 != item )
    {
      names.push_back ( item->text().toStdString() );
    }
  }

  if ( Qt::Checked == _impl->loadDataCheckBox->checkState() )
  {
    // Add the job to the manager.
    Usul::Jobs::Job::RefPtr job ( Usul::Jobs::create ( boost::bind ( &AddFileSystemWidget::_loadData, names, parent, callback ) ) );
    Usul::Jobs::Manager::instance().addJob ( job );
  }
  if ( Qt::Checked == _impl->showDataExtentsCheckBox->checkState() )
  {
    // Add the job to the manager.
    Usul::Jobs::Job::RefPtr job ( Usul::Jobs::create ( boost::bind ( &AddFileSystemWidget::_showDataExtents, names, parent ) ) );
    Usul::Jobs::Manager::instance().addJob ( job );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Load the data.
//
///////////////////////////////////////////////////////////////////////////////

void AddFileSystemWidget::_loadData ( Filenames filenames, Minerva::Core::Data::Feature* parent, DataLoadedCallback callback )
{
  Minerva::Core::Data::Container::RefPtr container ( 0x0 != parent ? parent->asContainer() : 0x0 );
  
  if ( false == container.valid() )
    return;

  for ( Filenames::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter )
  {
    const std::string filename ( *iter );

    // See if the file is a directory...
    if ( boost::filesystem::is_directory ( filename ) )
    {
      Minerva::Core::Data::Container::RefPtr group ( new Minerva::Core::Data::Container );
      group->name ( filename );

      // Add the group.
      container->add ( group );

      // Search the directory (and all sub-directories).
      Minerva::Core::Functions::searchDirectory ( *group, filename );
    }
    else
    {
      Minerva::Core::Data::Feature::RefPtr feature ( Minerva::Core::Functions::readFile ( filename ) );
      if ( feature )
      {
        container->add ( feature );
      }
    }
  }

  if ( callback )
  {
    callback();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper class to get color based on filename.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  class ColorChooser
  {
  public:

    typedef std::map<std::string,Usul::Math::Vec4f> ColorMap;

    ColorChooser() : _random ( 0.0f, 1.0f ), _colors() 
    {
    }

    Usul::Math::Vec4f operator () ( const std::string& filename )
    {
      const std::string ext ( Usul::File::extension ( filename ) );
      
      ColorMap::const_iterator iter ( _colors.find ( ext ) );
      if ( iter != _colors.end() )
        return iter->second;

      Usul::Math::Vec4f color ( _random(), _random(), _random(), 1.0 );
      _colors.insert ( std::make_pair ( ext, color ) );
      return color;
    }
  private:
    Usul::Adaptors::Random<float> _random;
    ColorMap _colors;

  } colors;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Make polygon for extents of filename.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  Minerva::Core::Data::DataObject* makeExtentsPolygon ( const std::string& filename )
  {
    // Typedefs.
    typedef Minerva::Core::Data::DataObject DataObject;
    typedef Minerva::Core::Data::Style Style;
    typedef Minerva::Core::Data::PolyStyle PolyStyle;
    typedef Minerva::Core::Data::Polygon Polygon;
    typedef Polygon::Vertices Vertices;
    typedef Polygon::Vertex Vertex;

    // Make the data object.
    DataObject::RefPtr object ( new DataObject );
    object->name ( filename );

    // Read the file.
    Minerva::Core::Data::Feature::RefPtr feature ( Minerva::Core::Functions::readFile ( filename ) );

    if ( false == feature.valid() )
      return 0x0;

    // Get the extents values.
    Minerva::Common::Extents extents ( feature->extents() );
    const double minLon ( extents.minLon() );
    const double minLat ( extents.minLat() );
    const double maxLon ( extents.maxLon() );
    const double maxLat ( extents.maxLat() );

    // Get the filesize.
    const unsigned long long size ( boost::filesystem::file_size ( filename ) );
    const double elevation ( static_cast<double> ( size ) / 100000.0 );

    // Make the polygon.
    Polygon::RefPtr polygon ( new Polygon );

    // The vertices.
    Vertices vertices;
    vertices.push_back ( Vertex ( minLon, minLat, elevation ) );
    vertices.push_back ( Vertex ( maxLon, minLat, elevation ) );
    vertices.push_back ( Vertex ( maxLon, maxLat, elevation ) );
    vertices.push_back ( Vertex ( minLon, maxLat, elevation ) );
    vertices.push_back ( Vertex ( minLon, minLat, elevation ) );

    polygon->outerBoundary ( vertices );
    polygon->altitudeMode ( Minerva::Core::Data::ALTITUDE_MODE_RELATIVE_TO_GROUND );
    polygon->extrude ( true );

    Usul::Math::Vec4f color ( Detail::colors ( filename ) );
    color[3] = 0.5;
    
    Style::RefPtr style ( new Style );
    PolyStyle::RefPtr polyStyle ( new PolyStyle );
    polyStyle->color ( color );
    style->polystyle ( polyStyle.get() );

    object->style ( style );
    object->geometry ( polygon.get() );

    return object.release();
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show data extents and file size.
//
///////////////////////////////////////////////////////////////////////////////

void AddFileSystemWidget::_showDataExtents( Filenames filenames, Minerva::Core::Data::Feature* parent )
{
  Minerva::Core::Data::Container::RefPtr container ( 0x0 != parent ? parent->asContainer() : 0x0 );

  if ( false == container.valid() )
    return;

  for ( Filenames::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter )
  {
    const std::string filename ( *iter );

    // See if the file is a directory...
    if ( boost::filesystem::is_directory ( filename ) )
    {
      Filenames names;

      typedef boost::filesystem::directory_iterator Iterator;

      Iterator iter ( filename );
      Iterator end;
      for( ; iter != end; ++iter )
      {
        const boost::filesystem::path &path = iter->path();
#if BOOST_VERSION >= 104600
        names.push_back ( path.string() );
#else
        names.push_back ( path.native_directory_string() );
#endif
      }

      AddFileSystemWidget::_showDataExtents ( names, parent );
    }
    else
    {
      try
      {
        Minerva::Core::Data::DataObject::RefPtr object ( Detail::makeExtentsPolygon ( filename ) );
        container->add ( object );
      }
      USUL_DEFINE_SAFE_CALL_CATCH_BLOCKS ( "1383982710" )
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove selected files.
//
///////////////////////////////////////////////////////////////////////////////

void AddFileSystemWidget::on_removeSelectedFilesButton_clicked()
{
  typedef QList<QListWidgetItem *> Items;
  Items items ( _impl->listWidget->selectedItems() );
  for ( Items::iterator iter = items.begin(); iter != items.end(); ++iter )
  {
    _impl->listWidget->takeItem ( _impl->listWidget->row ( *iter ) );
    delete *iter;
  }
  
  _impl->listWidget->update();
}
