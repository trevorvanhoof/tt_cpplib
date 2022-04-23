#include "tt_debugdraw_2d.h"
#include "tt_gl_cpp.h"


namespace TT {
	DebugDraw2D::DebugDraw2D() : program({ Shader("#version 410\n"
	"in vec2 aVertex;"
	"uniform vec4 uRect;"
	"uniform vec2 uResolution;"
	"uniform int uMode;"
	"out vec2 vUv;"
	"void main(){"
	"vUv=aVertex;"
	"if(uMode == 2) { gl_Position = vec4(vec2(uRect[(gl_VertexID % 2) * 2], uRect[(gl_VertexID % 2) * 2 + 1])/(uResolution*.5)*vec2(1,-1)+vec2(-1,1), 0.0, 1.0); return; }"
	"if(uMode == 3) { gl_Position = vec4(aVertex/(uResolution*.5)*vec2(1,-1)+vec2(-1,1), 0.0, 1.0); return; }"
	"gl_Position=vec4((aVertex*uRect.zw+uRect.xy)/(uResolution*.5)*vec2(1,-1)+vec2(-1,1),0,1);"
	"};", GL_VERTEX_SHADER), Shader("#version 410\n"
	"in vec2 vUv;"
	"out vec4 cd;"
	"uniform sampler2D uImage;"
	"uniform vec4 uTint;"
	"uniform int uMode;"
	"uniform int uChr;"
	"void main(){"
	"cd=uTint;if(uMode==1){vec4 T = texture(uImage,(vUv+vec2(uChr,0))/vec2(95,1)); if(T.xyz == vec3(19,19,20)/255) discard; cd*=T;};"
	"};", GL_FRAGMENT_SHADER) }, 2),
		mtl(program),
		quad({ 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }, std::initializer_list<unsigned char>({ 0, 1, 2, 0, 2, 3 }), { {2, GL_FLOAT} }, GL_TRIANGLES),
		line({ 0.0f, 0.0f, 1.0f, 0.0f }, std::initializer_list<unsigned char>({ 0, 1 }), { {2, GL_FLOAT} }, GL_LINES),
		font("fira_code_spritefont.png")
	{
		mtl.set("uImage", font);
		glGenBuffers(1, &polygonBuffer);
	}

	void DebugDraw2D::updateScreenSize(int w, int h) {
		mtl.set("uResolution", (float)w, (float)h);
		width = w;
		height = h;
	}

	void DebugDraw2D::drawRect(int x, int y, int w, int h, int r, int g, int b, int a) {
		if (!a) return;
		if (a != 255) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		mtl.set("uMode", 0);
		mtl.set("uTint", (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
		mtl.set("uRect", (float)x, (float)y, (float)w, (float)h);
		mtl.use();
		quad.draw();

		if (a != 255)
			glDisable(GL_BLEND);
	}

	void DebugDraw2D::drawPolygon(const std::vector<float>& points, int r, int g, int b, int a) {
		if (!points.size()) return;

		if (!a) return;
		if (a != 255) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		mtl.set("uMode", 3);
		mtl.set("uTint", (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
		mtl.use();

		glBindBuffer(GL_ARRAY_BUFFER, polygonBuffer);
		glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), &points[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)points.size() >> 1);

		if (a != 255)
			glDisable(GL_BLEND);
	}

	void DebugDraw2D::drawLine(int x0, int y0, int x1, int y1, int r, int g, int b, int a) {
		if (!a) return;
		if (a != 255) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		mtl.set("uMode", 2);
		mtl.set("uTint", (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
		mtl.set("uRect", (float)x0, (float)y0, (float)x1, (float)y1);
		mtl.use();
		line.draw();

		if (a != 255)
			glDisable(GL_BLEND);
	}

	void DebugDraw2D::drawText(int x, int y, const char* text, int r, int g, int b, int a) {
		if (!a) return;
		if (a != 255) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		mtl.set("uMode", 1);
		mtl.set("uTint", (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
		while (*text != 0) {
			mtl.set("uChr", (int)(*text - 32));
			mtl.set("uRect", (float)x, (float)y, (float)8, (float)16);
			x += 8;
			text += 1;
			mtl.use();
			quad.draw();
		}

		if (a != 255)
			glDisable(GL_BLEND);
	}

	void DebugDraw2D::setViewport(double left, double bottom, double right, double top) {
		this->left = left;
		this->bottom = bottom;
		this->right = right;
		this->top = top;
	}

	void DebugDraw2D::mapPoint(double x, double y, int& px, int& py) {
		x -= left;
		y -= bottom;
		x /= (right - left);
		y /= (top - bottom);
		x *= width;
		y *= height;
		px = (int)x;
		py = (int)y;
	}
}
