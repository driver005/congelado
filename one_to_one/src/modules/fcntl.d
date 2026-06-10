module modules.fcntl;
@nogc nothrow:

// Re-export fcntl() from the POSIX binding, mirroring the C++ module that
// wraps <fcntl.h> and exports only ::fcntl.

public import core.sys.posix.fcntl : fcntl;
