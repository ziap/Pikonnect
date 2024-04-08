#include "utils.h"

// Smoothstep function: https://en.wikipedia.org/wiki/Smoothstep
float smoothstep(float t) {
  return t * t * (3.0f - 2.0f * t);
}

// Bit mixing function for fasthash64
static uint64_t mix(uint64_t h) {
  h ^= h >> 23;
  h *= 0x2127599bf4325c37;
  h ^= h >> 47;
  return h;
}

// Fast but not secure hash function
// Copied from: https://github.com/ztanml/fast-hash/
// Line: 35
uint64_t fasthash64(const void *buf, uint64_t len, uint64_t seed) {
	const uint64_t    m = 0x880355f21e6d1965;
	const uint64_t *pos = (const uint64_t *)buf;
	const uint64_t *end = pos + (len / 8);
	const uint8_t *pos2;
	uint64_t h = seed ^ (len * m);
	uint64_t v;

	while (pos != end) {
		v  = *pos++;
		h ^= mix(v);
		h *= m;
	}

	pos2 = (const uint8_t*)pos;
	v = 0;

	switch (len & 7) {
	case 7: v ^= (uint64_t)pos2[6] << 48;
	case 6: v ^= (uint64_t)pos2[5] << 40;
	case 5: v ^= (uint64_t)pos2[4] << 32;
	case 4: v ^= (uint64_t)pos2[3] << 24;
	case 3: v ^= (uint64_t)pos2[2] << 16;
	case 2: v ^= (uint64_t)pos2[1] << 8;
	case 1: v ^= (uint64_t)pos2[0];
		h ^= mix(v);
		h *= m;
	}

	return mix(h);
}

// Permuted congruential generator
// An algorithm for generating pseudorandom numbers
// Copied and modified from: https://www.pcg-random.org/download.html
uint32_t pcg32(uint64_t *state) {
  uint64_t s = *state;
  *state = s * 0x5851f42d4c957f2d + 0x14057b7ef767814f;

  uint32_t xorshifted = ((s >> 18u) ^ s) >> 27u;
  uint32_t rot = s >> 59u;
  return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// Unbiased random integer generation in a range, Lemire's method
// Paper: https://arxiv.org/pdf/1805.10941.pdf
uint32_t pcg32_bounded(uint64_t *state, uint32_t range) {
  uint32_t t = (-range) % range;
  uint32_t x, l;
  uint64_t m;

  do {
    x = pcg32(state);
    m = (uint64_t)x * range;
    l = (uint32_t)m;
  } while (l < t);

  return m >> 32;
}

