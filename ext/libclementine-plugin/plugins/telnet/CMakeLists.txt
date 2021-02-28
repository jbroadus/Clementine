include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-plugin)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)

set(SOURCES
  service.cpp
  player.cpp
  )

set(HEADERS
  service.h
  player.h
  )

unset(MOC)
qt5_wrap_cpp(MOC ${HEADERS})

add_library(clementine-plugin-telnet SHARED
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine-plugin-telnet
  Qt5::Core
  Qt5::Network
  clementine-plugin
)
