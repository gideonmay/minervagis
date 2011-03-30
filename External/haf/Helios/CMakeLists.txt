
include_directories ( "${PROJECT_SOURCE_DIR}/External/haf" )

add_definitions ( "-D_COMPILING_HELIOS" )

SET ( SOURCES 
./Menus/Action.cpp
./Menus/Builder.cpp
./Menus/Button.cpp
./Menus/Item.cpp
./Menus/Menu.cpp
./Menus/MenuAdapter.cpp
./Menus/MenuBar.cpp
./Menus/Separator.cpp
./Menus/ToolBar.cpp
./Menus/Visitor.cpp
)

SET ( HEADERS
./Menus/Action.h
./Menus/Builder.h
./Menus/Button.h
./Menus/Functors.h
./Menus/Item.h
./Menus/Menu.h
./Menus/MenuAdapter.h
./Menus/MenuBar.h
./Menus/Separator.h
./Menus/ToolBar.h
./Menus/Visitor.h
)

SET(MOC_HDRS
./Menus/Action.h
./Menus/MenuAdapter.h
)

QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

SET ( TARGET_NAME Helios )

ADD_LIBRARY( ${TARGET_NAME} SHARED ${SOURCES} ${HEADERS} ${MOC_SRCS} )

TARGET_LINK_LIBRARIES( ${TARGET_NAME} ${QT_LIBRARIES} )
TARGET_LINK_LIBRARIES( ${TARGET_NAME} Usul )

  INSTALL(TARGETS ${TARGET_NAME}
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib )
