project(example-plugin LANGUAGES CXX)
cmake_minimum_required(VERSION 3.1.0)

set(QT_MIN_VERSION 5.6.0)
find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Core REQUIRED)

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

qt5_wrap_cpp(MOC ${HEADERS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_library(example-plugin SHARED
  ${SOURCES}
  ${MOC}
)

target_link_libraries(example-plugin
  Qt5::Core
  Qt5::Network
  libclementine-plugin
)
