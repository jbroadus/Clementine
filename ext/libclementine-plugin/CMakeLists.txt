include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(COMMON_SOURCES
  common/pluginbase.cpp
)

set(HEADERS
  interfaces/service.h
  interfaces/player.h
  common/pluginbase.h
)

qt5_wrap_cpp(MOC ${HEADERS})


# Plugin library
add_library(clementine-plugin STATIC
  ${COMMON_SOURCES}
  ${MOC}
)

target_link_libraries(clementine-plugin
  Qt5::Core
)


# Host library
add_library(clementine-plugin-host STATIC
  plugin-host/init.cpp
  ${COMMON_SOURCES}
  ${MOC}
  )

target_link_libraries(clementine-plugin-host
  Qt5::Core
)

if (USE_PLUGINS_STATIC)
  target_link_libraries(clementine-plugin-host
    PUBLIC clementine-plugin-bundle)
endif (USE_PLUGINS_STATIC)

add_subdirectory(plugins)
