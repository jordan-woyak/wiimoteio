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

#ifndef WMLIB_DEVICE_H_
#define WMLIB_DEVICE_H_

extern "C"
{
#include <Rpc.h>
#include <Setupapi.h>
#include <Hidsdi.h>
}

#include <algorithm>
#include <vector>

#include "common_types.h"

namespace wio
{

class device
{
public:
	typedef HANDLE native_handle_type;

	device()
		: m_device(native_handle_type())
		, m_use_writefile(true)
	{}

	explicit device(native_handle_type dev_handle)
		: m_device(dev_handle)
		, m_use_writefile(true)
	{
		m_overlapped.hEvent = CreateEvent(nullptr, true, true, nullptr);
		m_overlapped.Offset = 0;
		m_overlapped.OffsetHigh = 0;
	}

	device(device&& other)
		: m_device(native_handle_type())
		, m_use_writefile(true)
	{
		*this = std::move(other);
	}

	~device()
	{
		if (is_connected())
			disconnect();
	}

	device& operator=(device&& other)
	{
		if (is_connected())
			disconnect();

		swap(other);

		return *this;
	}

	void disconnect()
	{
		CloseHandle(m_device);
		m_device = native_handle_type();

		CloseHandle(m_overlapped.hEvent);
	}

	size_t read(u8* data, size_t length);
	bool write(const u8* data, size_t length);

	void swap(device& other)
	{
		std::swap(m_device, other.m_device);
		std::swap(m_overlapped, other.m_overlapped);
	}

	bool is_connected() const
	{
		return (m_device != native_handle_type());
	}

	native_handle_type native_handle() const
	{
		return m_device;
	}

private:
	device(const device&);
	device& operator=(const device&);

	native_handle_type m_device;
	bool m_use_writefile;
	OVERLAPPED m_overlapped;
};

std::vector<device> find_devices(size_t max_devices);

}	// namespace

#endif
