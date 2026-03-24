module;
#include <unistd.h>
export module modules.unistd;

export {
    using ::close;
    using ::read;
    using ::write;
}
