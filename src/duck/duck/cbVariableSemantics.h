#pragma once

namespace mini
{
	namespace gk2
	{
		enum class VariableSemantic
		{
			//start view related
			MatV,
			MatVT,
			MatVInv,
			MatVInvT,
			Vec4CamPos,
			Vec4CamDir,
			Vec4CamRight,
			Vec4CamUp,
			//start proj related
			MatVP,
			MatVPT,
			MatVPInv,
			MatVPInvT,
			//end view related
			MatP,
			MatPT,
			MatPInv,
			MatPInvT,
			Vec2ViewportDims,
			FloatFOV,
			FloatNearPlane,
			FloatFarPlane,
			//end proj related
			//start model related
			MatM,
			MatMT,
			MatMInv,
			MatMInvT,
			MatMV,
			MatMVT,
			MatMVInv,
			MatMVInvT,
			MatMVP,
			MatMVPT,
			MatMVPInv,
			MatMVPInvT,
			//end model related
			//start clock related
			FloatDT,
			FloatT,
			FloatFPS,
			FloatTotalFrames
			//end clock related
		};
	}
}