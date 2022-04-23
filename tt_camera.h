#pragma once

#include "tt_customwindow.h"
#include "tt_debugdraw_3d.h"

namespace TT {
	struct OrbitCameraControl {
		// Input config
		float wheelScale = 1.05f;
		float yawScale = 0.004f;
		float pitchScale = 0.004f;
		float panScale = 0.002f;

		// Camera settings
		float viewDistance = 10.0f;
		float pitchRadians = 0.0f;
		float yawRadians = 0.0f;
		float horizontalFoVRadians = 1.5f;
		float near = 0.01f;
		float far = 10000.0f;
		__m128 pivot = _mm_setzero_ps();

		CameraMatrix RotationMatrix();
		CameraMatrix ViewMatrix();
		CameraMatrix ProjectionMatrix(float aspectRatio);
		void OnMouseEvent(Window& window, MouseEvent event);
		void OnWheelEvent(Window& window, TT::WheelEvent event);

	private:
		int mouseButton = 0;
		int prevMouseX = 0;
		int prevMouseY = 0;
		bool drag = false;
	};

	struct OrbitCameraWindowBase : public TT::Window {
		OrbitCameraWindowBase();

	protected:
		struct HDC__* dc;
		TT::DebugDraw3D* debugDraw;
		TT::OrbitCameraControl camera3D;

		virtual void OnResizeEvent(TT::ResizeEvent event) override;
		virtual void OnPaintEvent(TT::PaintEvent event) override;
		virtual void OnMouseEvent(TT::MouseEvent event) override;
		virtual void OnWheelEvent(TT::WheelEvent event) override;
	};
}
