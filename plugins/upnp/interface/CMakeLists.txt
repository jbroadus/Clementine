set(INTERFACE_HEADERS
  upnpplugin.h
)

qt5_wrap_cpp(INTERFACE_MOC ${INTERFACE_HEADERS})

add_library(clementine-upnp-interface STATIC
  ${INTERFACE_MOC}
)

target_link_libraries(clementine-upnp-interface
  Qt5::Core
)
