
cmake_minimum_required(VERSION "2.6.3")

PROJECT (MinervaGis)

SET(OUTPUT_BINDIR ${PROJECT_SOURCE_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})

SET(OUTPUT_LIBDIR ${PROJECT_SOURCE_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})

IF(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION GREATER 4)
    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    IF(WIN32)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    ELSE(WIN32)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    ENDIF(WIN32)
ELSE(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION GREATER 4)
    SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_BINDIR})
    SET(LIBRARY_OUTPUT_PATH ${OUTPUT_LIBDIR})
ENDIF(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION GREATER 4)

INCLUDE(CPack)
SET ( CPACK_PACKAGE_NAME Minerva )

# Set variables needed below...
SET ( CADKIT_INC_DIR ${PROJECT_SOURCE_DIR}/Source )
SET ( CMakeModules "${PROJECT_SOURCE_DIR}/CMakeModules")

# Path to look for .cmake files.
SET( CMAKE_MODULE_PATH ${CMakeModules} )

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

INCLUDE ( Cadkit )

#########################################################
#
#  Set some Apple specific properties.
#
#########################################################

IF (APPLE)
	
	option(OSX_DWARF_DEBUG "Use Dwarf Debug Style" ON)
	IF (OSX_DWARF_DEBUG )
		SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-2")
	ENDIF(OSX_DWARF_DEBUG )

	# CMake doesn't define _DEBUG when using Xcode.
	IF(${CMAKE_GENERATOR} MATCHES "Xcode")
		SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG" )
		
		SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
        
	ENDIF(${CMAKE_GENERATOR} MATCHES "Xcode")
	
ENDIF (APPLE)


#########################################################
#
#  Find boost.
#
#########################################################

SET ( BOOST_INCLUDEDIR $ENV{BOOST_INC_DIR} )
SET ( BOOST_LIBRARYDIR $ENV{BOOST_LIB_DIR} )
SET ( Boost_ADDITIONAL_VERSIONS "1.42" "1.42.0" "1.43" "1.43.0" "1.44" "1.44.0" "1.45" "1.45.0" )
IF(MSVC)

	ADD_DEFINITIONS( "-DBOOST_ALL_DYN_LINK" )
	FIND_PACKAGE(Boost)
	LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
	
ELSE(MSVC)

	FIND_PACKAGE(Boost COMPONENTS date_time filesystem regex thread system program_options signals )

ENDIF(MSVC)

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

# Find dependencies.
FIND_PACKAGE(OpenGL)

#SET ( OpenSceneGraph_DEBUG TRUE )
FIND_PACKAGE( OpenSceneGraph 2.8.0 REQUIRED osgDB osgUtil osgText osgGA osgViewer )
INCLUDE( FindXerces )
INCLUDE( FindGDAL )
INCLUDE( FindSpatialite )
INCLUDE( FindCURL )

#  Options to configure which libraries get built.
OPTION ( BUILD_MINERVA_VR "Build VR components." OFF )
OPTION ( BUILD_MINERVA_QT "Build Qt components." ON )

IF(BUILD_MINERVA_QT)

  SET ( QT_MAX_VERSION "4.9.9" )
  SET ( QT_USE_OPENGL TRUE )

  FIND_PACKAGE(Qt4)

  ADD_DEFINITIONS(${QT_DEFINITIONS})
  INCLUDE(${QT_USE_FILE})
ENDIF(BUILD_MINERVA_QT)

include_directories ( "${PROJECT_SOURCE_DIR}/Source" )

IF(MSVC)
	add_definitions ("-D_SCL_SECURE_NO_WARNINGS")
ENDIF(MSVC)

FIND_PACKAGE( ZLIB )
IF(ZLIB_FOUND)
  add_subdirectory ( External/minizip )
ENDIF()

# We only need to build helios if Qt is on.
IF(BUILD_MINERVA_QT)
	add_subdirectory ( External/haf/Helios )
ENDIF(BUILD_MINERVA_QT)

# include base libraries.
ADD_SUBDIRECTORY ( Source )

# include tools
ADD_SUBDIRECTORY ( Tools )

OPTION ( BUILD_MINERVA_TESTS "Build Minerva test programs."  OFF )

IF ( BUILD_MINERVA_TESTS )
	ADD_SUBDIRECTORY ( Tests )
ENDIF ( BUILD_MINERVA_TESTS )
