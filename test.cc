#include <functional>
#include <iostream>

int main() {
#ifdef __cpp_lib_move_only_function
    std::cout << "Supported! Value: " << __cpp_lib_move_only_function << std::endl;
#else
    std::cout << "Not supported by your current libc++ version." << std::endl;
#endif
    return 0;
}
