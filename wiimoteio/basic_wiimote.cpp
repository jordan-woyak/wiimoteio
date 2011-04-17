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

#include <numeric>

#include "basic_wiimote.h"

namespace wio
{

void basic_wiimote::read_thread_func(shared_state* state)
{
	// TODO: better quiting
	while (state->run)
	{
		std::vector<u8> report_data(22);

		// TODO: better conditional
		while (state->dev.read(report_data.data(), 22) > 0)
		{
			std::lock_guard<std::mutex> lk(state->handler_lock);

			// process input reports
			auto iter = state->report_handlers.begin(), iterend = state->report_handlers.end();
			while (iter != iterend)
				if ((*iter)->handle_report(report_data))
					iter = state->report_handlers.erase(iter);
				else
					++iter;
		}

		// TODO: remove sloppyness
		std::this_thread::yield();
	}
}

std::future<std::vector<u8>> basic_wiimote::read_data(address_space _space, address_type _address, u16 _length)
{
	report<rpt::read_data> rpt;
	rpt.set_address(_address);
	rpt.rumble = false;	// TODO: fix
	rpt.set_size(_length);
	rpt.space = _space;

	struct read_reply_handler : report_handler
	{
		bool handle_report(const std::vector<u8>& _rpt)
		{
			auto reply = reinterpret_cast<const report<rpt::read_data_reply>*>(_rpt.data());

			if (reply->is_sane() && reply->get_address() == (address & 0xffff))
			{
				if (0 == reply->error)
				{
					size_t const amt = std::min(bytes_left, reply->get_size());

					data.insert(data.end(), reply->data, reply->data + amt);

					address += amt;
					bytes_left -= amt;

					if (0 == bytes_left)
					{
						promise.set_value(std::move(data));
						return true;
					}
				}
				else
				{
					// read error
					data.clear();
					promise.set_value(std::move(data));
					return true;
				}
			}

			return false;
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
	send_report(rpt);

	return fut;
}

std::future<basic_wiimote::ack_error> basic_wiimote::write_data(address_space _space, address_type _address, const std::vector<u8>& data)
{
	auto iter = data.begin(), iterend = data.end();
	auto const count = (iterend - iter + 0xf) / 0x10;

	// TODO: check data.size() first
	std::unique_ptr<ack_reply_handler> handler(new ack_reply_handler(rpt::write_data::RPT_ID, count));
	auto fut = handler->promise.get_future();
		
	add_report_handler(std::move(handler));

	while (iter != iterend)
	{
		auto const size = std::min(iterend - iter, 0x10);

		report<rpt::write_data> write_data;
		write_data.set_address(_address);
		write_data.set_size(size);
		write_data.rumble = false;	// TODO: fix
		write_data.space = _space;	// control register

		std::copy(iter, iter + size, write_data.data);
		iter += size;

		send_report(write_data);
	}

	return fut;
}

}
