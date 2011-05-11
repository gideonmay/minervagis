
#########################################################
#
#  CMakeLists for Minerva library.
#
#########################################################


ADD_DEFINITIONS("-D_COMPILING_MINERVA")

IF(MSVC)
	ADD_DEFINITIONS("-DCURL_STATICLIB -D_CRT_SECURE_NO_WARNINGS")
ENDIF(MSVC)

LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )


# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES( 
		     ${Boost_INCLUDE_DIR}
		     ${CADKIT_INC_DIR}/GenericNurbs
		     ${OSG_INC_DIR} 
		     )

#########################################################
#
#  Add Source groups.
#
#########################################################

SOURCE_GROUP ( Source                FILES Export.h Extents.h Macros.h Serialize.h Visitor.h Visitor.cpp )
SOURCE_GROUP ( Source\\Algorithms    REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Algorithms/* )
SOURCE_GROUP ( Source\\Animate       REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Animate/* )
SOURCE_GROUP ( Source\\Commands      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Commands/* )
SOURCE_GROUP ( Source\\Data          REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Data/* )
SOURCE_GROUP ( Source\\Factory       REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Factory/* )
SOURCE_GROUP ( Source\\Functions     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Functions/* )
SOURCE_GROUP ( Source\\Jobs          REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Jobs/* )
SOURCE_GROUP ( Source\\Layers        REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Layers/* )
SOURCE_GROUP ( Source\\TileEngine    REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/TileEngine/* )
SOURCE_GROUP ( Source\\Utilities     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/* )
SOURCE_GROUP ( Source\\Visitors      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Visitors/* )


#########################################################
#
#  List all the headers
#
#########################################################

SET ( HEADERS
	./Algorithms/Composite.h
	./Algorithms/Resample.h
	./Algorithms/ResampleElevation.h
	./Algorithms/SubRegion.h
	./Data/Date.h
	./Data/AbstractView.h
	./Data/AltitudeMode.h
	./Data/Camera.h
	./Data/CameraState.h
	./Data/ColorStyle.h
	./Data/Container.h
	./Data/DataObject.h
	./Data/Feature.h
	./Data/Geometry.h
	./Data/IconStyle.h
	./Data/LabelStyle.h
	./Data/Line.h
	./Data/Link.h
	./Data/LineStyle.h
	./Data/LookAt.h
	./Data/Model.h
	./Data/ModelCache.h
	./Data/MultiGeometry.h
	./Data/MultiPoint.h
	./Data/NetworkLink.h
	./Data/Object.h
	./Data/OsgModel.h
	./Data/Point.h
	./Data/PointStyle.h
	./Data/Polygon.h
	./Data/PolyStyle.h
	./Data/Style.h
	./Data/TimePrimitive.h
	./Data/TimeSpan.h
	./Data/TimeStamp.h
	./Data/Transform.h
	./Data/UserData.h
	./DiskCache.h
	./ElevationData.h
	./Export.h
	./Factory/Readers.h
	./Functions/MakeBody.h
	./Functions/ReadFile.h
	./Functions/SearchDirectory.h
	./Jobs/BuildRaster.h
	./Jobs/BuildTiles.h
	./Layers/LayerInfo.h
	./Layers/RasterLayer.h
	./Layers/RasterLayerArcGIS.h
	./Layers/RasterLayerArcIMS.h
	./Layers/RasterLayerNetwork.h
	./Layers/RasterLayerWms.h
	./Macros.h
	./Navigator.h
	./TileEngine/Body.h
	./TileEngine/LandModel.h
	./TileEngine/LandModelEllipsoid.h
	./TileEngine/Mesh.h
	./TileEngine/SplitCallbacks.h
	./TileEngine/Tile.h
	./Utilities/Atmosphere.h
	./Utilities/Compass.h
	./Utilities/Hud.h
	./Utilities/SkyDome.h
	./Visitor.h
	./Visitors/FindMinMaxDates.h
	./Visitors/FindObject.h
	./Visitors/FindRasterLayers.h
	./Visitors/FindUnknowns.h
	./Visitors/ResizePoints.h
	./Visitors/StackPoints.h
	./Visitors/TemporalAnimation.h
)


#########################################################
#
#  List the sources.
#
#########################################################

SET (SOURCES
./Algorithms/ResampleElevation.cpp
./Data/Date.cpp
./Data/AbstractView.cpp
./Data/Camera.cpp
./Data/CameraState.cpp
./Data/ColorStyle.cpp
./Data/Container.cpp
./Data/DataObject.cpp
./Data/Feature.cpp
./Data/IconStyle.cpp
./Data/Geometry.cpp
./Data/LabelStyle.cpp
./Data/Line.cpp
./Data/Link.cpp
./Data/LineStyle.cpp
./Data/LookAt.cpp
./Data/Model.cpp
./Data/ModelCache.cpp
./Data/MultiGeometry.cpp
./Data/MultiPoint.cpp
./Data/NetworkLink.cpp
./Data/Object.cpp
./Data/OsgModel.cpp
./Data/Point.cpp
./Data/PointStyle.cpp
./Data/Polygon.cpp
./Data/PolyStyle.cpp
./Data/Style.cpp
./Data/TimePrimitive.cpp
./Data/TimeSpan.cpp
./Data/TimeStamp.cpp
./DiskCache.cpp
./ElevationData.cpp
./Factory/Readers.cpp
./Functions/MakeBody.cpp
./Functions/ReadFile.cpp
./Functions/SearchDirectory.cpp
./Jobs/BuildRaster.cpp
./Jobs/BuildTiles.cpp
./Layers/RasterLayer.cpp
./Layers/RasterLayerArcGIS.cpp
./Layers/RasterLayerArcIMS.cpp
./Layers/RasterLayerNetwork.cpp
./Layers/RasterLayerWms.cpp
./Navigator.cpp
./TileEngine/Body.cpp
./TileEngine/LandModelEllipsoid.cpp
./TileEngine/Mesh.cpp
./TileEngine/SplitCallbacks.cpp
./TileEngine/Tile.cpp
./Utilities/Atmosphere.cpp
./Utilities/Compass.cpp
./Utilities/Hud.cpp
./Utilities/SkyDome.cpp
./Visitor.cpp
./Visitors/FindMinMaxDates.cpp
./Visitors/FindObject.cpp
./Visitors/FindRasterLayers.cpp
./Visitors/ResizePoints.cpp
./Visitors/StackPoints.cpp
./Visitors/TemporalAnimation.cpp
)

# Create a Shared Library
ADD_LIBRARY(MinervaCore SHARED ${SOURCES} ${HEADERS} )

# Add a library
CADKIT_ADD_LIBRARY ( MinervaCore )

install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/cadkit/Minerva
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
LINK_CADKIT( MinervaCore Usul MinervaOsgTools MinervaCommon MinervaNetwork SerializeXML XmlTree )
TARGET_LINK_LIBRARIES(MinervaCore
	${Boost_DATE_TIME_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
  ${OPENTHREADS_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGDB_LIBRARY}
  ${OSGTEXT_LIBRARY}
  ${OSGUTIL_LIBRARY}
)
