
add_subdirectory(jose)

target_sources(${CMAKE_PROJECT_NAME} PUBLIC
clevisLib.cpp
clevisDecrypt.cpp
clevisEncrypt.cpp
)

if (BUILD_EXECUTABLE)
  target_sources(${CMAKE_PROJECT_NAME} PUBLIC main.cpp)
endif()


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})   
