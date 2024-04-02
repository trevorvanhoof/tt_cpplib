#include "tt_math.h"
#include "tt_cgmath.h"
#include <cmath>

namespace TT {

#define SPECIAL(T) \
	template<> T clamp(T v, T n, T x) { return _mm_max_ps(_mm_min_ps(v, x), n); } \
	template<> T min(T a, T b) { return _mm_min_ps(a, b); } \
	template<> T max(T a, T b) { return _mm_max_ps(a, b); } \
	template<> T floor(T a) { return _mm_floor_ps(a); } \
	template<> T ceil(T a) { return _mm_ceil_ps(a); } \
	template<> T mod(T a, T b) { return Vec(a) % b; }

	SPECIAL(__m128)
	SPECIAL(Vec)
	SPECIAL(Vec2)
	SPECIAL(Vec3)
	SPECIAL(Vec4)

#undef SPECIAL

	template<> float mod(float a, float b) {
		return a - b * std::floor(a / b);
	}

	template<> double mod(double a, double b) {
		return a - b * std::floor(a / b);
	}

	size_t hashCombine(size_t a, size_t b) {
		return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
	}
}
