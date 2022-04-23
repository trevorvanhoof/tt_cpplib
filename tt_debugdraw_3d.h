#pragma once

#include "MMath/Mat44.h"
#include "MMath/Vector.h"
#include "tt_globjects.h"
#include "tt_debugdraw_2d.h"

namespace TT {
	struct DebugDraw3D : public DebugDraw2D {
		Mat44 VP;
		void transformPoint(float x, float y, float z, int& xPx, int& yPx);

		void setCamera(Mat44 view, Mat44 proj);
		void drawPoint3D(float x, float y, float z, int sizePx, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawPolygon3D(const std::vector<float>& points, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawLine3D(float x0, float y0, float z0, float x1, float y1, float z1, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawText3D(float x, float y, float z, const char* text, int r = 255, int g = 255, int b = 255, int a = 255);
	};
}

