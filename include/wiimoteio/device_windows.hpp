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

extern "C"
{
#define NOMINMAX
#include <Rpc.h>
#include <Setupapi.h>
#include <Hidsdi.h>
}

#include <vector>
#include <thread>
#include <functional>

#include "common_types.hpp"

namespace wio
{

static const DWORD max_packet_size = 23;

class device
{
private:
	void init_overlaps()
	{
		m_read_overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);
		m_read_overlapped.Offset = 0;
		m_read_overlapped.OffsetHigh = 0;

		m_write_overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);
		m_write_overlapped.Offset = 0;
		m_write_overlapped.OffsetHigh = 0;
	}

public:
	typedef HANDLE native_handle_type;
	typedef u8 char_type;

	typedef std::vector<char_type> callback_data_type;
	typedef std::function<void(const callback_data_type&)> callback_type;

	device()
		: m_device(native_handle_type())
		, m_use_writefile(true)
	{
		init_overlaps();
	}

	explicit device(native_handle_type dev_handle)
		: m_device(dev_handle)
		, m_use_writefile(true)
	{
		init_overlaps();
	}

	void close()
	{
		callback_read(nullptr);

		CloseHandle(m_device);
		m_device = native_handle_type();
	}

	~device()
	{
		if (is_open())
			close();

		CloseHandle(m_read_overlapped.hEvent);
		CloseHandle(m_write_overlapped.hEvent);
	}

	size_t read(char_type* data, size_t length);
	bool write(const char_type* data, size_t length);

	void callback_read(const callback_type& callback)
	{
		if (m_callback_read_thread.joinable())
		{
			QueueUserAPC(&device::stop_callback_thread, m_callback_read_thread.native_handle(), 0);
			m_callback_read_thread.join();
		}

		if (callback)
			m_callback_read_thread = std::thread(std::mem_fun(&device::callback_read_thread_func), this, callback);
	}

	bool is_open() const
	{
		return (m_device != native_handle_type());
	}

	native_handle_type native_handle() const
	{
		return m_device;
	}

private:
	void callback_read_thread_func(const callback_type& callback)
	{
		try
		{
			std::vector<char_type> data(max_packet_size);
			while (read(data.data(), data.size()))
				callback(data);
		}
		catch (callback_thread_stop_t)
		{}
	}

	static VOID CALLBACK stop_callback_thread(ULONG_PTR)
	{
		throw callback_thread_stop_t();
	}

	struct callback_thread_stop_t {};

	// "deleted" functions
	device(const device&);
	device& operator=(const device&);

	native_handle_type m_device;
	bool m_use_writefile;
	OVERLAPPED m_read_overlapped;
	OVERLAPPED m_write_overlapped;

	std::thread m_callback_read_thread;
};


std::vector<std::unique_ptr<device>> find_devices(size_t max_devices);

}	// namespace

#include "impl/device_windows.hpp"
