#  CMakeLists for OsgTools.

ADD_DEFINITIONS("-D_COMPILING_MINERVA_OSG_TOOLS")

INCLUDE_DIRECTORIES( "${PROJECT_SOURCE_DIR}/../../" ${Boost_INCLUDE_DIR} ${OSG_INC_DIR} )

#  List the headers.
SET ( HEADERS
./ConvertMatrix.h
./ConvertMatrixTransform.h
./ConvertToTriangles.h
./ConvertVector.h
./Defaults.h
./Export.h
./FBOScreenCapture.h
./Font.h
./FrameDump.h
./Group.h
./Legend.h
./LegendImage.h
./LegendItem.h
./LegendObject.h
./LegendText.h
./OffScreenRenderer.h
./OffScreenRendererPBuffer.h
./Ray.h
./ScopedNodeMask.h
./ScopedProjection.h
./ScopedViewport.h
./ShapeFactory.h
./SortBackToFront.h
./StateSet.h
./TiledScreenCapture.h
./Torus.h
./Visitor.h
)


#########################################################
#
#  List the Sources.
#
#########################################################

SET (SOURCES
./ConvertToTriangles.cpp
./FBOScreenCapture.cpp
./Font.cpp
./FrameDump.cpp
./Group.cpp
./Legend.cpp
./LegendImage.cpp
./LegendItem.cpp
./LegendObject.cpp
./LegendText.cpp
./OffScreenRenderer.cpp
./OffScreenRendererPBuffer.cpp
./Ray.cpp
./ShapeFactory.cpp
./SortBackToFront.cpp
./StateSet.cpp
./TiledScreenCapture.cpp
./Torus.cpp
)

# Set the target name.
SET ( TARGET_NAME MinervaOsgTools )
SET ( TARGET_FILES ${HEADERS} ${SOURCES} )

# Create a Shared Library
ADD_LIBRARY(${TARGET_NAME} SHARED ${TARGET_FILES} )

# Add a library
CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

INSTALL ( FILES ${HEADERS} DESTINATION include/cadkit/Minerva/OsgTools )

# Link to these Cadkit libraries.
LINK_CADKIT(${TARGET_NAME} Usul)

# Link to these 3rd party librarise.
TARGET_LINK_LIBRARIES( ${TARGET_NAME}
	${Boost_LIBRARIES}
	${OPENGL_LIBRARIES}
	${OPENTHREADS_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGDB_LIBRARY}
  ${OSGTEXT_LIBRARY}
  ${OSGUTIL_LIBRARY}
  ${OSGVIEWER_LIBRARY}
)

