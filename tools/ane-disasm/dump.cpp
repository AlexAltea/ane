// SPDX-License-Identifier: MIT

#include "dump.h"

#include <iomanip>
#include <sstream>

std::string hex(uint64_t value)
{
	std::ostringstream oss;
	oss << "0x" << std::hex << std::uppercase << value;
	return oss.str();
}
