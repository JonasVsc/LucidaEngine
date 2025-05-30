#pragma once

// lib
#include <fmt/core.h>

#ifdef DEBUG

#define jinfo(...) do { \
    fmt::print("\033[1;38;2;128;128;128m[INFO] "); \
    fmt::println(__VA_ARGS__); \
    fmt::print("\033[0m"); \
} while (0)

#define jdebug(...) do { \
    fmt::print("\033[1;38;2;128;255;128m[DEBUG] "); \
    fmt::println(__VA_ARGS__); \
    fmt::print("\033[0m"); \
} while (0)

#define jwarn(...) do { \
    fmt::print("\033[1;2;38;2;255;255;128m[WARN] "); \
    fmt::println(__VA_ARGS__); \
    fmt::print("\033[0m"); \
} while (0)

#define jerr(...) do { \
    fmt::print("\033[1;2;38;2;255;128;128m[ERR] "); \
    fmt::println(__VA_ARGS__); \
    fmt::print("\033[0m"); \
} while (0)

#else

#define jinfo(...)
#define jdebug(...)
#define jwarn(...) 
#define jerr(...)

#endif