/*
Copyright (c) 2011 - wiimoteio project

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#pragma once

//#include <system_error>
#include <array>

namespace wio
{

std::vector<std::unique_ptr<device>> find_devices(size_t max_wiimotes)
{
	// TODO: eliminate redundancy !!!

	// VID = Nintendo, PID = Wiimote
	// must be sorted!
	static std::pair<u16, u16> const vid_pids[] =
	{
		std::make_pair(0x0001, 0x0002),
		std::make_pair(0x0002, 0x00f7),
		std::make_pair(0x057e, 0x0306),
	};

	//auto const vid_pids_end = vid_pids + 3;	// TODO: s/3/sizeof...

	std::vector<std::unique_ptr<device>> found_devices;

	return found_devices;
}

size_t device::read(char_type* data, size_t len)
{
	return 0;
}

bool device::write(const char_type* _data, size_t _len)
{
	return (::write(m_sock_out, _data, _len) == static_cast<int>(_len));
}

}	// namespace
