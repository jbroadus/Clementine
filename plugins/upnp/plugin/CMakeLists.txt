include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(SOURCES
    core/upnpaction.cpp
    core/upnpevents.cpp
    core/upnpmanager.cpp
    core/upnproot.cpp
    core/upnpserver.cpp
    core/upnpservice.cpp
    core/scpdparser.cpp
    core/scpdbuilder.cpp
    services/avtransport.cpp
)

set(HEADERS
    core/upnpevents.h
    core/upnpmanager.h
    core/upnproot.h
    core/upnpserver.h
)

set(RESOURCES
    resources/upnp.qrc
)

qt5_wrap_cpp(MOC ${HEADERS})
qt5_add_resources(QRC ${RESOURCES})

add_library(clementine-upnp SHARED
  ${SOURCES}
  ${MOC}
  ${QRC}
)

target_link_libraries(clementine-upnp
  ${QT_LIBRARIES}
  ${LIBUPNP_LIBRARIES}
  clementine-upnp-interface
)

