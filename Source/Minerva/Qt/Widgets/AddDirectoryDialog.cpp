
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

#include "Minerva/Qt/Widgets/AddDirectoryDialog.h"
#include "ui_AddDirectoryDialog.h"

#include "QtGui/QFileDialog"

#include "boost/filesystem/operations.hpp"
#include "boost/regex.hpp"
#include "boost/version.hpp"

using namespace Minerva::QtWidgets;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

AddDirectoryDialog::AddDirectoryDialog ( QWidget *parent ) : BaseClass ( parent ),
  _impl ( new Ui::AddDirectoryDialog )
{
  _impl->setupUi ( this );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

AddDirectoryDialog::~AddDirectoryDialog()
{
  delete _impl; _impl = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Browse for a directory.
//
///////////////////////////////////////////////////////////////////////////////

void AddDirectoryDialog::on_browseDirectory_clicked()
{
  QString dir ( QFileDialog::getExistingDirectory ( this, tr ( "Search Directory" ),
                                                  "",
                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ) );
  _impl->directory->setText ( dir );
}


namespace Detail {
  void searchDirectory ( const boost::filesystem::path& directory, const boost::regex& pattern, AddDirectoryDialog::Filenames& filenames )
  {
    boost::filesystem::directory_iterator iter ( directory );
    boost::filesystem::directory_iterator end;
    for( ; iter != end; ++iter )
    {
      const boost::filesystem::path &path ( iter->path() );

      // Make a recursive call if its a directory.
      if ( boost::filesystem::is_directory ( path ) )
      {
        Detail::searchDirectory ( path, pattern, filenames );
      }
      else
      {
#if BOOST_VERSION >= 104600
        const std::string name ( path.string() );
#else
        const std::string name ( path.native_directory_string() );
#endif

        if ( !pattern.empty() )
        {
          if ( boost::regex_match ( name, pattern ) )
          {
            filenames.push_back ( name );
          }
        }
        else
        {
          filenames.push_back ( name );
        }
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get all the filenames in the directory.
//
///////////////////////////////////////////////////////////////////////////////

void AddDirectoryDialog::getFilenames ( Filenames& filenames ) const
{
  const std::string directoryToSearch ( _impl->directory->text().toStdString() );
  if ( directoryToSearch.empty() )
    return;

  boost::regex pattern ( _impl->patternLineEdit->text().toStdString() );
  boost::filesystem::path path ( directoryToSearch );

  if ( boost::filesystem::exists ( path ) )
  {
    Detail::searchDirectory ( path, pattern, filenames );
  }
}
