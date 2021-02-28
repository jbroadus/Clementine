# The telnet plugin is intended to be a simple example.
option(INCLUDE_TELNET_PLUGIN "Telnet control plugin." OFF)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

if(INCLUDE_TELNET_PLUGIN)
  add_subdirectory(telnet)
endif(INCLUDE_TELNET_PLUGIN)
