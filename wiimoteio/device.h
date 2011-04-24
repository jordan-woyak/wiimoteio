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

#include <vector>
#include <thread>
#include <functional>

#include "common_types.h"

namespace wio
{

class device
{
public:
	typedef HANDLE native_handle_type;

	typedef std::vector<u8> callback_data_type;
	typedef std::function<void(const callback_data_type&)> callback_type;

	device()
		: m_device(native_handle_type())
		, m_use_writefile(true)
	{}

	explicit device(native_handle_type dev_handle)
		: m_device(dev_handle)
		, m_use_writefile(true)
	{
		m_read_overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);
		m_read_overlapped.Offset = 0;
		m_read_overlapped.OffsetHigh = 0;

		m_write_overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);
		m_write_overlapped.Offset = 0;
		m_write_overlapped.OffsetHigh = 0;
	}

	device(device&& other)
		: m_device(native_handle_type())
		, m_use_writefile(true)
	{
		*this = std::move(other);
	}

	~device()
	{
		callback_read(nullptr);

		if (is_open())
			close();
	}

	device& operator=(device&& other)
	{
		if (is_open())
			close();

		swap(other);

		return *this;
	}

	bool open();

	void close()
	{
		// TODO: safe?
		CancelIoEx(m_device, &m_read_overlapped);
		//CancelIoEx(m_device, &m_write_overlapped);

		CloseHandle(m_device);
		m_device = native_handle_type();

		CloseHandle(m_read_overlapped.hEvent);
		CloseHandle(m_write_overlapped.hEvent);
	}

	size_t read(u8* data, size_t length);
	bool write(const u8* data, size_t length);

	void callback_read(const callback_type& callback)
	{
		if (m_callback_read_thread.joinable())
		{
			QueueUserAPC(&device::stop_callback_thread, m_callback_read_thread.native_handle(), 0);
			m_callback_read_thread.join();
		}

		if (callback)
			m_callback_read_thread = std::thread(&device::callback_read_thread_func, m_device, callback_type(callback));
	}

	void swap(device& other)
	{
		std::swap(m_device, other.m_device);
		std::swap(m_use_writefile, other.m_use_writefile);
		std::swap(m_read_overlapped, other.m_read_overlapped);
		std::swap(m_write_overlapped, other.m_write_overlapped);
		m_callback_read_thread.swap(other.m_callback_read_thread);
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
	template <typename D>
	static VOID CALLBACK completion_routine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		auto data = *static_cast<D*>(lpOverlapped->hEvent);

		if (dwNumberOfBytesTransfered)
		{
			data.data.resize(dwNumberOfBytesTransfered + 1);
			data.callback(data.data);
		}
	}

	struct callback_thread_stop_type {};

	static void callback_read_thread_func(native_handle_type handle, const callback_type& callback)
	{
		struct user_data
		{
			user_data(const callback_type& _callback)
				: callback(_callback)
			{
				data.push_back(0xa1);
			}

			std::vector<u8> data;
			const callback_type& callback;

		} data(callback);

		OVERLAPPED overlapped;
		overlapped.hEvent = static_cast<HANDLE>(&data);
		overlapped.Offset = 0;
		overlapped.OffsetHigh = 0;

		try
		{
			while (true)
			{
				data.data.resize(23);
				ReadFileEx(handle, &data.data[1], 22, &overlapped, &device::completion_routine<user_data>);
				SleepEx(INFINITE, true);
			}
		}
		catch (callback_thread_stop_type)
		{
			CancelIo(handle);
		}
	}

	static VOID CALLBACK stop_callback_thread(ULONG_PTR)
	{
		throw callback_thread_stop_type();
	}

	device(const device&);
	device& operator=(const device&);

	native_handle_type m_device;
	bool m_use_writefile;
	OVERLAPPED m_read_overlapped;
	OVERLAPPED m_write_overlapped;

	std::thread m_callback_read_thread;
};

std::vector<device> find_devices(size_t max_devices);

inline bool device::open()
{
	if (!is_open())
	{
		auto devs = find_devices(1);
		if (!devs.empty())
			swap(devs.front());
	}

	return is_open();
}

}	// namespace

#endif
