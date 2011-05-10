
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2005, Perry L. Miller IV and Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Document class.
//
///////////////////////////////////////////////////////////////////////////////

#include "Usul/Documents/Document.h"
#include "Usul/Jobs/Job.h"
#include "Usul/Registry/Database.h"
#include "Usul/Threads/Safe.h"

#include <sstream>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace Usul;
using namespace Usul::Documents;


///////////////////////////////////////////////////////////////////////////////
//
//  Macro for printing if there is a stream.
//
///////////////////////////////////////////////////////////////////////////////

#define OUTPUT(stream) if ( 0x0 != stream ) (*stream)


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Document::Document ( const std::string &type ) : BaseClass(), 
  _file      (),
  _typeName  ( type )
{
  this->fileValid ( false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Document::~Document()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Open the file. Clears any data this document already has.
//
///////////////////////////////////////////////////////////////////////////////

void Document::open ( const std::string &file, Usul::Interfaces::IUnknown *caller, Unknown *progress )
{
  this->clear();
  this->read ( file, caller, progress );
  this->fileName ( file );
  this->fileValid ( true );
  this->modified ( false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the modified flag.
//
///////////////////////////////////////////////////////////////////////////////

bool Document::modified() const
{
  Guard guard ( this );
  return _file.modified();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the modified flag.
//
///////////////////////////////////////////////////////////////////////////////

void Document::modified ( bool m )
{
  // Get current state.
  const bool current ( this->modified() );

  // If the state is different.
  if ( current != m )
  {
    // Guard while setting the flag.
    Guard guard ( this );
    _file.modified ( m );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Assign the document a default file name.
//
///////////////////////////////////////////////////////////////////////////////

void Document::defaultFilename()
{
  // Assign default filename.
  static unsigned int count ( 0 );
  std::ostringstream name;
  name << "Untitled" << ++count;
  this->fileName  ( name.str() );
  this->fileValid ( false );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the name of this type of document.
//
///////////////////////////////////////////////////////////////////////////////

std::string Document::typeName() const
{
  Guard guard ( this );
  return _typeName;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Returns appropriate tag name for registry.
//
///////////////////////////////////////////////////////////////////////////////

std::string Document::registryTagName() const
{
  Guard guard ( this );

  const std::string name ( ( true == this->fileValid() ) ? this->fileName() : this->typeName() );
  const std::string tag ( Usul::Registry::Database::instance().convertToTag ( name ) );

  return tag;
}

/// Read the file and add it to existing document's data.
void Document::read ( const std::string &filename, Unknown *caller, Unknown *progress )
{
}

/// Write the document to given file name. Does not rename this document.
void Document::write ( const std::string &filename, Unknown *caller, Unknown *progress ) const
{
}
