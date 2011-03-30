
# Find needed libraries.
INCLUDE( FindVRJuggler )

FIND_PACKAGE(OpenGL)

LINK_DIRECTORIES( "$ENV{VJ_DEPS_DIR}/lib" )

ADD_DEFINITIONS("-D_COMPILING_VRV")

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES( 
		     ${CADKIT_INC_DIR}
		     ${OSG_INC_DIR} 
		     ${OT_INC_DIR} 
		     ${VR_JUGGLER_INCLUDES}
		     ${VRJ_INC_DIR}
		     "$ENV{VJ_DEPS_DIR}/include"
		     ${CPPDOM_INC_DIR}
		     ${GMTL_INC_DIR}
		     ${Boost_INCLUDE_DIR}
		     ${CMAKE_CURRENT_SOURCE_DIR}
		     )

SET ( HEADERS
./VRV/Devices/Wingman.h
./VRV/Devices/Callback.h
./VRV/Devices/JoystickDevice.h
./VRV/Devices/Messages.h
./VRV/Devices/ButtonDevice.h
./VRV/Devices/TrackerDevice.h
./VRV/Devices/Referenced.h
./VRV/Core/SharedData.h
./VRV/Core/Program.h
./VRV/Core/BaseApplication.h
./VRV/Core/JugglerFunctors.h
./VRV/Core/Exceptions.h
./VRV/Core/Application.h
./VRV/Core/VprIO.h
./VRV/Core/Navigator.h
./VRV/Export.h
./VRV/Common/Buttons.h
./VRV/Common/Libraries.h
./VRV/Common/Constants.h
)

#List the Sources
SET (SOURCES
./main.cpp
./VRV/Devices/JoystickDevice.cpp
./VRV/Devices/ButtonDevice.cpp
./VRV/Devices/TrackerDevice.cpp
./VRV/Devices/Callback.cpp
./VRV/Devices/Wingman.cpp
./VRV/Core/BaseApplication.cpp
./VRV/Core/Application.cpp
./VRV/Core/Navigator.cpp
)

# Create a Shared Library
ADD_EXECUTABLE(MinervaVR ${SOURCES} ${HEADERS} )

# Add the debug postfix.
SET_TARGET_PROPERTIES(MinervaVR PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

# Link the Library	
LINK_CADKIT( MinervaVR Usul XmlTree MinervaOsgTools MinervaDocument )
TARGET_LINK_LIBRARIES( MinervaVR
	${OPENGL_LIBRARIES}
	${OSG_LIB}
	${OSG_DB_LIB}
	${OSG_UTIL_LIB}
	${VRJ_LIBRARY}
	${VRJ_OGL_LIBRARY}
	${VPR_LIBRARY}
	${GADGET_LIBRARY}
	${Boost_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARY}
)

INSTALL(TARGETS MinervaVR
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION bin
    ARCHIVE DESTINATION bin )
