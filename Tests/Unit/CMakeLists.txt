
INCLUDE_DIRECTORIES( ${GOOGLE_TEST_INCLUDE_DIR} ${OSG_INC_DIR} ${Boost_INCLUDE_DIR} )

SET ( SOURCES
./Minerva/Common/ExtentsTest.cpp
./Minerva/Common/TileKeyTest.cpp
./Minerva/Core/TileEngine/TileTest.cpp
./Minerva/Layers/Kml/ParseTest.cpp
./Minerva/Layers/Kml/ParseMultiGeometryTest.cpp
./Minerva/Document/AnimationControllerTest.cpp
./Minerva/OsgTools/MatrixConvertText.cpp
./Minerva/Ellipsoid/EllipsoidTest.cpp
./Usul/Math/BarycentricTest.cpp
./Main.cpp
)

IF ( SPATIALITE_FOUND )
  SET ( MINERVA_LAYERS_OSM_SOURCES
	./Minerva/Layers/OSM/CacheTest.cpp )

  SET ( SOURCES ${SOURCES} ${MINERVA_LAYERS_OSM_SOURCES} )
ENDIF ( SPATIALITE_FOUND )

SET ( TARGET_NAME MinervaUnitTest )

ADD_EXECUTABLE( ${TARGET_NAME} ${SOURCES} )

# Add the target label.
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES PROJECT_LABEL "Test: ${TARGET_NAME}" )

# Add the debug postfix.
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}" )

# Link the Library	
LINK_CADKIT( ${TARGET_NAME} Usul MinervaCommon MinervaCore MinervaDocument MinervaKml )

IF ( SPATIALITE_FOUND )
  LINK_CADKIT( ${TARGET_NAME} MinervaOSM )
ENDIF ( SPATIALITE_FOUND )

TARGET_LINK_LIBRARIES( ${TARGET_NAME} ${GOOGLE_TEST_LIBRARY} )
