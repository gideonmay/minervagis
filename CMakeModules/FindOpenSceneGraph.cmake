
include ( MinervaFunctions )
include(FindPackageHandleStandardArgs)

# Find the include path
FIND_PATH ( OSG_INCLUDE_DIR osg/Node "$ENV{OSG_INC_DIR}")
SET(OSG_INC_DIR ${OSG_INCLUDE_DIR})

# Build the list of libraries to look for.
set(_osg_modules_to_process)
foreach(_osg_component ${OpenSceneGraph_FIND_COMPONENTS})
    list(APPEND _osg_modules_to_process ${_osg_component})
endforeach()
list(APPEND _osg_modules_to_process "osg")
list(APPEND _osg_modules_to_process "OpenThreads")
list(REMOVE_DUPLICATES _osg_modules_to_process)


#  Find OpenSceneGraph libraries.
foreach(_osg_module ${_osg_modules_to_process})
  #message("[ FindOpenSceneGraph.cmake:${CMAKE_CURRENT_LIST_LINE} ] "
  #          "Calling minerva_find_library(${_osg_module} $ENV{OSG_LIB_DIR})")

  minerva_find_library(${_osg_module} "$ENV{OSG_LIB_DIR}")

  string(TOUPPER ${_osg_module} _osg_module_UC)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(${_osg_module} DEFAULT_MSG ${_osg_module_UC}_LIBRARY OSG_INCLUDE_DIR)

endforeach()

