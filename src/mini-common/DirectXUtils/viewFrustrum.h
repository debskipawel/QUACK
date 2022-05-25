#pragma once
#include <DirectXMath.h>
#include <windows.h>
#include "dxstructures.h"

namespace mini
{
	class ViewFrustrum
	{
	public:
		ViewFrustrum(SIZE viewportSize, float fov, float nearPlane, float farPlane);

		SIZE viewportSize() const { return m_viewportSize; }
		void setViewportSize(SIZE viewportSize);
		float fov() const { return m_fov; }
		void setFov(float fov);
		float nearPlane() const { return m_nearPlane; }
		void setNearPlane(float nearPlane);
		float farPlane() const { return m_farPlane; }
		void setFarPlane(float farPlane);

		utils::ViewportDescription getViewportDescription() const;
		DirectX::XMMATRIX getProjectionMatrix() const;

	private:
		SIZE m_viewportSize;
		float m_fov, m_nearPlane, m_farPlane;
	};
}
