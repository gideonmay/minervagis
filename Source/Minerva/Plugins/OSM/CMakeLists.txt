
# Set include directories.
INCLUDE_DIRECTORIES( 
	${XERCESC_INCLUDE_DIR} 
	${Boost_INCLUDE_DIR}
	${OSG_INC_DIR} 
	${SPATIALITE_INCLUDE_DIRS}
)


# List the headers
SET ( HEADERS
./AdminBoundaryLayer.h
./Cache.h
./CityNameLayer.h
./Common.h
./Export.h
./Functions.h
./LineJob.h
./LineString.h
./LineStringParser.h
./Node.h
./Object.h
./OpenStreetMapFile.h
./OpenStreetMapXAPI.h
./Parser.h
./RoadLayer.h
./TileVectorJob.h
./Way.h
./XAPIMapQuery.h
)
			 
# List the Sources
SET (SOURCES
./AdminBoundaryLayer.cpp
./Cache.cpp
./CityNameLayer.cpp
./Functions.cpp
./LineJob.cpp
./LineString.cpp
./LineStringParser.cpp
./Node.cpp
./Object.cpp
./OpenStreetMapFile.cpp
./OpenStreetMapXAPI.cpp
./Parser.cpp
./RoadLayer.cpp
./TileVectorJob.cpp
./Way.cpp
./XAPIMapQuery.cpp
)

ADD_DEFINITIONS ("-D_COMPILING_MINERVA_OSM")

SET ( TARGET_NAME MinervaOSM )

# Create a Shared Library
ADD_LIBRARY( ${TARGET_NAME} SHARED ${HEADERS} ${SOURCES} )
CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

# Link the Library	
LINK_CADKIT( ${TARGET_NAME} Usul MinervaOsgTools SerializeXML XmlTree MinervaCore DatabaseSQLite MinervaNetwork )
TARGET_LINK_LIBRARIES( ${TARGET_NAME}
	${Boost_LIBRARIES}
	${OPENTHREADS_LIB}
	${OSG_LIB}
	${OSG_DB_LIB}
	${SPATIALITE_LIBRARY}
)
