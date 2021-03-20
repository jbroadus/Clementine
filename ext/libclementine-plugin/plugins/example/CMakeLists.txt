include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-plugin)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)

set(SOURCES
  example.cpp
  exampleplayer.cpp
  )

set(HEADERS
  example.h
  exampleplayer.h
  )

if (USE_STATIC_PLUGINS)
  add_definitions(-DQT_STATICPLUGIN)
endif (USE_STATIC_PLUGINS)

unset(MOC)
qt5_wrap_cpp(MOC ${HEADERS})

add_library(clementine-plugin-example ${PLUGIN_LIBRARY_TYPE}
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine-plugin-example
  Qt5::Core
  Qt5::Network
  clementine-plugin
)
