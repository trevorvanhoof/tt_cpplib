#pragma once

#include <xmmintrin.h>
#include <algorithm>
#include "tt_cgmath.h"

namespace TT {
	template<typename T> T clamp(T v, T n, T x) {
		return v < n ? n : (v > x ? x : v);
	}

	template<typename T> T min(T a, T b) {
		return a < b ? a : b;
	}

	template<typename T> T max(T a, T b) {
		return a > b ? a : b;
	}

	template<typename T> T mod(T a, T b) {
		return (b + (a % b)) % b;
	}

	template<typename T> T fract(T a) {
		return mod(a, (T)1.0);
	}

	template<typename T> T lerp(T a, T b, float w) {
		return (b - a) * w + a;
	}

	template<typename T> T floor(T a) {
		return std::floor(a);
	}

	template<typename T> T ceil(T a) {
		return std::ceil(a);
	}

#define SPECIAL(T) \
	template<> T clamp(T v, T n, T x); \
	template<> T min(T a, T b); \
	template<> T max(T a, T b); \
	template<> T floor(T a); \
	template<> T ceil(T a); \
	template<> T mod(T a, T b);
	SPECIAL(__m128)
	SPECIAL(Vec)
	SPECIAL(Vec2)
	SPECIAL(Vec3)
	SPECIAL(Vec4)
#undef SPECIAL

	template<> float mod(float a, float b);
	template<> double mod(double a, double b);

	size_t hashCombine(size_t a, size_t b);
}
