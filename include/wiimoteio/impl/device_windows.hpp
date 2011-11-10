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

//#include <system_error>
#include <array>

#include "device.hpp"

#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Hid.lib")

namespace wio
{

std::vector<std::unique_ptr<device>> find_devices(size_t max_wiimotes)
{
	// VID = Nintendo, PID = Wiimote
	// must be sorted!
	static std::pair<USHORT, USHORT> const vid_pids[] =
	{
		std::make_pair(0x0001, 0x0002),
		std::make_pair(0x0002, 0x00f7),
		std::make_pair(0x057e, 0x0306),
	};

	auto const vid_pids_end = vid_pids + 3;	// TODO: s/3/sizeof...

	std::vector<std::unique_ptr<device>> found_devices;
	
	// Get the device id
	GUID device_id;
	HidD_GetHidGuid(&device_id);

	// Get all hid devices connected
	HDEVINFO const device_info = SetupDiGetClassDevs(&device_id, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	DWORD index = 0;
	while (found_devices.size() != max_wiimotes)
	{
		// Query the next hid device info
		SP_DEVICE_INTERFACE_DATA device_data;
		device_data.cbSize = sizeof(device_data);

		if (!SetupDiEnumDeviceInterfaces(device_info, nullptr, &device_id, index++, &device_data))
			break;

		// Get the size of the data block required
		DWORD len = 0;
		SetupDiGetDeviceInterfaceDetail(device_info, &device_data, nullptr, 0, &len, nullptr);

		std::unique_ptr<u8[]> const detail_array(new u8[len]);

		PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)detail_array.get();
		detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Query the data for this device
		if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_data, detail_data, len, nullptr, nullptr))
			continue;

		// Open new device
		HANDLE const dev = CreateFile(detail_data->DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			0, nullptr,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
		
		if (INVALID_HANDLE_VALUE == dev)
			continue;

		// Get device attributes
		HIDD_ATTRIBUTES attr;
		attr.Size = sizeof(attr);

		HidD_GetAttributes(dev, &attr);
		if (std::binary_search(vid_pids, vid_pids_end, std::make_pair(attr.VendorID, attr.ProductID)))
		{
			// This is a wiimote
			found_devices.push_back(std::unique_ptr<device>(new device(dev)));
		}
		else
			CloseHandle(dev);
	}

	SetupDiDestroyDeviceInfoList(device_info);

	return found_devices;
}

size_t device::read(char_type* data, size_t len)
{
	DWORD bytes_read = 0;

	data[0] = 0xa1;

	ResetEvent(m_read_overlapped.hEvent);

	if (0 == ReadFile(m_device, &data[1], len - 1, &bytes_read, &m_read_overlapped))
	{
		auto const err = GetLastError();
		if (ERROR_IO_PENDING == err)
		{
			WaitForSingleObjectEx(m_read_overlapped.hEvent, INFINITE, true);
			if (GetOverlappedResult(m_device, &m_read_overlapped, &bytes_read, false))
				return bytes_read + 1;
		}
	}
	else
		return bytes_read + 1;

	return 0;
}

bool device::write(const char_type* _data, size_t len)
{
	DWORD bytes_written = 0;	// not really used

	// TODO: lame
	std::array<char_type, max_packet_size> data;
	std::copy(_data, _data + len, data.begin());

	ResetEvent(m_write_overlapped.hEvent);	// TODO: event isn't being used

	if (m_use_writefile)
	{
		if (0 == WriteFile(m_device, &data[1], max_packet_size - 1, &bytes_written, &m_write_overlapped))
		{
			// BlueSoleil always returns true, give HidD_SetOutputReport a try (MS stack)
			m_use_writefile = false;
		}
		else
			return true;
	}

	return (0 != HidD_SetOutputReport(m_device, &data[1], max_packet_size - 1));
}

}	// namespace
