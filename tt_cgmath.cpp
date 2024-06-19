#include "tt_cgmath.h"

namespace TT {
	Mat44 Mat44::frustum(float left, float right, float top, float bottom, float near, float far) {
		float A = (right + left) / (right - left);
		float B = (top + bottom) / (top - bottom);
		float C = -(far + near) / (far - near);
		float D = -(2.0f * far * near) / (far - near);
		return { 
			(2.0f * near) / (right - left), 0, A, 0,
			0, -(2.0f * near) / (top - bottom), B, 0,
			0, 0, C, -1,
			0, 0, D, 0
		};
	}

	Mat44 Mat44::orthoSymmetric(float width, float height, float near, float far) {
		return { 2.0f / width, 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / height, 0.0f, 0.0f,
			0.0f, 0.0f, -2.0f / (far - near), 0.0f,
			0.0f, 0.0f, -(far + near) / (far - near), 1.0 };
	}
	
	Mat44 Mat44::perspectiveY(float fovRadians, float aspect, float near, float far) {
		float fH = tanf(fovRadians * 0.5f) * near;
		float fW = fH * aspect;
		return frustum(-fW, fW, -fH, fH, near, far);
	}

    Mat44 Mat44::translate(float x, float y, float z) {
        return {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            x, y, z, 1.0f,
        };
    }

    Mat44 Mat44::translate(Vec translate) {
        return {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            translate.x, translate.y, translate.z, 1.0f,
        };
    }

	Mat44 Mat44::scale(float x, float y, float z) {
		return {
			x, 0.0f, 0.0f, 0.0f,
			0.0f, y, 0.0f, 0.0f,
			0.0f, 0.0f, z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
	}

	Mat44 Mat44::rotateX(float radians) {
		float sa = sinf(radians);
		float ca = cosf(radians);
		return {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, ca, sa, 0.0f,
			0.0f, -sa, ca, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
	}

	Mat44 Mat44::rotateY(float radians) {
		float sa = sinf(radians);
		float ca = cosf(radians);
		return {
			ca, 0.0f, -sa, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			sa, 0.0f, ca, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
	}

	Mat44 Mat44::rotateZ(float radians) {
		float sa = sinf(radians);
		float ca = cosf(radians);
		return {
			ca, sa, 0.0f, 0.0f,
			-sa, ca, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
	}

	Mat44 Mat44::rotate(float radiansX, float radiansY, float radiansZ, ERotateOrder order) {
		Mat44 rotations[3] = {
			Mat44::rotateX(radiansX),
			Mat44::rotateY(radiansY),
			Mat44::rotateZ(radiansZ) 
		};
		int first = (((int)order >> 4) & 0b11);
		int second = (((int)order >> 2) & 0b11);
		int third = (((int)order >> 0) & 0b11);
		return rotations[first] * rotations[second] * rotations[third];
	}

	Mat44 Mat44::rotate(Vec radians, ERotateOrder order) { return Mat44::rotate(radians.x, radians.y, radians.z, order); }

	Mat44 Mat44::TRS(float x, float y, float z, float radiansX, float radiansY, float radiansZ, float sx, float sy, float sz, ERotateOrder order) {
		Mat44 r = rotate(radiansX, radiansY, radiansZ) * scale(sx, sy, sz);
		r.col[3].m128_f32[0] = x;
		r.col[3].m128_f32[1] = y;
		r.col[3].m128_f32[2] = z;
		return r;
	}

	Mat44 Mat44::TRS(Vec translate, Vec radians, Vec scale, ERotateOrder order) { return Mat44::TRS(translate.x, translate.y, translate.z, radians.x, radians.y, radians.z, scale.x, scale.y, scale.z, order); }

	Mat44& Mat44::operator*=(const Mat44& b) {
		// TODO: document whether 'b' is parent or child
		__m128 x = _mm_shuffle_ps(col[0], col[0], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 y = _mm_shuffle_ps(col[0], col[0], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 z = _mm_shuffle_ps(col[0], col[0], _MM_SHUFFLE(2, 2, 2, 2));
		__m128 w = _mm_shuffle_ps(col[0], col[0], _MM_SHUFFLE(3, 3, 3, 3));
		col[0] = _mm_add_ps(
			_mm_add_ps(
				_mm_mul_ps(x, b.col[0]),
				_mm_mul_ps(y, b.col[1])
			),
			_mm_add_ps(
				_mm_mul_ps(z, b.col[2]),
				_mm_mul_ps(w, b.col[3])
			)
		);

		x = _mm_shuffle_ps(col[1], col[1], _MM_SHUFFLE(0, 0, 0, 0));
		y = _mm_shuffle_ps(col[1], col[1], _MM_SHUFFLE(1, 1, 1, 1));
		z = _mm_shuffle_ps(col[1], col[1], _MM_SHUFFLE(2, 2, 2, 2));
		w = _mm_shuffle_ps(col[1], col[1], _MM_SHUFFLE(3, 3, 3, 3));
		col[1] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, b.col[0]),
			_mm_mul_ps(y, b.col[1])),
			_mm_add_ps(_mm_mul_ps(z, b.col[2]),
				_mm_mul_ps(w, b.col[3])));

		x = _mm_shuffle_ps(col[2], col[2], _MM_SHUFFLE(0, 0, 0, 0));
		y = _mm_shuffle_ps(col[2], col[2], _MM_SHUFFLE(1, 1, 1, 1));
		z = _mm_shuffle_ps(col[2], col[2], _MM_SHUFFLE(2, 2, 2, 2));
		w = _mm_shuffle_ps(col[2], col[2], _MM_SHUFFLE(3, 3, 3, 3));
		col[2] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, b.col[0]),
			_mm_mul_ps(y, b.col[1])),
			_mm_add_ps(_mm_mul_ps(z, b.col[2]),
				_mm_mul_ps(w, b.col[3])));

		x = _mm_shuffle_ps(col[3], col[3], _MM_SHUFFLE(0, 0, 0, 0));
		y = _mm_shuffle_ps(col[3], col[3], _MM_SHUFFLE(1, 1, 1, 1));
		z = _mm_shuffle_ps(col[3], col[3], _MM_SHUFFLE(2, 2, 2, 2));
		w = _mm_shuffle_ps(col[3], col[3], _MM_SHUFFLE(3, 3, 3, 3));
		col[3] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, b.col[0]),
			_mm_mul_ps(y, b.col[1])),
			_mm_add_ps(_mm_mul_ps(z, b.col[2]),
				_mm_mul_ps(w, b.col[3])));

		return *this;
	}

	Mat44 Mat44::operator*(const Mat44& b) const {
		Mat44 c = *this;
		c *= b;
		return c;
	}

	Vec4 Mat44::operator*(const Vec4& b) const {
		Vec4 r;
		__m128 x = _mm_shuffle_ps(b.m, b.m, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 y = _mm_shuffle_ps(b.m, b.m, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 z = _mm_shuffle_ps(b.m, b.m, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 w = _mm_shuffle_ps(b.m, b.m, _MM_SHUFFLE(3, 3, 3, 3));
		r.m = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, col[0]), _mm_mul_ps(y, col[1])), _mm_add_ps(_mm_mul_ps(z, col[2]), _mm_mul_ps(w, col[3])));
		return r;
	}

	void Mat44::inverse() {
		__m128 minor0, minor1, minor2, minor3;
		__m128 row0, row1, row2, row3;
		__m128 det, tmp1;
		tmp1 = _mm_shuffle_ps(col[0], col[1], _MM_SHUFFLE(1, 0, 1, 0));
		//tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(m)), (__m64*)(m + 4));
		row1 = _mm_shuffle_ps(col[2], col[3], _MM_SHUFFLE(1, 0, 1, 0));
		//row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(m + 8)), (__m64*)(m + 12));
		row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
		row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
		tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(m + 2)), (__m64*)(m + 6));
		row3 = _mm_shuffle_ps(col[2], col[3], _MM_SHUFFLE(3, 2, 3, 2));
		//row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(m + 10)), (__m64*)(m + 14));
		row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
		row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
		// -----------------------------------------------
		tmp1 = _mm_mul_ps(row2, row3);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor0 = _mm_mul_ps(row1, tmp1);
		minor1 = _mm_mul_ps(row0, tmp1);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
		minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
		minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
		// -----------------------------------------------
		tmp1 = _mm_mul_ps(row1, row2);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
		minor3 = _mm_mul_ps(row0, tmp1);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
		minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
		minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
		// -----------------------------------------------
		tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		row2 = _mm_shuffle_ps(row2, row2, 0x4E);
		minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
		minor2 = _mm_mul_ps(row0, tmp1);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
		minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
		minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
		// -----------------------------------------------
		tmp1 = _mm_mul_ps(row0, row1);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
		minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
		minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
		// -----------------------------------------------
		tmp1 = _mm_mul_ps(row0, row3);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
		minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
		minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
		// -----------------------------------------------
		tmp1 = _mm_mul_ps(row0, row2);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
		minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
		minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
		// -----------------------------------------------
		det = _mm_mul_ps(row0, minor0);
		det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
		det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
		tmp1 = _mm_rcp_ss(det);
		det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
		det = _mm_shuffle_ps(det, det, 0x00);
		minor0 = _mm_mul_ps(det, minor0);
		_mm_storel_pi((__m64*)(m), minor0);
		_mm_storeh_pi((__m64*)(m + 2), minor0);
		minor1 = _mm_mul_ps(det, minor1);
		_mm_storel_pi((__m64*)(m + 4), minor1);
		_mm_storeh_pi((__m64*)(m + 6), minor1);
		minor2 = _mm_mul_ps(det, minor2);
		_mm_storel_pi((__m64*)(m + 8), minor2);
		_mm_storeh_pi((__m64*)(m + 10), minor2);
		minor3 = _mm_mul_ps(det, minor3);
		_mm_storel_pi((__m64*)(m + 12), minor3);
		_mm_storeh_pi((__m64*)(m + 14), minor3);
	}

	void Mat44::transpose() {
		_MM_TRANSPOSE4_PS(col[0], col[1], col[2], col[3]);
	}

	Mat44 Mat44::inversed() const {
		Mat44 a = *this;
		a.inverse();
		return a;
	}

	Mat44 Mat44::transposed() const {
		Mat44 a = *this;
		a.transpose();
		return a;
	}

	bool areMatricesAlmostEqual(float* a, float* b) {
		float delta = 0.0f;
		for (int i = 0; i < 16; ++i) {
			delta += fabsf(a[i] - b[i]);
		}
		return delta < 1e-5f;
	}

	Vec::Vec() : m(_mm_setzero_ps()) {}
	Vec::Vec(float s) : m(_mm_set_ps1(s)) {}
	Vec::Vec(float x, float y, float z, float w) : m(_mm_set_ps(w, z, y, x)) {}
	Vec::Vec(__m128 m) : m(m) {}

	Vec::operator __m128() const {
		return m;
	}

	Vec Vec::dot(__m128 other) const {
		__m128 factors = _mm_mul_ps(m, other);
		__m128 sumA = _mm_add_ps(factors, _mm_shuffle_ps(factors, factors, _MM_SHUFFLE(2, 3, 0, 1)));
		__m128 sumB = _mm_add_ps(sumA, _mm_shuffle_ps(sumA, sumA, _MM_SHUFFLE(0, 0, 2, 2)));
		return sumB;
	}

	Vec Vec::sqrLen() const {
		return dot(*this);
	}

	Vec Vec::len() const {
		return _mm_sqrt_ps(sqrLen());
	}

	Vec Vec::normalized() const {
		return _mm_mul_ps(m, _mm_rsqrt_ps(sqrLen()));
	}

	Vec Vec::operator+() const { return m; }
	Vec Vec::operator-() const { return _mm_sub_ps(_mm_setzero_ps(), m); }
	Vec Vec::operator*(float other) const { return _mm_mul_ps(m, _mm_set_ps1(other)); }
	Vec Vec::operator/(float other) const { return _mm_div_ps(m, _mm_set_ps1(other)); }
	Vec Vec::operator+(float other) const { return _mm_add_ps(m, _mm_set_ps1(other)); }
	Vec Vec::operator-(float other) const { return _mm_sub_ps(m, _mm_set_ps1(other)); }
	Vec Vec::operator%(float other) const { __m128 tmp = _mm_set_ps1(other); return _mm_sub_ps(m, _mm_mul_ps(tmp, _mm_floor_ps(_mm_div_ps(m, tmp)))); }
	Vec Vec::operator*(__m128 other) const { return _mm_mul_ps(m, other); }
	Vec Vec::operator/(__m128 other) const { return _mm_div_ps(m, other); }
	Vec Vec::operator+(__m128 other) const { return _mm_add_ps(m, other); }
	Vec Vec::operator-(__m128 other) const { return _mm_sub_ps(m, other); }
	Vec Vec::operator%(__m128 other) const { return _mm_sub_ps(m, _mm_mul_ps(other, _mm_floor_ps(_mm_div_ps(m, other)))); }

	Vec Vec::operator<(__m128 other) const { return _mm_cmpnge_ps(m, other); }
	Vec Vec::operator>(__m128 other) const { return _mm_cmpnle_ps(m, other); }
	Vec Vec::operator<=(__m128 other) const { return _mm_cmple_ps(m, other); }
	Vec Vec::operator>=(__m128 other) const { return _mm_cmpge_ps(m, other); }
	Vec Vec::operator==(__m128 other) const { return _mm_cmpeq_ps(m, other); }
	Vec Vec::operator!=(__m128 other) const { return _mm_cmpneq_ps(m, other); }

	Vec& Vec::normalize() { m = normalized(); return *this; }
	Vec& Vec::operator*=(float other) { m = *this * other; return *this; }
	Vec& Vec::operator/=(float other) { m = *this / other; return *this; }
	Vec& Vec::operator+=(float other) { m = *this + other; return *this; }
	Vec& Vec::operator-=(float other) { m = *this - other; return *this; }
	Vec& Vec::operator%=(float other) { m = *this % other; return *this; }
	Vec& Vec::operator*=(__m128 other) { m = *this * other; return *this; }
	Vec& Vec::operator/=(__m128 other) { m = *this / other; return *this; }
	Vec& Vec::operator+=(__m128 other) { m = *this + other; return *this; }
	Vec& Vec::operator-=(__m128 other) { m = *this - other; return *this; }
	Vec& Vec::operator%=(__m128 other) { m = *this % other; return *this; }
	Vec Vec::swizzle(unsigned char a, unsigned char b, unsigned char c, unsigned char d) const { 
		return _mm_set_ps(m.m128_f32[d], m.m128_f32[c], m.m128_f32[b], m.m128_f32[a]);
	}

	const float PI = 3.141592653589f;
	const float TAU = PI + PI;
	const float DEG2RAD = PI / 180.0f;
	const double PId = 3.141592653589;
	const double TAUd = PId + PId;
	const double DEG2RADd = PId / 180.0;
	const Mat44 MAT44_IDENTITY = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
}

#if 0
struct TestMatricesScope {
	Mat44 m;

	TestMatricesScope() {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	~TestMatricesScope() {
		float buf[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, buf);
		if (!areMatricesAlmostEqual(m.m, buf))
			__debugbreak();
	}
};

void testMatrices() {
	Window window;
	window.CreateGLContext();
	loadGLFunctions();

	{
		TestMatricesScope t;
		t.m = Mat44::rotateX(45.0f / 180.0f * 3.141592653589f);
		glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	}

	{
		TestMatricesScope t;
		t.m = Mat44::rotateY(45.0f / 180.0f * 3.141592653589f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
	}

	{
		TestMatricesScope t;
		t.m = Mat44::rotateZ(45.0f / 180.0f * 3.141592653589f);
		glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
	}

	{
		TestMatricesScope t;

		glTranslatef(0.0f, 0.0f, -10.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(45.0f, 1.0f, 0.0f, 0.0f);

		Mat44 a = Mat44::rotateX(45.0f / 180.0f * 3.141592653589f);
		Mat44 b = Mat44::rotateY(45.0f / 180.0f * 3.141592653589f);
		Mat44 c = Mat44::translate(0.0f, 0.0f, -10.0f);
		t.m = c * b * a;
	}

	{
		TestMatricesScope t;
		Transform x;
		x.radians.z = 45.0f * DEG2RAD;
		t.m = x.worldMatrix();
		glLoadMatrixf(t.m.m);
	}
}
#endif
