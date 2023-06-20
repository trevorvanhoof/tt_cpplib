#pragma once

#include <cmath>
#include <xmmintrin.h>
#include <smmintrin.h>

namespace TT {
	enum class ERotateOrder {
		XYZ = 0b00'01'10,
		XZY = 0b00'10'01,
		YXZ = 0b01'00'10,
		YZX = 0b01'10'00,
		ZXY = 0b10'00'01,
		ZYX = 0b10'01'00,
	};

	__declspec(align(16)) struct Vec {
		union {
			struct {
				float x;
				float y;
				float z;
				float w;
			};
			__m128 m;
		};
		Vec();
		Vec(float s);
		Vec(float x, float y, float z, float w);
		Vec(__m128 m);
		operator __m128() const;
		Vec dot(__m128 other) const;
		Vec sqrLen() const;
		Vec len() const;
		Vec normalized() const;
		Vec operator+() const;
		Vec operator-() const;
		Vec operator*(float other) const;
		Vec operator/(float other) const;
		Vec operator+(float other) const;
		Vec operator-(float other) const;
		Vec operator%(float other) const;
		Vec operator*(__m128 other) const;
		Vec operator/(__m128 other) const;
		Vec operator+(__m128 other) const;
		Vec operator-(__m128 other) const;
		Vec operator%(__m128 other) const;
		// comparison operators return all 0 or all 1 bits to indicate false / true
		Vec operator<(__m128 other) const;
		Vec operator>(__m128 other) const;
		Vec operator<=(__m128 other) const;
		Vec operator>=(__m128 other) const;
		Vec operator==(__m128 other) const;
		Vec operator!=(__m128 other) const;
		Vec& normalize();
		Vec& operator*=(float other);
		Vec& operator/=(float other);
		Vec& operator+=(float other);
		Vec& operator-=(float other);
		Vec& operator%=(float other);
		Vec& operator*=(__m128 other);
		Vec& operator/=(__m128 other);
		Vec& operator+=(__m128 other);
		Vec& operator-=(__m128 other);
		Vec& operator%=(__m128 other);
		Vec swizzle(unsigned char a, unsigned char b, unsigned char c, unsigned char d) const;
	};

	struct Vec2 : public Vec {
		Vec2() : Vec() {}
		Vec2(float s) : Vec(s) {}
		Vec2(float* xy) : Vec(xy[0], xy[1], 0.0f, 0.0f) {}
		Vec2(float x, float y) : Vec(x, y, 0.0f, 0.0f) {}
		Vec2(__m128 m) : Vec(m) {}
		Vec2(Vec m) : Vec(m.m) {}
	};
	struct Vec3 : public Vec {
		Vec3() : Vec() {}
		Vec3(float s) : Vec(s) {}
		Vec3(float* xyz) : Vec(xyz[0], xyz[1], xyz[2], 0.0f) {}
		Vec3(float x, float y, float z) : Vec(x, y, z, 0.0f) {}
		Vec3(__m128 m) : Vec(m) {}
		Vec3(Vec m) : Vec(m.m) {}
		Vec3 cross(Vec3 b) { return Vec3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
	};
	struct Vec4 : public Vec {
		Vec4() : Vec() {}
		Vec4(float s) : Vec(s) {}
		Vec4(float* xyzw) : Vec(xyzw[0], xyzw[1], xyzw[2], xyzw[3]) {}
		Vec4(float x, float y, float z, float w) : Vec(x, y, z, w) {}
		Vec4(__m128 m) : Vec(m) {}
		Vec4(Vec m) : Vec(m.m) {}
	};

	__declspec(align(16))
	struct Mat22 {
		__m128 m;
	};

	__declspec(align(16))
	struct Mat33 {
		float m[9];
	};

	__declspec(align(16))
	struct Mat44 {
		union {
			// the gl spec writes down columns
			// but in memory they are stored column major
			__m128 col[4];
			float m[16];
		};

		Mat44& operator*=(const Mat44& b);
		Mat44 operator*(const Mat44& b) const;
		Vec4 operator*(const Vec4& b) const;
		static Mat44 frustum(float left, float right, float top, float bottom, float near, float far);
		static Mat44 perspectiveY(float fovRadians, float aspect, float near, float far);
		static Mat44 translate(float x, float y, float z);
		static Mat44 scale(float x, float y, float z);
		static Mat44 rotateX(float radians);
		static Mat44 rotateY(float radians);
		static Mat44 rotateZ(float radians);
		static Mat44 rotate(float radiansX, float radiansY, float radiansZ, ERotateOrder order = ERotateOrder::YXZ);
		static Mat44 rotate(Vec radians, ERotateOrder order = ERotateOrder::YXZ);
		static Mat44 TRS(float x = 0.0f, float y = 0.0f, float z = 0.0f, float radiansX = 0.0f, float radiansY = 0.0f, float radiansZ = 0.0f, float sx = 1.0f, float sy = 1.0f, float sz = 1.0f, ERotateOrder order = ERotateOrder::YXZ);
		static Mat44 TRS(Vec translate, Vec radians, Vec scake, ERotateOrder order = ERotateOrder::YXZ);
		void inverse();
		void transpose();
		Mat44 inversed() const;
		Mat44 transposed() const;
	};

	extern const float PI;
	extern const float TAU;
	extern const float DEG2RAD;
	extern const double PId;
	extern const double TAUd;
	extern const double DEG2RADd;
	extern const Mat44 IDENTITY;
}
