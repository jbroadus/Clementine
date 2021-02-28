include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(BASE_SOURCES
  base/servicebase.cpp
)

set(INTERFACE_HEADERS
  interfaces/player.h
)

set(BASE_HEADERS
  base/servicebase.h
)

qt5_wrap_cpp(INTERFACE_MOC ${INTERFACE_HEADERS})
qt5_wrap_cpp(BASE_MOC ${BASE_HEADERS})


# Plugin library
add_library(clementine-plugin STATIC
  ${BASE_SOURCES}
  ${BASE_MOC}
  ${INTERFACE_MOC}
)

target_link_libraries(clementine-plugin
  Qt5::Core
)


# Host library
add_library(clementine-plugin-host STATIC
  ${INTERFACE_MOC}
)

target_link_libraries(clementine-plugin-host
  Qt5::Core
)

add_subdirectory(plugins)
