module;

#include <iostream>
#include <string>

export module cc_tmp:utils_logging;

import std;
import cc_abi;

export {

    namespace tensorflow {

        class Logging
        {
        public:
            static void LogInfo(const std::string& msg)
            {
                std::cout << "[INFO] " << msg << std::endl;
            }

            static void LogError(const std::string& msg)
            {
                std::cerr << "[ERROR] " << msg << std::endl;
            }
        };

    } // namespace tensorflow

} // export
