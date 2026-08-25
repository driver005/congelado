module;

#include "c/extern/env/dynamic_library.h"

export module cc_abi_builder_env:dynamic_library;

import cc_abi_builder_intern;


export namespace ice {

class DynamicLibrary {
 public:
  static void* load(const char* library_filename, TF_Status* status) {

    return TF_LoadSharedLibrary(library_filename, status);

  }

  static void* get_symbol(void* handle, const char* symbol_name, TF_Status* status) {

    return TF_GetSymbolFromLibrary(handle, symbol_name, status);

  }
};

}  // namespace ice
