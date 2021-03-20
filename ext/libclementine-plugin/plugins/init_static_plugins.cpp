#include <QtPlugin>

extern const QStaticPlugin qt_static_plugin_ExamplePlugin();


__attribute__((constructor))
void RegisterStaticPlugins() {
  qRegisterStaticPluginFunction(qt_static_plugin_ExamplePlugin());
}
