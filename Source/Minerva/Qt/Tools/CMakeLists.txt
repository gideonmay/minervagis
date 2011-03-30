
# Set Include Folders
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} 
                     ${QT_INCLUDE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

# Add definitions
ADD_DEFINITIONS ("-D_COMPILING_MINERVA_QT_TOOLS")

SET(HEADERS
  ./Color.h
  ColorButton.h
  ./Export.h
  FileDialog.h
  Image.h
  Move.h
  ScopedSignals.h
  Show.h
  StringsView.h
)

# List the Sources.
SET (SOURCES
  ColorButton.cpp
  FileDialog.cpp
  Image.cpp
  Move.cpp
  Show.cpp
  StringsView.cpp
)

# These need to be moc'd.
SET(MOC_HDRS
  ColorButton.h
)

# Create the moc files.
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS} )

# Create a Shared Library
ADD_LIBRARY( QtTools SHARED ${SOURCES} ${HEADERS} ${MOC_SRCS} )

# Add a library
CADKIT_ADD_LIBRARY ( QtTools )

# Link the Library	
LINK_CADKIT( QtTools Usul )

TARGET_LINK_LIBRARIES( QtTools ${QT_LIBRARIES} )

INSTALL(TARGETS QtTools
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Add source directory.
SOURCE_GROUP ( Source REGULAR_EXPRESSION ./* )
