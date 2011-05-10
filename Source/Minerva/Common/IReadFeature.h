
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_INTERFACES_IREAD_FEATURE_H__
#define __MINERVA_INTERFACES_IREAD_FEATURE_H__

#include "Usul/Interfaces/IUnknown.h"

namespace Minerva { namespace Core { namespace Data { class Feature; } } }

#include <vector>

namespace Minerva {
namespace Common {

struct IReadFeature : public Usul::Interfaces::IUnknown
{
  /// Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( IReadFeature );
  
  /// Id for this interface.
  enum { IID = 3756431822u };
  
  typedef std::pair < std::string, std::string > Filter;
  typedef std::vector<Filter> Filters;
  
  /// Get the filters for this reader.
  virtual Filters filters() const = 0;
  
  /// Can this reader handle the extension?
  virtual bool canHandle ( const std::string& extension ) const = 0;
  
  /// Read a layer.
  virtual Minerva::Core::Data::Feature* readFeature ( const std::string& filename ) = 0;
  
}; // struct IReadFeature

} // end namespace Interfaces
} // end namespace Minerva


#endif // __MINERVA_INTERFACES_IREAD_FEATURE_H__ 
