
#include <system_error>

#include "device.h"

#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Hid.lib")

namespace wio
{

static const DWORD max_packet_size = 23;
static const DWORD default_timeout = 5 * 1000;

std::vector<device> find_devices(size_t max_wiimotes)
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

	std::vector<device> found_wiimotes;
	
	// Get the device id
	GUID device_id;
	HidD_GetHidGuid(&device_id);

	// Get all hid devices connected
	HDEVINFO const device_info = SetupDiGetClassDevs(&device_id, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	DWORD index = 0;
	while (found_wiimotes.size() != max_wiimotes)
	{
		// Query the next hid device info
		SP_DEVICE_INTERFACE_DATA device_data;
		device_data.cbSize = sizeof(device_data);

		if (!SetupDiEnumDeviceInterfaces(device_info, nullptr, &device_id, index++, &device_data))
			break;

		// Get the size of the data block required
		DWORD len = 0;
		SetupDiGetDeviceInterfaceDetail(device_info, &device_data, nullptr, 0, &len, nullptr);

		std::unique_ptr<u8> const detail_array(new u8[len]);

		PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)detail_array.get();
		detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Query the data for this device
		if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_data, detail_data, len, nullptr, nullptr))
			continue;

		// Open new device
		HANDLE const dev = CreateFile(detail_data->DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
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
			found_wiimotes.push_back(device(dev));
		}
		else
			CloseHandle(dev);
	}

	SetupDiDestroyDeviceInfoList(device_info);

	return found_wiimotes;
}

size_t device::read(u8* data, size_t len)
{
	DWORD bytes_read = 0;

	//std::vector<u8> buf(max_payload);
	//buf[0] = 0xa1;
	data[0] = 0xa1;

	if (0 == ReadFile(m_device, &data[1], len, &bytes_read, &m_overlapped))
	{
		auto const err = GetLastError();
		if (ERROR_IO_PENDING == err)
		{
			// function is completing asynchronously

			if (WAIT_OBJECT_0 == WaitForSingleObject(m_overlapped.hEvent, default_timeout))
				GetOverlappedResult(m_device, &m_overlapped, &bytes_read, false);
			else
			{
				// timed out
				CancelIo(m_device);
			}
		}
		else if (ERROR_INVALID_USER_BUFFER == err)
		{
			return -1;
		}
	}

	ResetEvent(m_overlapped.hEvent);

	//data.resize(bytes_read);
	//return buff;

	return bytes_read;
}

bool device::write(const u8* data, size_t len)
{
	bool result = false;
	DWORD bytes_written = 0;

	if (m_use_writefile)
		if (WriteFile(m_device, data + 1, max_packet_size - 1, &bytes_written, &m_overlapped))
			result = true;
		else
			m_use_writefile = false;	// WriteFile failed, give HidD_SetOutputReport a try

	if (!m_use_writefile)
		result = (0 != HidD_SetOutputReport(m_device, const_cast<u8*>(data + 1), len - 1));

	return result;
}

}	// namespace