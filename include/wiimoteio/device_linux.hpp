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

#include <vector>
#include <thread>
#include <functional>

#include "common_types.hpp"

#include <sys/time.h>
#include <errno.h>

#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>

namespace wio
{

class device
{
public:
	typedef int native_handle_type;

	typedef u8 char_type;

	typedef std::vector<char_type> callback_data_type;
	typedef std::function<void(const callback_data_type&)> callback_type;

	device()
		: m_sock_in()
		, m_sock_out()
	{
	}

	explicit device(native_handle_type _in, native_handle_type _out)
		: m_sock_in(_in)
		, m_sock_out(_out)
	{
	}

	device(const device&) = delete;
	device& operator=(const device&) = delete;

	void close()
	{
	}

	~device()
	{
		if (is_open())
			close();
	}

	size_t read(char_type* data, size_t length);
	bool write(const char_type* data, size_t length);

	void callback_read(const callback_type& callback)
	{
	}

	bool is_open() const
	{
		return (m_sock_in != native_handle_type());
	}

private:
	native_handle_type m_sock_in, m_sock_out;

	std::thread m_callback_read_thread;
};


std::vector<std::unique_ptr<device>> find_devices(size_t max_devices);

}	// namespace

#include "impl/device_linux.hpp"
