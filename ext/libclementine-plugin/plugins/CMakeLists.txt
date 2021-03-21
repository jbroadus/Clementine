option(INCLUDE_EXAMPLE_PLUGIN "Example telnet control plugin." OFF)

if(USE_PLUGINS_STATIC)
  set(PLUGIN_LIBRARY_TYPE STATIC)
else(USE_PLUGINS_STATIC)
  set(PLUGIN_LIBRARY_TYPE SHARED)
endif(USE_PLUGINS_STATIC)

add_library(clementine-plugin-bundle STATIC
  init_static_plugins.cpp
)
target_link_libraries(clementine-plugin-bundle
  Qt5::Core
)
 
if(INCLUDE_EXAMPLE_PLUGIN)
  add_subdirectory(example)
  target_link_libraries(clementine-plugin-bundle
    PUBLIC clementine-plugin-example
  )
endif(INCLUDE_EXAMPLE_PLUGIN)
