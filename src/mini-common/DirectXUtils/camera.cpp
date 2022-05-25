#include "camera.h"

using namespace mini;
using namespace DirectX;

OrbitCamera::OrbitCamera(XMFLOAT3 target, float minDist, float maxDist, float dist)
	: m_angleX(0), m_angleY(0), m_target(target.x, target.y, target.z, 1.0f), m_distance(dist)
{
	SetDistanceRange(minDist, maxDist);
}

OrbitCamera::OrbitCamera(float minDist, float maxDist, float distance)
	: OrbitCamera(XMFLOAT3{0.0f, 0.0f, 0.0f}, minDist, maxDist, distance)
{ }

void OrbitCamera::SetDistanceRange(float minDistance, float maxDistance)
{
	if (maxDistance < minDistance)
		maxDistance = minDistance;
	m_minDistance = minDistance;
	m_maxDistance = maxDistance;
	ClampDistance();
}

void OrbitCamera::ClampDistance()
{
	if (m_distance < m_minDistance)
		m_distance = m_minDistance;
	else if (m_distance > m_maxDistance)
		m_distance = m_maxDistance;
}

XMMATRIX OrbitCamera::getViewMatrix() const
{
	return XMMatrixTranslation(-m_target.x, -m_target.y, -m_target.z) * XMMatrixRotationY(-m_angleY) *
		XMMatrixRotationX(-m_angleX) * XMMatrixTranslation(0, 0, m_distance);
}

XMVECTOR FPSCamera::getForwardDir() const
{
	auto forward = XMVectorSet(0, 0, 1, 0);
	return XMVector3TransformNormal(forward, XMMatrixRotationY(getYAngle()));
}

XMVECTOR FPSCamera::getRightDir() const
{
	auto right = XMVectorSet(1, 0, 0, 0);
	return XMVector3TransformNormal(right, XMMatrixRotationY(getYAngle()));
}

void OrbitCamera::MoveTarget(FXMVECTOR v)
{
	auto pos = XMLoadFloat4(&m_target);
	pos = pos + v;
	XMStoreFloat4(&m_target, pos);
}

void OrbitCamera::Rotate(float dx, float dy)
{
	m_angleX = XMScalarModAngle(m_angleX + dx);
	m_angleY = XMScalarModAngle(m_angleY + dy);
}

void OrbitCamera::Zoom(float dd)
{
	m_distance += dd;
	ClampDistance();
}

DirectX::XMFLOAT4 OrbitCamera::getCameraPosition() const
{
	if (m_distance == 0.0f)
		return m_target;
	XMMATRIX viewMtx = getViewMatrix();
	XMVECTOR det;
	viewMtx = XMMatrixInverse(&det, viewMtx);
	//auto alt = XMMatrixTranslation(0.0f, 0.0f, -m_distance) * XMMatrixRotationY(m_angleY) * XMMatrixRotationX(-m_angleX);
	XMFLOAT3 res(0.0f, 0.0f, 0.0f);
	auto transl = XMVector3TransformCoord(XMLoadFloat3(&res), viewMtx);
	XMStoreFloat3(&res, transl);
	return XMFLOAT4(res.x, res.y, res.z, 1.0f);

}

