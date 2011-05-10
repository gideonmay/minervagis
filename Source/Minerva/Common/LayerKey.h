
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Key for a layer.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_COMMON_LAYER_KEY_H__
#define __MINERVA_COMMON_LAYER_KEY_H__

#include "Minerva/Common/Export.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"

namespace Minerva {
namespace Common {


class MINERVA_COMMON_EXPORT LayerKey : public Usul::Base::Referenced
{
public:

  USUL_DECLARE_REF_POINTERS ( LayerKey );

  typedef Usul::Base::Referenced BaseClass;

  LayerKey ( const std::string& name, std::size_t id );

  const std::string& name() const;

  std::size_t id() const;

protected:

  virtual ~LayerKey();

private:

  std::string _name;
  std::size_t _id;
};


}
}

#endif // __MINERVA_COMMON_LAYER_KEY_H__
