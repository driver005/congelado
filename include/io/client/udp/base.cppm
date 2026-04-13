export module io_client.udp;

export import :types;

#if defined(_WIN32)
export import :win32;
#else
export import :posix;
#endif
