#ifndef TENSORFLOW_C_PLATFORM_H_
#define TENSORFLOW_C_PLATFORM_H_

// Platform detection
#if defined(__ANDROID__) || defined(__IOS__) || defined(__EMSCRIPTEN__)
#    define IS_MOBILE_PLATFORM 1
#endif

#endif // TENSORFLOW_C_PLATFORM_H_
