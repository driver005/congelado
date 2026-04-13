export module io_base_leverage;

export import :types;

#if defined(_WIN32)
export import :win32;
#else
export import :posix;
#endif
