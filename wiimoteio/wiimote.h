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

#ifndef WMLIB_WIIMOTE_H_
#define WMLIB_WIIMOTE_H_

#include <vector>
#include <list>
#include <chrono>
#include <array>
#include <atomic>
#include <future>

extern "C"
{
#include <Rpc.h>
#include <Setupapi.h>
#include <Hidsdi.h>
}

#include "common_types.h"
#include "report.h"
#include "device.h"
#include "worker_thread.h"
#include "data_report.h"
#include "extension.h"

namespace wio
{

typedef std::pair<std::array<u8, 9>, std::array<u8, 2>> ir_sensitivity;

namespace leds
{

enum
{
	_0000, _0001, _0010, _0011,
	_0100, _0101, _0110, _0111,
	_1000, _1001, _1010, _1011,
	_1100, _1101, _1110, _1111,
	end
};

static u8 player(u8 p)
{ return (_0001 << p); };

}	// namespace

class wiimote
{
public:
	// TODO: put elsewhere
	typedef u8 ack_error;
	typedef u32 address_type;

	wiimote()
	{}

	explicit wiimote(device&& _dev)
		//: m_device(std::move(_dev))
	{
		m_state.rumble.store(false);
		m_state.battery.store(0);
		m_state.leds.store(0);
		m_state.extid.store(extid::nothing);

		m_device = std::move(_dev);

		m_device.callback_read([this](const device::callback_data_type& _rpt)
		{
			std::lock_guard<std::mutex> lk(m_read_handler_lock);

			// process report
			auto iter = m_read_handlers.begin(), iterend = m_read_handlers.end();
			while (iter != iterend)
				if ((*iter)->handle_report(_rpt))
					iter = m_read_handlers.erase(iter);
				else
					++iter;
		});

		struct status_handler : specific_report_handler<rpt::status>
		{
			bool handle_report(const report<rpt::status>& status)
			{
				func(status);

				return false;
			}

			std::function<void(const report<rpt::status>& status)> func;
		};

		std::unique_ptr<status_handler> handler(new status_handler);

		handler->func = [this](const report<rpt::status>& status)
		{
			printf("got status\n");
			m_state.battery.store(status.battery);
			//m_state.leds.store(status.leds);

			// TODO: only do this after enabling ext feature
			// read extid
			if (status.extension)
			{
				// initialize extension
				//std::vector<u8> data(1);
				//data[0] = 0x55;
				//write_register(0xa400f0, data);
				//data[0] = 0x00;
				//write_register(0xa400fb, data);//.wait();

			//	data = read_register(0xa400fa, 6).get();

			//	extid_t val = extid::nothing;
			//	if (!data.empty())
			//		val = (data[0] << 8) | data[5];

			//	m_state.extid.store(val);
			}
		};

		add_report_handler(std::move(handler));

		m_worker.schedule_job([this]
		{
			report<rpt::status_request> status_rq;
			status_rq.rumble = m_state.rumble.load();
			
			// TODO: testing
			send_report(status_rq);
			//std::this_thread::sleep_for(std::chrono::milliseconds(50));
			//send_report(status_rq);
			//send_report(status_rq);
		});
	}

	~wiimote()
	{
		m_worker.remove_all();
		m_device.callback_read(nullptr);
		m_device.close();
	}

	void close()
	{
		m_device.callback_read(nullptr);
		m_device.close();
	}

	bool is_open() const
	{
		return m_device.is_open();
	}

	u8 get_leds() const
	{
		return m_state.leds.load();
	}

	//std::future<ack_error> set_leds(u8 _leds)
	void set_leds(u8 _leds)
	{
		//std::unique_ptr<ack_reply_handler> handler(new ack_reply_handler(rpt::leds::RPT_ID));

		//auto fut = handler->promise.get_future();

		//add_report_handler(std::move(handler));

		m_state.leds.store(_leds);

		m_worker.schedule_job([this, _leds]
		{
			//m_state.leds.store(_leds);

			report<rpt::leds> leds;
			leds.rumble = m_state.rumble.load();
			leds._leds = _leds;
			send_report(leds);
		});

		//return fut;
	}

	void set_rumble(bool _rumble)
	{
		m_state.rumble.store(_rumble);

		report<rpt::rumble> rumble_rpt;
		//rumble_rpt._rumble = m_rumble;

		//m_wiimote.send_report(rumble_rpt);
	}

private:
	template <typename C, typename D>
	void rumble_until(const std::chrono::time_point<C, D>& abs)
	{
		set_rumble(true);

		// TOOD: super broken
		m_worker.schedule_job_at([this]
		{
			set_rumble(false);

		}, abs);
	}

public:
	template <typename R, typename P>
	void rumble_for(const std::chrono::duration<R, P>& rel)
	{
		rumble_until(std::chrono::steady_clock::now() + rel);
	}

	typedef float battery_level;
	
	battery_level get_battery_level() const
	{
		//return (battery_level)m_state.battery.load() / std::numeric_limits<u8>::max();

		return (battery_level)m_state.battery.load() / 0xc0;
	}

	// speaker mute
	//bool get_speaker_mute() const
	//{}

	void set_speaker_mute(bool _mute)
	{
		//report<rpt::speaker_mute> mute_rpt;
		//mute_rpt.rumble = false;	// TODO:
		//mute_rpt.enable = _mute;

		//m_wiimote.send_report(mute_rpt);
	}

	void request_status()
	{
		//report<rpt::status_request> request;
		//request.rumble = m_rumble;

		//m_wiimote.send_report(request);
	}

	void set_ir_sensitivity(const ir_sensitivity& sens)
	{
		//write_register(0xb00000, sens.first);
		//write_register(0xb0001a, sens.second);
	}

	enum address_space : u8
	{
		space_eeprom = 0,
		space_register = 1,
		//space_eeprom_alt = 2,
	};

	std::future<std::vector<u8>> read_data(address_space _space, address_type _address, u16 _length);
	std::future<ack_error> write_data(address_space _space, address_type _address, const std::vector<u8>& data);

	// eeprom
	std::future<std::vector<u8>> read_eeprom(u16 _address, u16 _length)
	{ return read_data(space_eeprom, _address, _length); }

	std::future<ack_error> write_eeprom(u16 _address, const std::vector<u8>& _data)
	{ return write_data(space_eeprom, _address, _data); }

	// register
	std::future<std::vector<u8>> read_register(address_type _address, u16 _length)
	{ return read_data(space_register, _address, _length); }

	std::future<ack_error> write_register(address_type _address, const std::vector<u8>& _data)
	{ return write_data(space_register, _address, _data); }

	enum features
	{
		feat_none,
	
		feat_button,
		feat_accel,
		feat_ext,
		feat_ir,
		feat_gyro,
		
		feat_all = feat_button | feat_accel | feat_ext | feat_ir | feat_gyro,
	};
	
	features get_features() const
	{

	}

	void set_features(features feats)
	{
		if (feats & feat_button)
		{
			m_worker.schedule_job([this]
			{
				report<rpt::report_mode> mode_rpt;
				//mode_rpt.continuous = true;
				mode_rpt.continuous = false;
				mode_rpt.rumble = m_state.rumble.load();
				mode_rpt.mode = rptmode::button;
				//mode_rpt.mode = 0;

				send_report(mode_rpt);
			});
		}
	}

	// TODO: asyncronous
	extid_t get_extension_id()
	{
#if 1
		// initialize extension
		std::vector<u8> data(1);
		data[0] = 0x55;
		write_register(0xa400f0, data);
		data[0] = 0x00;
		write_register(0xa400fb, data).wait();

		data = read_register(0xa400fa, 6).get();

		extid_t val = extid::nothing;
		if (!data.empty())
			val = (data[0] << 8) | data[5];

		m_state.extid.store(val);
#endif

		return m_state.extid.load();
	}

private:
	wiimote(const wiimote&);
	wiimote& operator=(const wiimote&);

	template <typename R>
	void send_report(const report<R>& _report)
	{
		//static_assert(std::is_base_of<output_report<>, R>::value, "bad report type");

		m_device.write(reinterpret_cast<const u8*>(&_report), sizeof(_report));
	}

	template <typename R>
	void schedule_report(const report<R>& _report)
	{
		//static_assert(std::is_base_of<output_report<>, R>::value, "bad report type");

		// TODO: don't like this _report copy here
		m_worker.schedule_job([this, _report]
		{
			send_report(_report);
		});
	}

	struct report_handler
	{
		virtual bool handle_report(const std::vector<u8>&) = 0;
	};

	template <typename R>
	struct specific_report_handler : report_handler
	{
		bool handle_report(const std::vector<u8>& _rpt)
		{
			auto result = report_cast<R>(_rpt);
			
			if (result)
				return handle_report(*result);
			else
				return false;
		}

		virtual bool handle_report(const report<R>&) = 0;
	};

	struct ack_reply_handler : specific_report_handler<rpt::ack>
	{
		bool handle_report(const report<rpt::ack>& reply)
		{
			if (reply.ack_id == rpt_id && 0 == --counter)
			{
				promise.set_value(reply.error);
				return true;
			}
			
			return false;
		}

		ack_reply_handler(u8 _rpt_id, size_t _counter = 1)
			: rpt_id(_rpt_id), counter(_counter)
		{}

		size_t counter;
		u8 rpt_id;
		std::promise<ack_error> promise;

	private:
		ack_reply_handler(const ack_reply_handler& other);
		ack_reply_handler& operator=(const ack_reply_handler& other);
	};

	device m_device;

	worker_thread m_worker;

	std::list<std::unique_ptr<report_handler>> m_read_handlers;

	std::mutex m_read_handler_lock;

	struct
	{
		std::atomic<u8> leds;	// TODO: atomic not needed
		std::atomic<bool> rumble;
		std::atomic<u8> battery;
		std::atomic<extid_t> extid;
	
	} m_state;

	void add_report_handler(std::unique_ptr<report_handler>&& _handler)
	{
		{
		std::lock_guard<std::mutex> lk(m_read_handler_lock);
		m_read_handlers.push_back(std::move(_handler));
		}

		//m_handler_condvar.second->notify_one();
	}

	void read_thread_func();

	//struct
	//{
	//	void enable(int features);
	//	int status();
	//	int available();

	//} features;

	//int m_flags;

	//basic_wiimote m_wiimote;

	//rpt::status m_status;
	//bool m_rumble;
};

std::vector<std::unique_ptr<wiimote>> find_Wiimotes(size_t max_wiimotes);

}	// namespace

#endif
