#include "inputElements.h"

using namespace mini;
using namespace std;

bool operator<(const D3D11_INPUT_ELEMENT_DESC& left, const D3D11_INPUT_ELEMENT_DESC& right)
{
	if (left.InputSlot == right.InputSlot)
	{
		if (left.AlignedByteOffset == right.AlignedByteOffset)
		{
			auto cmp = strcmp(left.SemanticName, right.SemanticName);
			if (cmp == 0)
			{
				if(left.SemanticIndex == right.SemanticIndex)
				{
					if (left.Format == right.Format)
					{
						if (left.InputSlotClass == right.InputSlotClass)
							return left.InstanceDataStepRate < right.InstanceDataStepRate;
						return left.InputSlotClass < right.InputSlotClass;
					}
					return left.Format < right.Format;
				}
				return left.SemanticIndex < right.SemanticIndex;
			}
			return cmp < 0;
		}
		return left.AlignedByteOffset < right.AlignedByteOffset;
	}
	return left.InputSlot < right.InputSlot;
}

bool operator==(const D3D11_INPUT_ELEMENT_DESC& left, const D3D11_INPUT_ELEMENT_DESC& right)
{
	return left.InputSlot == right.InputSlot && left.AlignedByteOffset == right.AlignedByteOffset &&
		strcmp(left.SemanticName, right.SemanticName) == 0 && left.SemanticIndex == right.SemanticIndex &&
		left.Format == right.Format && left.InputSlotClass == right.InputSlotClass &&
		left.InstanceDataStepRate == right.InstanceDataStepRate;
}

bool operator<(const D3D11_SIGNATURE_PARAMETER_DESC& left, const D3D11_SIGNATURE_PARAMETER_DESC& right)
{
	if (left.Register == right.Register)
	{
		auto cmp = strcmp(left.SemanticName, right.SemanticName);
		if (cmp == 0)
		{
			if (left.SemanticIndex == right.SemanticIndex)
			{
				if (left.ComponentType == right.ComponentType)
				{
					if(left.Mask == right.Mask)
					{
						if (left.SystemValueType == right.SystemValueType)
						{
							if(left.ReadWriteMask == right.ReadWriteMask)
							{
								if (left.MinPrecision == right.MinPrecision)
								{
									return left.Stream < right.Stream;
								}
								return left.MinPrecision < right.MinPrecision;
							}
							return left.ReadWriteMask < right.ReadWriteMask;
						}
						return left.SystemValueType < right.SystemValueType;
					}
					return left.Mask < right.Mask;
				}
				return left.ComponentType < right.ComponentType;
			}
			return left.SemanticIndex < right.SemanticIndex;
		}
		return cmp < 0;
	}
	return left.Register < right.Register;
}

bool operator==(const D3D11_SIGNATURE_PARAMETER_DESC& left, const D3D11_SIGNATURE_PARAMETER_DESC& right)
{
	return left.Register == right.Register && strcmp(left.SemanticName, right.SemanticName) == 0 &&
		left.SemanticIndex == right.SemanticIndex && left.ComponentType == right.ComponentType &&
		left.Mask == right.Mask && left.SystemValueType == right.SystemValueType &&
		left.ReadWriteMask == right.ReadWriteMask && left.MinPrecision == right.MinPrecision &&
		left.Stream == right.Stream;
}