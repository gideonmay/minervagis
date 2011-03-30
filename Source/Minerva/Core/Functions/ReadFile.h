
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Read the file.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_FUNCTIONS_READ_FILE_H__
#define __MINERVA_CORE_FUNCTIONS_READ_FILE_H__

#include "Minerva/Core/Export.h"

#include <string>

namespace Minerva { namespace Core { namespace Data { class Feature; } } }

namespace Minerva {
namespace Core {
namespace Functions {


MINERVA_EXPORT Minerva::Core::Data::Feature* readFile ( const std::string& filename );


}
}
}


#endif // __MINERVA_CORE_FUNCTIONS_READ_FILE_H__
