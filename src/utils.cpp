#include "utils.h"

// Smoothstep function: https://en.wikipedia.org/wiki/Smoothstep
float easing_cubic(float t) {
  return t * t * (3.0f - 2.0f * t);
}
