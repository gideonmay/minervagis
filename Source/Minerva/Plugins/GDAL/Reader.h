
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_LAYERS_GDAL_READERS_H__
#define __MINERVA_LAYERS_GDAL_READERS_H__

#include "Usul/Base/Referenced.h"

#include "Minerva/Common/IReadFeature.h"

namespace Minerva {
namespace Layers {
namespace GDAL {
      
class Reader : 
  public Usul::Base::Referenced,
  public Minerva::Common::IReadFeature 
{
public:

  typedef Usul::Base::Referenced BaseClass;

  USUL_DECLARE_QUERY_POINTERS ( Reader );
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  Reader();

  /// Get the filters for this reader.
  virtual Filters filters() const;
  
  /// Can this reader handle the extension?
  virtual bool canHandle ( const std::string& extension ) const;
  
  // Read.
  virtual Minerva::Core::Data::Feature* readFeature ( const std::string& filename );
  
protected:
  
  virtual ~Reader();
  
private:
  
  Filters _filters;
};

}
}
}

#endif // __MINERVA_LAYERS_GDAL_READERS_H__
