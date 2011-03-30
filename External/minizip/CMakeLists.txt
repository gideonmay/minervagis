

INCLUDE_DIRECTORIES( ${ZLIB_INCLUDE_DIR} )

#IF(MSVC)
#	ADD_DEFINITIONS ( "-DZLIB_WINAPI" )
#ENDIF(MSVC)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC" )
  SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" )
ENDIF()
	
#List the Sources
SET (SOURCES
./mztools.c
./unzip.c
./ioapi.c
./zip.c
)

IF(WIN32)
  SET ( SOURCES ${SOURCES} ./minizip.def )
  SET ( SOURCES ${SOURCES} ./iowin32.c )
ENDIF()

# Create a Shared Library
ADD_LIBRARY(minizip STATIC ${SOURCES} )
CADKIT_ADD_LIBRARY ( minizip )

# Link the Library  
TARGET_LINK_LIBRARIES(minizip ${ZLIB_LIBRARIES} )
