#ifndef UTILS_H
#define UTILS_H

// Marks an unreachable point of execution for optimization and debugging
// Copied from the possible implementation section of:
// https://en.cppreference.com/w/cpp/utility/unreachable
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  #define unreachable() (__assume(false))
#else // GCC, Clang
  #define unreachable() __builtin_unreachable()
#endif

#endif
