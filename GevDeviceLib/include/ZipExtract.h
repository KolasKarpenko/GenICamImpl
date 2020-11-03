#pragma once


// STL includes
#include <vector>
#include <cstdint>

namespace gevdevice
{


class ZipExtract
{
public:
	bool Extract(std::vector<uint8_t>& outData, const std::vector<uint8_t>& inData);
};


}


