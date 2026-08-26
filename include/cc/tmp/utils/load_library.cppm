module;

#include <string>

export module cc_tmp:utils_load_library;

import std;
import cc_abi;

export {

    namespace tensorflow {

        class LoadLibrary
        {
        public:
            static ice::DynamicLibrary Load(const std::string& path)
            {
                return ice::DynamicLibrary(path);
            }
        };

    } // namespace tensorflow

} // export
