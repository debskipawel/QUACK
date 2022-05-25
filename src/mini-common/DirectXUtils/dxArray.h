#pragma once
#include "dxptr.h"
#include "ptr_vector.h"

namespace mini
{
	template<typename T>
	using dx_ptr_vector = ptr_vector<T, DxDeleter<T>>;
}