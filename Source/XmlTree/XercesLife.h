
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2005, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Starts and terminates Xerces library.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _XML_TREE_XERCES_LIFE_H_
#define _XML_TREE_XERCES_LIFE_H_

#include "XmlTree/Export.h"

#include "boost/noncopyable.hpp"


namespace XmlTree
{
  struct XML_TREE_EXPORT XercesLife : public boost::noncopyable
  {
    // Constructor initializes Xerces.
    XercesLife();

    // Destructor terminates Xerces.
    ~XercesLife();
  };
}


#endif // _XML_TREE_XERCES_LIFE_H_
