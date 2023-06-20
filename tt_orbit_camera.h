#pragma once

#include "tt_components.h"
#include "tt_window.h"

namespace TT {
	struct OrbitCameraControl {
		bool drag = false;
		float yaw = 45.0f * DEG2RAD;
		float pitch = -45.0f * DEG2RAD;
		float distance = 10.0f;

		Transform& cameraTransform;
		int beginDragX;
		int beginDragY;
		float beginDrawYaw;
		float beginDrawPitch;

		OrbitCameraControl(Transform& cameraTransform);
		void sync();
		void onMouseEvent(const TT::MouseEvent& event);
		void onWheelEvent(const TT::WheelEvent& event);

		Signal<> changed;
	};
}
