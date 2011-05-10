
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

#ifndef _USUL_DOCUMENT_H_
#define _USUL_DOCUMENT_H_

#include "Usul/Export/Export.h"

#include "Usul/Documents/FileInfo.h"

#include "Usul/Base/Object.h"
#include "Usul/Interfaces/IUnknown.h"
#include "Usul/Pointers/Pointers.h"

#include <string>
#include <iosfwd>
#include <vector>


namespace Usul {
namespace Documents {


class USUL_EXPORT Document : public Usul::Base::Object
{
public:

  /// Typedefs.
  typedef Usul::Base::Object                    BaseClass;
  typedef Usul::Interfaces::IUnknown            Unknown;
  typedef std::pair<std::string,std::string>    Filter;
  typedef std::vector<Filter>                   Filters;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( Document );

  /// Construction.
  Document ( const std::string &type );

  /// Clear any existing data.
  virtual void                clear ( Unknown *caller = 0x0 ) {}

  /// Assign the document a default file name
  void                        defaultFilename();

  /// Set/get the file name.
  virtual const std::string & fileName() const { return _file.name(); }
  virtual void                fileName ( const std::string &n ) { _file.name ( n ); }

  /// Set/get the valid flag.
  virtual bool                fileValid() const { return _file.valid(); }
  virtual void                fileValid ( bool v ) { _file.valid ( v ); }

  /// Get the filters that correspond to what this document can do.
  virtual Filters             filtersExport() const = 0;
  virtual Filters             filtersInsert() const = 0;
  virtual Filters             filtersOpen()   const = 0;
  virtual Filters             filtersSave()   const = 0;

  /// Set/get the modified flag.
  virtual bool                modified() const;
  virtual void                modified ( bool m );

  /// Open the file. Clears any data this document already has.
  void                        open ( const std::string &filename, Unknown *caller = 0x0, Unknown *progress = 0x0 );

  /// Read the file and add it to existing document's data.
  virtual void                read ( const std::string &filename, Unknown *caller = 0x0, Unknown *progress = 0x0 );

  /// Returns appropriate tag name for registry.
  virtual std::string         registryTagName() const;

  /// Return the name of this type of document.
  virtual std::string         typeName() const;

  /// Write the document to given file name. Does not rename this document.
  virtual void                write ( const std::string &filename, Unknown *caller = 0x0, Unknown *progress = 0x0 ) const;

protected:

  /// Do not copy.
  Document ( const Document & );
  Document &operator = ( const Document & );

  /// Use reference counting.
  virtual ~Document();

private:

  FileInfo _file;
  std::string _typeName;
};


} // namespace Documents
} // namespace Usul


#endif // _USUL_DOCUMENT_H_

