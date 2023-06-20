#include "tt_orbit_camera.h"

namespace {
	float clampf(float v, float n, float x) {
		if (v < n)return n;
		if (v > x) return x;
		return v;
	}
}

namespace TT {
	OrbitCameraControl::OrbitCameraControl(Transform& cameraTransform) : cameraTransform(cameraTransform) {
		sync();
	}

	void OrbitCameraControl::sync() {
		cameraTransform.radians.x = pitch;
		cameraTransform.radians.y = yaw;
		cameraTransform.translate.m = _mm_mul_ps(cameraTransform.localMatrix().col[2], _mm_set_ps1(distance));
		changed.emit();
	}

	void OrbitCameraControl::onMouseEvent(const TT::MouseEvent& event) {
		if (event.type == TT::Event::EType::MouseDown) {
			beginDragX = event.x;
			beginDragY = event.y;
			beginDrawYaw = yaw;
			beginDrawPitch = pitch;
			drag = true;
		}

		if (event.type == TT::Event::EType::MouseUp) {
			drag = false;
		}

		if (event.type == TT::Event::EType::MouseMove && drag) {
			int dx = event.x - beginDragX;
			int dy = event.y - beginDragY;
			yaw = fmodf(beginDrawYaw - dx * DEG2RAD * 0.5f, 360.0f * DEG2RAD);
			pitch = clampf(beginDrawPitch - dy * DEG2RAD * 0.5f, -90.0f * DEG2RAD, 0.0f * DEG2RAD);
		}

		sync();
	}

	void OrbitCameraControl::onWheelEvent(const TT::WheelEvent& event) {
		for (int _ = 0; _ < event.delta; ++_) {
			distance /= 1.001f;
		}

		for (int _ = 0; _ < -event.delta; ++_) {
			distance *= 1.001f;
		}

		sync();
	}
}
