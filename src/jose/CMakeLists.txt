target_sources(${CMAKE_PROJECT_NAME} PUBLIC
  joseCommon.cpp
  joseClevisDecrypt.cpp
  joseClevisEncrypt.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})   
