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

#include "wiimote.hpp"
#include "report.hpp"

namespace wio
{

std::vector<std::unique_ptr<wiimote>> find_Wiimotes(size_t max_wiimotes)
{
	auto devs = find_devices(max_wiimotes);

	std::vector<std::unique_ptr<wiimote>> wiimotes;
	wiimotes.reserve(devs.size());

	std::for_each(devs.begin(), devs.end(), [&wiimotes](std::unique_ptr<device>& dev)
	{
		//u8 foo[22];
		//if (dev.read(foo, 22) != -1)	// TODO: ugly
			wiimotes.push_back(std::unique_ptr<wiimote>(new wiimote(std::move(dev))));
	});

	return wiimotes;
}

std::future<std::vector<u8>> wiimote::read_data(address_space _space, address_type _address, u16 _length)
{
	struct read_reply_handler : specific_report_handler<rpt::read_data_reply>
	{
		void handle_report(const report<rpt::read_data_reply>& reply)
		{
			if (reply.get_address() == (address & 0xffff))
			{
				if (0 == reply.error)
				{
					size_t const amt = std::min(bytes_left, reply.get_size());

					data.insert(data.end(), reply.data, reply.data + amt);

					address += amt;
					bytes_left -= amt;

					if (0 == bytes_left)
					{
						promise.set_value(std::move(data));
						throw remove_handler();
					}
				}
				else
				{
					// read error
					data.clear();
					promise.set_value(std::move(data));
					throw remove_handler();
				}
			}
		}

		read_reply_handler(address_type _address, size_t _length)
			: address(_address)
			, bytes_left(_length)
		{
			data.reserve(_length);
		}

		address_type address;
		u16 bytes_left;
		std::vector<u8> data;
		std::promise<std::vector<u8>> promise;

	private:
		read_reply_handler(const read_reply_handler& other) {}
		read_reply_handler& operator=(const read_reply_handler& other) { return *this; }
	};

	std::unique_ptr<read_reply_handler> handler(new read_reply_handler(_address, _length));

	auto fut = handler->promise.get_future();

	add_report_handler(std::move(handler));

	m_worker.schedule_job([this, _address, _length, _space]
	{
		report<rpt::read_data> rpt;
		rpt.set_address(_address);
		rpt.set_size(_length);
		rpt.space = _space;

		send_report(rpt);
	});

	return fut;
}

std::future<wiimote::ack_error> wiimote::write_data(address_space _space, address_type _address, const std::vector<u8>& _data)
{
	static const size_t max_write = 0x10;

	// TODO: check _data.empty() first

	auto iter = _data.begin(), iterend = _data.end();
	auto const count = (_data.size() + 0xf) / max_write;

	std::unique_ptr<ack_reply_handler<rpt::write_data>> handler(new ack_reply_handler<rpt::write_data>(count));
	auto fut = handler->promise.get_future();

	add_report_handler(std::move(handler));

	while (iter != iterend)
	{
		auto const size = std::min(_data.size(), max_write);

		// TODO: too many copies of data
		std::vector<u8> data(iter, iter + size);

		m_worker.schedule_job([size, data, _address, _space, this]
		{
			report<rpt::write_data> write_data;
			write_data.set_address(_address);
			write_data.set_size(size);
			write_data.space = _space;
			std::copy(data.begin(), data.end(), write_data.data);

			send_report(write_data);
		});

		iter += size;
		_address += size;
	}

	return fut;
}

}	// namespace
