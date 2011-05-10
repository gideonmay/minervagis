
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

#include "XmlTree/XercesLife.h"
#include "XmlTree/Functions.h"

#include "Usul/Functions/SafeCall.h"

#include "xercesc/util/OutOfMemoryException.hpp"
#include "xercesc/util/PlatformUtils.hpp"

#include <stdexcept>

using namespace XmlTree;


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize and terminate when this DLL is loaded and unloaded.
//
///////////////////////////////////////////////////////////////////////////////

namespace
{
  XmlTree::XercesLife life;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize.
//
///////////////////////////////////////////////////////////////////////////////

namespace XmlTree
{
  namespace Helper
  {
    void initialize()
    {
      USUL_TRY_BLOCK
      {
        xercesc::XMLPlatformUtils::Initialize();
      }
      catch ( const xercesc::OutOfMemoryException & )
      {
        throw std::runtime_error 
          ( "Error 4811502520: Ran out of memory while initializing Xerces" );
      }
      catch ( const xercesc::XMLException &e )
      {
        throw std::runtime_error ( 
          "Error 2787644571: Failed to initialize Xerces. " + 
          XmlTree::Functions::translate ( e.getMessage() ) );
      }
      catch ( const std::exception &e )
      {
        throw std::runtime_error ( 
          "Error 3205772651: Standard exception caught while initializing Xerces. " +
          std::string ( ( ( e.what() ) ? e.what() : "" ) ) );
      }
      catch ( ... )
      {
        throw std::runtime_error ( 
          "Error 2268037835: Unknown exception caught while initializing Xerces" );
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Finish.
//
///////////////////////////////////////////////////////////////////////////////

namespace XmlTree
{
  namespace Helper
  {
    void terminate()
    {
      USUL_TRY_BLOCK
      {
        xercesc::XMLPlatformUtils::Terminate();
      }
      catch ( const xercesc::XMLException &e )
      {
        throw std::runtime_error ( 
          "Error 1972549100: Failed to terminate Xerces, " + 
          XmlTree::Functions::translate ( e.getMessage() ) );
      }
      catch ( const std::exception &e )
      {
        throw std::runtime_error ( 
          "Error 2032356942: Standard exception caught while terminating Xerces. " + 
          std::string ( ( ( e.what() ) ? e.what() : "" ) ) );
      }
      catch ( ... )
      {
        throw std::runtime_error ( "Error 2543876004: Unknown exception caught while terminating Xerces" );
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

XercesLife::XercesLife()
{
  XmlTree::Helper::initialize();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor
//
///////////////////////////////////////////////////////////////////////////////

XercesLife::~XercesLife()
{
  Usul::Functions::safeCall ( XmlTree::Helper::terminate, "3297964011" );
}
