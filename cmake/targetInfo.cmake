# Function to print the location and include directories of a target
function(print_target_info target_name)
  if(TARGET ${target_name})
    # Get target type
    get_target_property(target_type ${target_name} TYPE)
    message(STATUS "Target ${target_name} is of type: ${target_type}")

    # Print location (for non-INTERFACE libraries)
    if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
      get_target_property(target_location ${target_name} LOCATION)
      if(target_location)
        message(STATUS "Location of target ${target_name}: ${target_location}")
      else()
        get_target_property(imported_location ${target_name} IMPORTED_LOCATION)
        if(imported_location)
          message(STATUS "Imported location of target ${target_name}: ${imported_location}")
        else()
          message(STATUS "Location/Imported location of target ${target_name} is not set or not available.")
        endif()
      endif()
    endif()

    # Print include directories
    get_target_property(target_include_dirs ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
    if(target_include_dirs)
      message(STATUS "Include directories for target ${target_name}:")
      foreach(dir ${target_include_dirs})
        message(STATUS "  ${dir}")
      endforeach()
    else()
      message(STATUS "No INTERFACE_INCLUDE_DIRECTORIES set for target ${target_name}")
    endif()

  else()
    message(STATUS "Target ${target_name} does not exist.")
  endif()
endfunction()

