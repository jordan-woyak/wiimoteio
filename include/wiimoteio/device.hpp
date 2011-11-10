
#pragma once

namespace wio
{
constexpr std::size_t max_packet_size = 23;
}

#if defined(_WIN32)
	#include "device_windows.hpp"
#elif defined(__linux__)
	#include "device_linux.hpp"
#endif
