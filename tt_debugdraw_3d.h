#pragma once

#include "tt_globjects.h"
#include "tt_debugdraw_2d.h"
#include <immintrin.h>

namespace TT {
	__declspec(align(16))
		struct CameraMatrix {
		__m128 cols[4];

		CameraMatrix operator*(const CameraMatrix& other) {
			return {
			_mm_add_ps(_mm_add_ps(_mm_mul_ps(other.cols[0], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[0]), 0b00000000))),
				_mm_mul_ps(other.cols[1], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[0]), 0b01010101)))),
				_mm_add_ps(_mm_mul_ps(other.cols[2], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[0]), 0b10101010))),
					_mm_mul_ps(other.cols[3], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[0]), 0b11111111))))),
			_mm_add_ps(_mm_add_ps(_mm_mul_ps(other.cols[0], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[1]), 0b00000000))),
				_mm_mul_ps(other.cols[1], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[1]), 0b01010101)))),
				_mm_add_ps(_mm_mul_ps(other.cols[2], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[1]), 0b10101010))),
					_mm_mul_ps(other.cols[3], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[1]), 0b11111111))))),
			_mm_add_ps(_mm_add_ps(_mm_mul_ps(other.cols[0], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[2]), 0b00000000))),
				_mm_mul_ps(other.cols[1], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[2]), 0b01010101)))),
				_mm_add_ps(_mm_mul_ps(other.cols[2], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[2]), 0b10101010))),
					_mm_mul_ps(other.cols[3], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[2]), 0b11111111))))),
			_mm_add_ps(_mm_add_ps(_mm_mul_ps(other.cols[0], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[3]), 0b00000000))),
				_mm_mul_ps(other.cols[1], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[3]), 0b01010101)))),
				_mm_add_ps(_mm_mul_ps(other.cols[2], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[3]), 0b10101010))),
					_mm_mul_ps(other.cols[3], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(cols[3]), 0b11111111)))))
			};
		}

		__m128 operator*(__m128 v) {
			return { _mm_add_ps(_mm_add_ps(_mm_mul_ps(cols[0], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v), 0b00000000))),
				_mm_mul_ps(cols[1], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v), 0b01010101)))),
				_mm_add_ps(_mm_mul_ps(cols[2], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v), 0b10101010))),
					_mm_mul_ps(cols[3], _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v), 0b11111111))))) };
		}
	};

	struct DebugDraw3D : public DebugDraw2D {
		CameraMatrix VP;
		void transformPoint(float x, float y, float z, int& xPx, int& yPx);

		void setCamera(CameraMatrix view, CameraMatrix proj);
		void drawPoint3D(float x, float y, float z, int sizePx, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawPolygon3D(const std::vector<float>& points, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawLine3D(float x0, float y0, float z0, float x1, float y1, float z1, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawText3D(float x, float y, float z, const char* text, int r = 255, int g = 255, int b = 255, int a = 255);
	};
}
