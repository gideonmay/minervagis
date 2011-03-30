
#########################################################
#
#  CMakeLists for Minerva Kml Layer.
#
#########################################################

ADD_DEFINITIONS("-D_COMPILING_MINERVA_KML")

#  Set link directories.
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

#  Set include directories.
INCLUDE_DIRECTORIES( 
			 ${Boost_INCLUDE_DIR}
		     ${OSG_INC_DIR} 
			 ${PROJECT_SOURCE_DIR}/External
		     )

IF(COLLADA_FOUND)
  ADD_DEFINITIONS("-DHAVE_COLLADA")
  INCLUDE_DIRECTORIES(${COLLADA_INCLUDE_DIR} ${COLLADA_INCLUDE_DIR}/1.4 )
ENDIF(COLLADA_FOUND)

# List the headers
SET ( HEADERS
	./Factory.h
	./KmlLayer.h
	./KmlReader.h
	./LoadModel.h
	./ModelPostProcess.h
)
			 
# List the Sources
SET (SOURCES
	./Factory.cpp
	./KmlLayer.cpp
	./KmlReader.cpp
	./LoadModel.cpp
	./ModelPostProcess.cpp
)

# Add header, cpp and include directory.
IF(ZLIB_FOUND)

  ADD_DEFINITIONS ( "-DHAVE_ZLIB" )

	SET ( HEADERS ${HEADERS} ZipFile.h )
	SET ( SOURCES ${SOURCES} ZipFile.cpp )
	
	INCLUDE_DIRECTORIES( ${ZLIB_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR}/contrib )
	
	#IF(MSVC)
	#	ADD_DEFINITIONS ( "-DZLIB_WINAPI" )
	#ENDIF(MSVC)
	
ENDIF(ZLIB_FOUND)

IF(MSVC)
	ADD_DEFINITIONS ( "-DDOM_DYNAMIC" )
ENDIF(MSVC)

SET ( TARGET_NAME MinervaKml )

# Create a Shared Library
ADD_LIBRARY( ${TARGET_NAME} SHARED ${HEADERS} ${SOURCES} )
CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

# Link the Library	
LINK_CADKIT( ${TARGET_NAME} Usul MinervaOsgTools SerializeXML XmlTree MinervaCore )
TARGET_LINK_LIBRARIES( ${TARGET_NAME}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
  ${OPENTHREADS_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGDB_LIBRARY}
)

IF(COLLADA_FOUND)
  TARGET_LINK_LIBRARIES( ${TARGET_NAME}	${COLLADA14DOM_LIBRARY})
ENDIF(COLLADA_FOUND)

# Link to zlib.
IF(ZLIB_FOUND)
	TARGET_LINK_LIBRARIES( ${TARGET_NAME} ${ZLIB_LIBRARIES} minizip )
ENDIF(ZLIB_FOUND)
