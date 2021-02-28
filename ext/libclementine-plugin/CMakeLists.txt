include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(plugins)

set(SOURCES
  common/pluginbase.cpp
)

set(HEADERS
  interfaces/service.h
  interfaces/player.h
  common/pluginbase.h
)

qt5_wrap_cpp(MOC ${HEADERS})

add_library(libclementine-plugin STATIC
  ${SOURCES}
  ${MOC}
)

target_link_libraries(libclementine-plugin
  Qt5::Core
)
