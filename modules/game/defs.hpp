#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define MODULE_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
    #define MODULE_EXPORT __attribute__((visibility("default")))
#else
    #define GDE_EXPORT
#endif
