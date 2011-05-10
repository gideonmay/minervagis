
# Add needed preprocessor definitions.
ADD_DEFINITIONS("-D_COMPILING_MINERVA_DOCUMENT")

IF(MSVC)
	ADD_DEFINITIONS("-DNOMINMAX -D_CRT_SECURE_NO_DEPRECATE")
ENDIF(MSVC)

# Add directories to search for libraries to link to.
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# Set Include Folders
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR}
		     ${CADKIT_INC_DIR}/GenericNurbs
		     ${OSG_INC_DIR} 
		     )

SET ( HEADERS
./AnimationController.h
./CameraPath.h
./CameraPathController.h
./CurvePlayer.h
./Export.h
./MinervaDocument.h
./OffScreenView.h
./SceneView.h
./PathBuilder.h
./View.h
)

#List the Sources
SET (SOURCES
./AnimationController.cpp
./CameraPath.cpp
./CameraPathController.cpp
./CurvePlayer.cpp
./MinervaDocument.cpp
./OffScreenView.cpp
./SceneView.cpp
./PathBuilder.cpp
./View.cpp
)

# Create a Shared Library
ADD_LIBRARY( MinervaDocument SHARED ${HEADERS} ${SOURCES} )

CADKIT_ADD_LIBRARY ( MinervaDocument )
			
# Link the Library	
LINK_CADKIT( MinervaDocument Usul MinervaOsgTools SerializeXML XmlTree MinervaCore )
TARGET_LINK_LIBRARIES( MinervaDocument
	${Boost_LIBRARIES}
	${OPENTHREADS_LIB}
  ${OSG_LIBRARY}
  ${OSGDB_LIBRARY}
  ${OSGUTIL_LIBRARY}
  ${OSGGA_LIBRARY}
)

INSTALL ( FILES ${HEADERS} DESTINATION include/cadkit/Minerva/Document )
