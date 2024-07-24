target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  joseCommon.cpp
  joseClevisDecrypt.cpp
  joseClevisEncrypt.cpp
  joseTang.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})   
