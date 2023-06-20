#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include "tt_cgmath.h"
#include "tt_messages.h"
#include "tt_render_concepts.h"

#define TT_BEGIN_SUB_COMPONENT(CLASS, ...) struct CLASS : public __VA_ARGS__ { virtual Component::TypeId typeId() const override { return CLASS::sTypeId(); } static Component::TypeId sTypeId() { return #CLASS; }
#define TT_BEGIN_COMPONENT(CLASS) TT_BEGIN_SUB_COMPONENT(CLASS, Component)
#define TT_END_COMPONENT };

namespace TT {
	struct Component {
		struct Entity* entity = nullptr;
		typedef std::string TypeId;
		virtual TypeId typeId() const = 0;
	};

	struct Entity {
		std::unordered_map<Component::TypeId, std::vector<Component*>> components;

		Component* registerComponent(Component* component);

		template<typename T>
		T* addComponent() { return (T*)registerComponent(new T); }

		Component* getComponent(Component::TypeId typeId) const;

		template<typename T>
		T* getComponent() const { return (T*)getComponent(T::sTypeId()); }
	};

	TT_BEGIN_COMPONENT(Transform);
		Transform* parent = nullptr;
		Vec3 translate = {0,0,0};
		Vec3 radians = {0,0,0};
		Vec3 scale = {1,1,1};
		ERotateOrder rotateOrder = ERotateOrder::YXZ;

		Mat44 localMatrix() const;
		Mat44 worldMatrix() const;
		Mat44 parentMatrix() const;
	TT_END_COMPONENT;

	TT_BEGIN_COMPONENT(Camera);
		float verticalFieldOfViewRadians = 72.0f * DEG2RAD;
		float near = 0.01f;
		float far = 10000.0f;

		Mat44 projectionMatrix(float aspectRatio) const;
		Mat44 viewMatrix() const;
		Vec2 frustumExtents(float aspectRatio) const;
	TT_END_COMPONENT;

	TT_BEGIN_COMPONENT(Drawable);
		virtual void draw(const std::unordered_map<std::string, UniformValue>& uniforms) const = 0;
	TT_END_COMPONENT;
}

// #undef TT_BEGIN_SUB_COMPONENT
// #undef TT_BEGIN_COMPONENT
// #undef TT_END_COMPONENT
