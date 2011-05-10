
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# Add definitions
ADD_DEFINITIONS ("-D_COMPILING_MINERVA_GDAL")

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES( 
  ${Boost_INCLUDE_DIR}
  ${OSG_INC_DIR} 
  ${GDAL_INCLUDE_DIR}
)


#List the Sources
SET (SOURCES
./Common.h
./Common.cpp
./Convert.h
./ConnectionInfo.h
./ConnectionInfo.cpp
./Dataset.cpp
./Dataset.h
./Export.h
./FlatLandModel.h
./FlatLandModel.cpp
./Init.cpp
./LabelData.h
./LabelData.cpp
./RasterLayerGDAL.cpp
./RasterLayerGDAL.h
./RasterPolygonLayer.h
./RasterPolygonLayer.cpp
./Reader.h
./Reader.cpp
./OGRConvert.h
./OGRConvert.cpp
./OGRReader.h
./OGRReader.cpp
./OGRVectorLayer.cpp
./OGRVectorLayer.h
./PostGISLayer.h
./PostGISLayer.cpp
)

SET ( TARGET_NAME MinervaGDAL )

# Create a Shared Library
IF (BUILD_GDAL_PLUGIN_AS_BUNDLE )
  ADD_LIBRARY( ${TARGET_NAME} MODULE ${SOURCES} )
ELSE(BUILD_GDAL_PLUGIN_AS_BUNDLE )
  ADD_LIBRARY( ${TARGET_NAME} SHARED ${SOURCES} )
ENDIF (BUILD_GDAL_PLUGIN_AS_BUNDLE )

CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

IF (BUILD_GDAL_PLUGIN_AS_BUNDLE )
  ADD_DEFINITIONS ("-D_DARWIN")
  SET_TARGET_PROPERTIES (${TARGET_NAME} PROPERTIES SUFFIX ".bundle" )
  SET ( CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-WI -bind_at_load -bundle" )
ENDIF (BUILD_GDAL_PLUGIN_AS_BUNDLE )

# Link the Library	
LINK_CADKIT( ${TARGET_NAME} Usul SerializeXML XmlTree MinervaCore )
TARGET_LINK_LIBRARIES( ${TARGET_NAME}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${GDAL_LIBRARY}
  ${OPENTHREADS_LIBRARY}
  ${OSG_LIBRARY}
)

install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/cadkit/Minerva/Plugins
            PATTERN "*.h"	
            PATTERN "*.cpp" EXCLUDE
            PATTERN ".svn" EXCLUDE
            PATTERN "Debug" EXCLUDE
            PATTERN "Release" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "*~" EXCLUDE		  
            PATTERN "*.vcproj" EXCLUDE )
