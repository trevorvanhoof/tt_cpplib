#include "tt_components.h"

namespace TT {
	Component* Entity::registerComponent(Component* component) {
		auto& list = components[component->typeId()];
		list.push_back(component);
		component->entity = this;
		return component;
	}

	Component* Entity::getComponent(Component::TypeId typeId) const {
		const auto& it = components.find(typeId);
		if (it == components.end() || it->second.size() == 0) return nullptr;
		return it->second.front();
	}

	Mat44 Transform::localMatrix() const {
		Mat44 t = Mat44::translate(translate.x, translate.y, translate.z);
		Mat44 rx = Mat44::rotateX(radians.x);
		Mat44 ry = Mat44::rotateY(radians.y);
		Mat44 rz = Mat44::rotateZ(radians.z);
		Mat44 s = Mat44::scale(scale.x, scale.y, scale.z);
		Mat44 rs = ry * rx * rz * s;
		return t * rs;
	}

	Mat44 Transform::worldMatrix() const {
		if (parent == nullptr)
			return localMatrix();
		return parent->worldMatrix() * localMatrix();
	}

	Mat44 Transform::parentMatrix() const {
		if (parent == nullptr)
			return IDENTITY;
		return parent->worldMatrix();
	}

	Mat44 Camera::projectionMatrix(float aspectRatio) const {
		return Mat44::perspectiveY(verticalFieldOfViewRadians, aspectRatio, near, far);
	}

	Mat44 Camera::viewMatrix() const {
		if (!entity || !entity->getComponent<Transform>()) return {};
		return entity->getComponent<Transform>()->worldMatrix().inversed();
	}

	Vec2 Camera::frustumExtents(float aspectRatio) const {
		float ey = std::tanf(verticalFieldOfViewRadians * 0.5f);
		float ex = ey * aspectRatio;
		return { ex, ey };
	}
}
