#include "vertexDef.h"

using namespace DirectX;
using namespace mini;

const D3D11_INPUT_ELEMENT_DESC VertexPositionNormal::Layout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(VertexPositionNormal, position), 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPositionNormal, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 }
};