#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

namespace mini
{
	struct VertexPositionNormal
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;

		static const D3D11_INPUT_ELEMENT_DESC Layout[2];
	};
}