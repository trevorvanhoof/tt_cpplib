#pragma once

#include "tt_globjects.h"

namespace TT {
	struct DebugDraw2D {
		Program program;
		Material mtl;
		Mesh quad;
		Mesh line;
		unsigned int polygonBuffer;
		Image font;

		int width = 2;
		int height = 2;

		DebugDraw2D();
		void updateScreenSize(int w, int h);
		void drawRect(int x, int y, int w, int h, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawPolygon(const std::vector<float>& points, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawLine(int x0, int y0, int x1, int y1, int r = 255, int g = 255, int b = 255, int a = 255);
		void drawText(int x, int y, const char* text, int r = 255, int g = 255, int b = 255, int a = 255);

		double left = -1.0;
		double bottom = -1.0;
		double right = 1.0;
		double top = 1.0;

		void setViewport(double left, double bottom, double right, double top);
		void mapPoint(double x, double y, int& px, int& py);
	};
}
