module modules.unistd;
@nogc nothrow:

// Re-export the three POSIX functions used by this codebase, mirroring the C++
// module that wraps <unistd.h> and exports only close, read, write.

public import core.sys.posix.unistd : close, read, write;
