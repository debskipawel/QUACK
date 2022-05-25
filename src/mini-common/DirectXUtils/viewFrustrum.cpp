#include "viewFrustrum.h"

using namespace DirectX;
using namespace mini;

ViewFrustrum::ViewFrustrum(SIZE viewportSize, float fov, float nearPlane, float farPlane)
	: m_viewportSize(viewportSize), m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane)
{
}

void ViewFrustrum::setViewportSize(SIZE viewportSize)
{
	m_viewportSize = viewportSize;
}

void ViewFrustrum::setFov(float fov)
{
	m_fov = fov;
}

void ViewFrustrum::setNearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
}

void ViewFrustrum::setFarPlane(float farPlane)
{
	m_farPlane = farPlane;
}

utils::ViewportDescription ViewFrustrum::getViewportDescription() const
{
	return utils::ViewportDescription(m_viewportSize.cx, m_viewportSize.cy);
}

DirectX::XMMATRIX ViewFrustrum::getProjectionMatrix() const
{
	return XMMatrixPerspectiveFovLH(m_fov, m_viewportSize.cx / static_cast<float>(m_viewportSize.cy), m_nearPlane, m_farPlane);
}
