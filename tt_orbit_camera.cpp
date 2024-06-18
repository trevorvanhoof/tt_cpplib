#include "tt_orbit_camera.h"

namespace {
	float clampf(float v, float n, float x) {
		if (v < n)return n;
		if (v > x) return x;
		return v;
	}
}

namespace TT {
    Mat44 OrbitCameraControl::localMatrix() const {
        Mat44 rx = Mat44::rotateX(pitch);
        Mat44 ry = Mat44::rotateY(yaw);
        Mat44 rs = ry * rx;
        Vec3 translate = _mm_mul_ps(rs.col[2], _mm_set_ps1(distance));
        Mat44 t = Mat44::translate(translate.x, translate.y, translate.z);
        return t * rs;
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

		changed.emit();
	}

	void OrbitCameraControl::onWheelEvent(const TT::WheelEvent& event) {
		for (int _ = 0; _ < event.delta; ++_) {
			distance /= 1.001f;
		}

		for (int _ = 0; _ < -event.delta; ++_) {
			distance *= 1.001f;
		}

        changed.emit();
	}
}
