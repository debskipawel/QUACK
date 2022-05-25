#pragma once
#include <string>
#include <vector>

namespace mini
{
	namespace gk2
	{
		struct CBVariableDesc
		{
			std::string name;
			size_t offset;
			size_t size;
		};

		struct CBufferDesc
		{
			size_t size;
			std::vector<CBVariableDesc> variables;
		};
	}
}
