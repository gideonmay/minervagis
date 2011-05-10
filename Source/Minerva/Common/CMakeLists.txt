
#########################################################
#
#  CMakeLists for Minerva Common library.
#
#########################################################

ADD_DEFINITIONS("-D_COMPILING_MINERVA_COMMON")

IF(MSVC)
	ADD_DEFINITIONS(" -D_CRT_SECURE_NO_WARNINGS")
ENDIF(MSVC)

LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )


#########################################################
#
#  List all the headers
#
#########################################################

SET ( HEADERS
./Coordinates.h
./Export.h
./Extents.h
./IBuildScene.h
./IDatabaseConnection.h
./IElevationChangedListener.h
./IElevationData.h
./IElevationDatabase.h
./IIntersectNotify.h
./ILayerAddGUIQt.h
./ILayerModifyGUIQt.h
./IPlanetCoordinates.h
./IReadFeature.h
./IReadImageFile.h
./IRefreshData.h
./ITile.h
./ITileVectorData.h
./ITileVectorJob.h
./ITimer.h
./ITimerFactory.h
./IWithinExtents.h
./LayerKey.h
./TileKey.h
)


#########################################################
#
#  List the sources.
#
#########################################################

SET (SOURCES
./Coordinates.cpp
./LayerKey.cpp
./TileKey.cpp
)

# Create a Shared Library
ADD_LIBRARY(MinervaCommon SHARED ${SOURCES} ${HEADERS} )

# Add a library
CADKIT_ADD_LIBRARY ( MinervaCommon )

install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/cadkit/Minerva/
            PATTERN "*.h"	
            PATTERN "*.cpp" EXCLUDE
            PATTERN ".svn" EXCLUDE
            PATTERN "Debug" EXCLUDE
            PATTERN "Release" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "*~" EXCLUDE		  
            PATTERN "*.vcproj" EXCLUDE )
                    
INSTALL(FILES  ${CMAKE_CURRENT_SOURCE_DIR}/../Config.h DESTINATION include/cadkit/Minerva )
INSTALL(FILES  ${CMAKE_CURRENT_SOURCE_DIR}/../Version.h DESTINATION include/cadkit/Minerva )

# Link the Library	
LINK_CADKIT( MinervaCommon Usul SerializeXML XmlTree )
TARGET_LINK_LIBRARIES(MinervaCommon
	${Boost_DATE_TIME_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
)
