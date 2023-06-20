#pragma once

#include "tt_signals.h"
#include "tt_window.h"
#include "tt_math.h"
#include "fontstash/fontstash.h"
#include "gl46corefontstash.h"

struct Size {
	int x, y;
};

struct Rect {
	int x, y, w, h;
	bool contains(int px, int py) {
		if (px < x || py < y || px > x + w || py > y + h) return false;
		return true;
	}
};

struct UIContext {
	void init() {
		// Create GL stash for 512x512 texture, our coordinate system has zero at top-left.
		fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);

		// Add font to stash.
		defaultFont = fonsAddFont(fs, "sans", "DroidSerif-Regular.ttf");

		// Get additional render resources
		float vertices[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
		};
		unsigned char indices[] = { 0,1,2, 2,1,3 };
		quad.attribs.push_back({ TT::MeshAttrib::Semantic::POSITION, TT::MeshAttrib::Dimensions::D2, TT::MeshAttrib::ElementType::F32 });
		quad.alloc(sizeof(float) * 4 * 2, vertices, 6, TT::Mesh::IndexType::U8, indices);

		mtl.program = TT::ProgramManager::fetchProgram({ "ui.vert.glsl", "ui.frag.glsl" });

		// Set a default font
		unsigned int white = glfonsRGBA(255, 255, 255, 255);
		fonsSetSize(fs, 16.0f);
		fonsSetFont(fs, defaultFont);
		fonsSetColor(fs, white);
		fonsSetAlign(fs, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
	}

	void cleanup() {
		glfonsDelete(fs);
	}

	void resize(int width, int height) {
		glFonsSetResolution(fs, width, height);
		mtl.set("uResolution", { (float)width, (float)height });
		viewportHeight = height;
	}

	FONScontext* fs = nullptr;
	int defaultFont = FONS_INVALID;
	TT::Mesh quad;
	TT::Material mtl;
	int viewportHeight;
};

void drawRect(UIContext& painter, Rect q, float r, float g, float b, float a) {
	painter.mtl.use();
	painter.mtl.set("uColor", { r,g,b,a });
	painter.mtl.set("uGeometry", { (float)q.x , (float)q.y, (float)q.w, (float)q.h });
	painter.quad.drawIndexed(GL_TRIANGLES);
}

void layoutText(UIContext& painter, std::vector<FONSquad>& layout, const std::string& text, int x, int y, bool noClear = false) {
	if (noClear) {
		layout.reserve(layout.size() + text.size());
	}
	else {
		layout.clear();
		layout.reserve(text.size());
	}
	FONStextIter iter;
	fonsTextIterInit(painter.fs, &iter, (float)x, (float)y, text.data(), text.data() + text.size());
	FONSquad q;
	while (fonsTextIterNext(painter.fs, &iter, &q))
		layout.push_back(q);
}

void drawText(UIContext& painter, const std::vector<FONSquad>& layout) {
	for (const FONSquad& q : layout)
		fonsEmitQuad(painter.fs, q);
	fonsFlush(painter.fs);
}

namespace {
	template<typename T>
	T sum(const std::vector<T>& a) {
		T r = 0;
		for (const T& v : a)
			r += v;
		return r;
	}

	template<typename T>
	bool remove(std::vector<T>& a, const T& k) {
		auto it = std::find(a.begin(), a.end(), k);
		if (it == a.end()) return false;
		a.erase(it);
		return true;
	}

	template<typename T>
	T get(const std::vector<T>& a, size_t i, const T& f) {
		if (i < 0 || i >= a.size()) return f;
		return a[i];
	}
}

struct Object {
protected:
	// Children of an object are owned by the object
	std::vector<Object*> _children = {};
	Object* _parent = nullptr;
	Rect _geometry = {};
	bool _mouseDrag = false;
	Size _preferredSize = {};

	bool catchMouseEvent(const TT::MouseEvent& event) {
		switch (event.type) {
		case TT::Event::EType::MouseDoubleClick:
			if (!_geometry.contains(event.x, event.y))
				return false;
			break;
		case TT::Event::EType::MouseDown:
			if (!_geometry.contains(event.x, event.y))
				return false;
			_mouseDrag = true;
			break;
		case TT::Event::EType::MouseMove:
			if (!_mouseDrag && !_geometry.contains(event.x, event.y))
				return false;
			break;
		case TT::Event::EType::MouseUp:
			if (!_mouseDrag)
				return false;
			_mouseDrag = false;
			break;
		}
		return true;
	}

public:
	Object* parent() const { return _parent; }
	void setParent(Object* parent) { 
		if (parent == _parent) 
			return;
		if (_parent)
			remove(_parent->_children, this);
		_parent = parent;
		if(_parent)
			_parent->_children.push_back(this);

	}
	const std::vector<Object*>& children() const { return _children; }
	const void addChild(Object* child) { child->setParent(this); }

	virtual ~Object() {
		for (auto child : _children)
			delete child;
	}

	Rect geometry() { return _geometry; }
	virtual void setGeometry(Rect geometry) { _geometry = geometry; }

	virtual void draw(UIContext& painter) {
		for (Object* ch : _children)
			ch->draw(painter);
	}

	virtual bool onMouseEvent(const TT::MouseEvent& event) {
		if (!catchMouseEvent(event)) 
			return false;
		for (Object* ch : _children)
			if(ch->onMouseEvent(event))
				return true;
		return false;
	}

	Size preferredSize() { return _preferredSize; }
	void setPreferredSize(const Size& preferredSize) { _preferredSize = preferredSize; }
};

struct Layout : public Object {
	int spacing = 0;
	int margins[4] = {};
	virtual void layout() = 0;
	void setGeometry(Rect geometry) override { _geometry = geometry; layout(); }
};

struct HBoxLayout : public Layout {
	std::vector<int> stretch;
	std::vector<int> minimumSizes;
	void layout() override {
		int portions = sum(stretch);

		int remainder = _geometry.w - margins[0] - margins[2] - spacing * ((int)_children.size() - 1);
		for (size_t i = 0; i < _children.size(); ++i)
			remainder -= get(minimumSizes, i, _children[i]->preferredSize().x);

		int x = _geometry.x + margins[0];
		for (size_t i = 0; i < _children.size(); ++i) {
			int w = get(minimumSizes, i, _children[i]->preferredSize().x);
			if (portions == 0) {
				w += remainder / (int)_children.size();
			} else {
				w += (remainder * get(stretch, i, 0)) / portions;
			}
			_children[i]->setGeometry({ x, _geometry.y + margins[1], w, _geometry.h - margins[1] - margins[3] });
			x += w + spacing;
		}
	}
};

struct VBoxLayout : public Layout {
	std::vector<int> stretch;
	std::vector<int> minimumSizes;
	void layout() override {
		int portions = sum(stretch);

		int remainder = _geometry.h - margins[1] - margins[3] - spacing * ((int)_children.size() - 1);
		for (size_t i = 0; i < _children.size(); ++i)
			remainder -= get(minimumSizes, i, _children[i]->preferredSize().y);

		int y = _geometry.y + margins[1];
		for (size_t i = 0; i < _children.size(); ++i) {
			int h = get(minimumSizes, i, _children[i]->preferredSize().y);
			if (portions == 0) {
				h += remainder / (int)_children.size();
			} else {
				h += (remainder * get(stretch, i, 0)) / portions;
			}
			_children[i]->setGeometry({ _geometry.x + margins[0], y, _geometry.w - margins[0] - margins[2], h });
			y += h + spacing;
		}
	}
};

struct Widget : public Object {
protected:
	void drawInitScissor(UIContext& painter) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(_geometry.x, painter.viewportHeight - _geometry.y - _geometry.h, _geometry.w, _geometry.h);
	}

	void drawClearScissor() {
		glDisable(GL_SCISSOR_TEST);
	}
};

struct Label : public Widget {
protected:
	std::string _text;

public:
	std::string text() { return _text; }
	void setText(const std::string& text) { _text = text; }

	Label(const std::string& text = "") : _text(text), Widget() {}

	virtual void draw(UIContext& painter) override {
		// TODO: Only recalc this on font changes
		float bounds[4];
		fonsTextBounds(painter.fs, 0.0f, 0.0f, "...", nullptr, bounds);
		float elideSpace = bounds[2];

		drawInitScissor(painter);
		fonsDrawText(painter.fs, (float)_geometry.x, (float)_geometry.y, _text.data(), _text.data() + _text.size());
		drawClearScissor();

		Widget::draw(painter);
	}
};

struct SelectableLabel : public Label {
protected:
	int start = -1;
	int end = -1;
	std::vector<FONSquad> layout;

	// Assumes layout, start and end are set up already
	void drawHighlight(UIContext& painter) {
		if (start != -1) {
			if (end != -1) {
				int a = TT::min(start, end);
				int left;
				if (a >= layout.size())
					left = (int)layout.back().x1;
				else
					left = (int)layout[a].x0;

				int b = TT::max(start, end);
				int right;
				if (b >= layout.size())
					right = (int)layout.back().x1;
				else
					right = (int)layout[b].x0;
				drawRect(painter, { left, _geometry.y, right - left, _geometry.h }, 0.0f, 0.7f, 1.0f, 1.0f);
			}
			else {
				int left;
				if (start >= layout.size())
					left = (int)layout.back().x1;
				else
					left = (int)layout[start].x0;
				drawRect(painter, { left - 1, _geometry.y, 2, _geometry.h }, 1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}

public:
	using Label::Label;

	bool showHighlight = true;

	virtual bool onMouseEvent(const TT::MouseEvent& event) {
		if (!catchMouseEvent(event)) return false;
		if (event.type == TT::Event::EType::MouseDoubleClick) {
			start = 0;
			end = (int)_text.size();
			return true;
		}
		if (!_mouseDrag) return false;
		
		int i = 0;
		for (const auto& q : layout) {
			if (event.x < q.x1) {
				if (event.type == TT::Event::EType::MouseDown) {
					if (event.x < (q.x0 + q.x1) * 0.5f) {
						start = i;
					} else {
						start = i + 1;
					}
					end = -1;
				} else {
					if (event.x < (q.x0 + q.x1) * 0.5f) {
						end = i;
					} else {
						end = i + 1;
					}
				}
				break;
			}
			++i;
		}
		return true;
	}

	virtual void draw(UIContext& painter) override {
		drawInitScissor(painter);
		layoutText(painter, layout, _text, _geometry.x + 4, _geometry.y);
		if(showHighlight)
			drawHighlight(painter);
		drawText(painter, layout);
		drawClearScissor();
		Widget::draw(painter);
	}
};

struct ElidedLabel : public SelectableLabel {
	using SelectableLabel::SelectableLabel;

	void draw(UIContext& painter) override {
		float bounds[4];
		fonsTextBounds(painter.fs, 0.0f, 0.0f, "...", nullptr, bounds);
		float elideSpace = bounds[2];

		drawInitScissor(painter);
		// layoutText with elide
		layout.clear();
		layout.reserve(_text.size());
		FONStextIter iter;
		fonsTextIterInit(painter.fs, &iter, (float)_geometry.x, (float)_geometry.y, _text.data(), _text.data() + _text.size());
		FONSquad q;
		float elideLimit = _geometry.x + _geometry.w - elideSpace;
		while (fonsTextIterNext(painter.fs, &iter, &q)) {
			if (q.x1 > elideLimit && iter.next != iter.end) {
				layoutText(painter, layout, "...", (int)iter.x, _geometry.y, true);
				break;
			}
			layout.push_back(q);
		}
		drawHighlight(painter);
		drawText(painter, layout);
		drawClearScissor();

		Widget::draw(painter);
	}
};

struct Slider : public Widget {
	float min = 0.0f;
	float max = 1.0f;
	float value = 0.5f;

	TT::Signal<float> valueChanged;

	virtual bool onMouseEvent(const TT::MouseEvent& event) {
		if (!catchMouseEvent(event)) return false;
		if (!_mouseDrag) return false;

		float t = (event.x - _geometry.x) / (float)_geometry.w;
		value = TT::clamp(t, 0.0f, 1.0f) * (max - min) + min;
		valueChanged.emit(value);
		return true;
	}

	void draw(UIContext& painter) override {
		drawRect(painter, _geometry, 0.0f, 0.0f, 0.0f, 0.5f);
		float t = (value - min) / (max - min);
		drawRect(painter, { _geometry.x , _geometry.y, (int)((float)_geometry.w * t), _geometry.h}, 1.0f, 1.0f, 1.0f, 1.0f);
	}
};

struct LabelSlider : public HBoxLayout {
protected:
	ElidedLabel* label;
	Slider* slider;

public:
	TT::Signal<float>& valueChanged() { return slider->valueChanged; }

	LabelSlider(UIContext& painter, const std::string& caption, int labelWidth = 0) {
		margins[1] = 4;
		margins[3] = 4;
		spacing = 2;
		_preferredSize.y = 24;

		label = new ElidedLabel(caption);
		addChild(label);
		stretch.push_back(0);
		if (labelWidth == 0) {
			float bounds[4];
			fonsTextBounds(painter.fs, 0.0f, 0.0f, caption.data(), nullptr, bounds);
			minimumSizes.push_back((int)bounds[2] + 16);
		} else {
			minimumSizes.push_back(labelWidth);
		}

		slider = new Slider();
		addChild(slider);
		stretch.push_back(1);
	}
};
