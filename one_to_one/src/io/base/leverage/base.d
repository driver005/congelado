module io.base.leverage;
@nogc nothrow:

public import io.base.leverage.types;

version (Windows) {
    public import io.base.leverage.win32;
    // PORT-NOTE: on Windows, use Win32Leverager as the concrete Leverager type
    alias DefaultLeverager = Win32Leverager;
} else {
    public import io.base.leverage.posix;
    // PORT-NOTE: on Linux/POSIX, use PosixLeverager as the concrete Leverager type
    alias DefaultLeverager = PosixLeverager;
}
