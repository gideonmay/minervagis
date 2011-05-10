
function(minerva_find_library module hint_path)
   
  string(TOUPPER ${module} module_uc)

   find_library(${module_uc}_LIBRARY_RELEASE
       NAMES ${module}
       HINTS
            ${hint_path}
       PATH_SUFFIXES lib64 lib
   )

   find_library(${module_uc}_LIBRARY_DEBUG
       NAMES ${module}d
       HINTS
            ${hint_path}
       PATH_SUFFIXES lib64 lib
    )

   if(NOT ${module_uc}_LIBRARY_DEBUG)
      # They don't have a debug library
      set(${module_uc}_LIBRARY_DEBUG ${${module_uc}_LIBRARY_RELEASE} PARENT_SCOPE)
      set(${module_uc}_LIBRARY ${${module_uc}_LIBRARY_RELEASE} PARENT_SCOPE)
   else()
      # They really have a FOO_LIBRARY_DEBUG
      set(${module_uc}_LIBRARY 
          optimized ${${module_uc}_LIBRARY_RELEASE}
          debug ${${module_uc}_LIBRARY_DEBUG}
          PARENT_SCOPE
      )
   endif()

  mark_as_advanced(${module_uc}_LIBRARY)
  mark_as_advanced(${module_uc}_LIBRARY_DEBUG)
  mark_as_advanced(${module_uc}_LIBRARY_RELEASE)

endfunction(minerva_find_library library hint_path)

