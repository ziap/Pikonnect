#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// Marks an unreachable point of execution for optimization and debugging
// Copied from the possible implementation section of:
// https://en.cppreference.com/w/cpp/utility/unreachable
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  #define unreachable (__assume(false))
#else // GCC, Clang
  #define unreachable __builtin_unreachable()
#endif

extern float smoothstep(float t);
extern float fbm(float x, float c[5]);

extern uint64_t fasthash64(const void *buf, uint64_t len, uint64_t seed);
extern uint32_t pcg32(uint64_t *state);
extern uint32_t pcg32_bounded(uint64_t *state, uint32_t range);

#endif
