#include "tt_camera.h"
#include "tt_gl_cpp.h"

namespace TT {
	CameraMatrix OrbitCameraControl::ViewMatrix() {
		float cx = std::cosf(-pitchRadians);
		float cy = std::cosf(-yawRadians);
		float sx = std::sinf(-pitchRadians);
		float sy = std::sinf(-yawRadians);
		CameraMatrix pitch = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cx, sx, 0.0f,
			0.0f, -sx, cx, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		CameraMatrix yaw = {
			cy, 0.0f, -sy, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			sy, 0.0f, cy, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		CameraMatrix matrix = yaw * pitch;
		matrix.cols[3] = _mm_sub_ps(_mm_setzero_ps(), _mm_add_ps(_mm_mul_ps(matrix.cols[0], _mm_set_ps1(pivot.m128_f32[0])),
			_mm_add_ps(_mm_mul_ps(matrix.cols[1], _mm_set_ps1(pivot.m128_f32[1])),
				_mm_mul_ps(matrix.cols[2], _mm_set_ps1(pivot.m128_f32[2]))))) ;
		matrix.cols[3].m128_f32[2] -= viewDistance;
		return matrix;
	}

	CameraMatrix OrbitCameraControl::RotationMatrix() {
		float cx = std::cosf(pitchRadians);
		float cy = std::cosf(yawRadians);
		float sx = std::sinf(pitchRadians);
		float sy = std::sinf(yawRadians);
		CameraMatrix pitch = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cx, sx, 0.0f,
			0.0f, -sx, cx, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		CameraMatrix yaw = {
			cy, 0.0f, -sy, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			sy, 0.0f, cy, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		return pitch * yaw;
	}

	CameraMatrix OrbitCameraControl::ProjectionMatrix(float aspectRatio) {
		float halfWidth = tanf(0.5f * horizontalFoVRadians) * near;
		float halfHeight = halfWidth / aspectRatio;
		float dz = near - far;
		return { near / halfWidth, 0.0f, 0.0f, 0.0f,
			0.0f, near / halfHeight, 0.0f, 0.0f,
			0.0f, 0.0f, (far + near) / dz,  -1.0f,
			0.0f, 0.0f, (2.0f * far * near) / dz, 0.0f };
	}

	void OrbitCameraControl::OnMouseEvent(Window& window, MouseEvent event) {
		if (event.type == Event::EType::MouseDown) {
			prevMouseX = event.x;
			prevMouseY = event.y;
			drag = true;
			mouseButton = event.button;
		}

		if (drag && event.type == Event::EType::MouseMove) {
			if (mouseButton == 0) {
				pitchRadians = std::max(-3.1415926f / 2.0f, std::min(3.1415926f / 2.0f, pitchRadians - (prevMouseY - (int)event.y) * pitchScale));
				yawRadians += (prevMouseX - (int)event.x) * yawScale;
				window.Repaint();
			}
			if (mouseButton == 1) {
				CameraMatrix C = RotationMatrix();
				pivot = _mm_add_ps(pivot, _mm_mul_ps(C.cols[0], _mm_set_ps1((prevMouseX - (int)event.x) * panScale)));
				pivot = _mm_add_ps(pivot, _mm_mul_ps(C.cols[1], _mm_set_ps1((prevMouseY - (int)event.y) * panScale)));
				window.Repaint();
			}
			prevMouseX = event.x;
			prevMouseY = event.y;
		}

		if (event.type == TT::Event::EType::MouseUp) {
			drag = false;
		}
	}

	void OrbitCameraControl::OnWheelEvent(Window& window, TT::WheelEvent event) {
		if (event.delta < 0)
			viewDistance *= wheelScale;
		else
			viewDistance /= wheelScale;
		window.Repaint();
	}

	OrbitCameraWindowBase::OrbitCameraWindowBase() {
		dc = CreateGLContext();
		// TODO: Make this optional
		debugDraw = new TT::DebugDraw3D;
	}

	void OrbitCameraWindowBase::OnResizeEvent(TT::ResizeEvent event) {
		int width = Width();
		int height = Height();
		glViewport(0, 0, width, height);
		debugDraw->updateScreenSize(width, height);
	}

	void OrbitCameraWindowBase::OnPaintEvent(TT::PaintEvent event) {
		// This just does some setup, call before the implementation.
		// Don't forget to call SwapBuffers(dc); at the end.
		float aspectRatio = Width() / (float)Height();
		debugDraw->setCamera(camera3D.ViewMatrix(), camera3D.ProjectionMatrix(aspectRatio));
	}

	void OrbitCameraWindowBase::OnMouseEvent(TT::MouseEvent event) { camera3D.OnMouseEvent(*this, event); }
	void OrbitCameraWindowBase::OnWheelEvent(TT::WheelEvent event) { camera3D.OnWheelEvent(*this, event); }
}
