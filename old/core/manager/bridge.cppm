export module core_plugin:bridge;
// PythonBridge/LuaBridge moved to plugins/python_bridge and plugins/lua_bridge — bridges are
// plugins now, same as serde formats (plugins/json, plugins/toml): core_plugin no longer
// hard-#includes <Python.h>/<lua.hpp> for every consumer, whether or not they ever touch FFI.
