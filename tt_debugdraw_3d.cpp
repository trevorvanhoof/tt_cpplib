#include "tt_debugdraw_3d.h"

namespace TT {
	void DebugDraw3D::setCamera(CameraMatrix view, CameraMatrix proj) {
		this->VP = view * proj;
	}

	void DebugDraw3D::transformPoint(float x, float y, float z, int& xPx, int& yPx) {
		__m128 v = { x, y, z, 1.0f };
		v = VP * v;
		xPx = (int)(((v.m128_f32[0] / v.m128_f32[3]) * 0.5f + 0.5f) * width);
		yPx = (int)(((v.m128_f32[1] / v.m128_f32[3]) * 0.5f + 0.5f) * height);
	}

	void DebugDraw3D::drawPoint3D(float x, float y, float z, int sizePx, int r, int g, int b, int a) {
		int xPx, yPx;
		transformPoint(x, y, z, xPx, yPx);
		xPx -= (sizePx >> 1);
		yPx -= (sizePx >> 1);
		drawRect(xPx, yPx, sizePx, sizePx, r, g, b, a);
	}

	void DebugDraw3D::drawPolygon3D(const std::vector<float>& points, int r, int g, int b, int a) {
		std::vector<float> tmp;
		tmp.resize((points.size() / 3) * 2);
		for (int i = 0; i < points.size() / 3; ++i) {
			int xPx, yPx;
			size_t j = (size_t)i * 3;
			transformPoint(points[j], points[j + 1], points[j + 2], xPx, yPx);
			tmp[(size_t)i * 2] = (float)xPx;
			tmp[(size_t)i * 2 + 1] = (float)yPx;
		}
		drawPolygon(tmp, r, g, b, a);
	}

	void DebugDraw3D::drawLine3D(float x0, float y0, float z0, float x1, float y1, float z1, int r, int g, int b, int a) {
		int xPx0, yPx0, xPx1, yPx1;
		transformPoint(x0, y0, z0, xPx0, yPx0);
		transformPoint(x1, y1, z1, xPx1, yPx1);
		drawLine(xPx0, yPx0, xPx1, yPx1, r, g, b, a);
	}

	void DebugDraw3D::drawText3D(float x, float y, float z, const char* text, int r, int g, int b, int a) {
		int xPx, yPx;
		transformPoint(x, y, z, xPx, yPx);
		drawText(xPx, yPx, text, r, g, b, a);
	}
}
