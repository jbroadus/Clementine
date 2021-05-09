include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(COMMON_SOURCES
  common/pluginbase.cpp
)

set(HEADERS
  interfaces/service.h
  interfaces/player.h
  interfaces/settings.h
)

set(COMMON_HEADERS
  common/pluginbase.h
  common/settingsbase.h
)

qt5_wrap_cpp(MOC ${HEADERS})
qt5_wrap_cpp(COMMON_MOC ${COMMON_HEADERS})


# Plugin library
add_library(clementine-plugin STATIC
  ${COMMON_SOURCES}
  ${MOC}
  ${COMMON_MOC}
)

target_link_libraries(clementine-plugin
  Qt5::Core
)


# Host library
add_library(clementine-plugin-host STATIC
  plugin-host/init.cpp
  ${MOC}
  )

target_link_libraries(clementine-plugin-host
  Qt5::Core
)

if (USE_PLUGINS_STATIC)
  target_link_libraries(clementine-plugin-host
    clementine-plugin-bundle)
endif (USE_PLUGINS_STATIC)

add_subdirectory(plugins)
