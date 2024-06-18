#pragma once

#include "tt_window.h"
#include "tt_signals.h"
#include "tt_cgmath.h"

namespace TT {
	struct OrbitCameraControl {
		bool drag = false;
		float yaw = 45.0f * DEG2RAD;
		float pitch = -45.0f * DEG2RAD;
		float distance = 10.0f;

		int beginDragX;
		int beginDragY;
		float beginDrawYaw;
		float beginDrawPitch;

        Mat44 localMatrix() const;
		void onMouseEvent(const TT::MouseEvent& event);
		void onWheelEvent(const TT::WheelEvent& event);

		Signal<> changed;
	};
}
