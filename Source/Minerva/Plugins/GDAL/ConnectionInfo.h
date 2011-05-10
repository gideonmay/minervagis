
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DB_CONNECTION_H__
#define __DB_CONNECTION_H__

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"
#include "Usul/Threads/Mutex.h"
#include "Usul/Threads/Guard.h"

#include "Serialize/XML/Macros.h"

#include <string>

typedef struct pg_conn PGconn;

namespace Minerva {
namespace Layers {
namespace GDAL {

class ConnectionInfo : public Usul::Base::Referenced
{
public:

  /// Typedefs.
  typedef Usul::Base::Referenced BaseClass;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( ConnectionInfo );

  ConnectionInfo();

  /// Build connection string for postgres.
  std::string          connectionString();

  /// Get the name of the connection
  std::string          name() const;

  /// Get/Set hostname.
  void                 hostname( const std::string& host );
  const std::string&   hostname() const;

  /// Get/Set database.
  void                 database( const std::string& db );
  const std::string&   database() const;

  /// Get/Set username.
  void                 username( const std::string& user );
  const std::string&   username() const;

  /// Get/Set password.
  void                 password( const std::string& pw );
  const std::string&   password() const;

	virtual void deserialize ( const XmlTree::Node &node );

protected:
  
  virtual ~ConnectionInfo();

private:

  std::string _host;
  std::string _database;
  std::string _user;
  std::string _password;

  SERIALIZE_XML_DEFINE_MAP;
	SERIALIZE_XML_CLASS_NAME ( ConnectionInfo );
	SERIALIZE_XML_SERIALIZE_FUNCTION;
	SERIALIZE_XML_ADD_MEMBER_FUNCTION;
};

}
}
}


#endif // __DB_CONNECTION_H__
