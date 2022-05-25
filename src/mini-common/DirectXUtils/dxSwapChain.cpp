#include "dxSwapChain.h"
#include "exceptions.h"

using namespace mini;

bool DxSwapChain::Present(unsigned syncInterval, unsigned flags) const
{
	return SUCCEEDED(m_swapChain->Present(syncInterval, flags));
}
