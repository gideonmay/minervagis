
add_subdirectory ( Common )
add_subdirectory ( Network )
add_subdirectory ( Core )
add_subdirectory ( OsgTools )
add_subdirectory ( Document )

# Add the plugins
add_subdirectory ( Plugins )

IF(QT_FOUND)

  add_subdirectory(Qt)

ENDIF(QT_FOUND)
