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

#include "impl/wiimote_speaker.h"

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

enum : u8
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

	// TODO:
	//wiimote()
	//{}

	explicit wiimote(device&& _dev)
		: m_device(std::move(_dev))
	{
		m_state.remote_rumble = false;
		m_state.rumble.store(false);

		m_state.present_ext.store(false);

		m_state.report_button.store(false);
		m_state.report_accel.store(false);
		m_state.report_ir.store(false);
		m_state.report_gyro.store(false);
		m_state.report_ext.store(false);

		m_state.battery.store(0);
		m_state.leds.store(0);
		m_state.extid.store(extid::nothing);

		m_device.callback_read([this](const device::callback_data_type& _rpt)
		{
			// hax
			if (0x22 == _rpt[1] && 0x18 == _rpt[4])
				printf("speaker data ack error: %d\n", (int)_rpt[5]);

			std::lock_guard<std::mutex> lk(m_read_handler_lock);

			// process report
			for (auto iter = m_read_handlers.begin(); iter != m_read_handlers.end();)
			{
				try
				{
					(*iter)->handle_report(_rpt);
					++iter;
				}
				catch (wiimote::report_handler::remove_handler)
				{
					iter = m_read_handlers.erase(iter);
					return;
				}
				catch (wiimote::report_handler::fully_handled)
				{
					return;
				}
			}

			// check for unhandled status report
			if (auto status = report_cast<rpt::status>(_rpt))
			{
				//printf("send mode\n");
				update_reporting_mode();
			}
		});

		request_status();
		
		update_accel_calibration();

		// TODO: only add when features are enabled
		add_report_handler(std::bind(&wiimote::handle_button_data_report, this, std::placeholders::_1));
		add_report_handler(std::bind(&wiimote::handle_accel_data_report, this, std::placeholders::_1));

		m_worker.schedule_job([this]
		{
			report<rpt::status_request> status_rq;
			send_report(status_rq);
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

			report<rpt::player_leds> leds;
			leds.leds = _leds;
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
		return (battery_level)m_state.battery.load() / 0xc0;
	}

	// speaker mute
	//bool get_speaker_mute() const
	//{}

	void set_speaker_mute(bool _mute)
	{
		//report<rpt::speaker_mute> mute_rpt;
		//mute_rpt.enable = _mute;

		//m_wiimote.send_report(mute_rpt);
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
	
	// enable/disable input features

	bool get_report_button() const
	{ return m_state.report_button.load(); }

	bool set_report_button(bool _enable)
	{
		m_state.report_button.store(_enable);
		
		update_reporting_mode();

		return true;	// button data is always available
	}

	bool get_report_accel() const
	{ return m_state.report_accel.load(); }

	bool set_report_accel(bool _enable)
	{
		m_state.report_accel.store(_enable);

		update_reporting_mode();

		return true;
	}

	bool get_report_ir() const
	{ return m_state.report_ir.load(); }

	bool set_report_ir(bool _enable)
	{
		m_state.report_ir.store(_enable);
		return false;
	}

	bool get_report_ext() const
	{ return m_state.report_ext.load(); }

	bool set_report_ext(bool _enable)
	{
		m_state.report_ext.store(_enable);

		update_reporting_mode();

		return false;
	}

	bool get_report_gyro() const
	{ return m_state.report_gyro.load(); }

	bool set_report_gyro(bool _enable)
	{
		m_state.report_gyro.store(_enable);
		return false;
	}

	enum : worker_thread::job_type
	{
		job_type_speaker,
	};

	typedef u32 speaker_rate_type;

	speaker_rate_type get_speaker_rate() const
	{
		return m_state.speaker_rate;
	};

	void set_speaker_rate(speaker_rate_type _rate)
	{
		m_state.speaker_rate = _rate;
	};

	enum speaker_format_type : u8
	{
		format_adpcm = 0x00,
		format_pcm = 0x40,
	};

	speaker_format_type get_speaker_format() const
	{
		return m_state.speaker_fmt;
	};

	void set_speaker_format(speaker_format_type fmt)
	{
		m_state.speaker_fmt = fmt;
	};

	typedef float speaker_volume_type;

	speaker_volume_type get_speaker_volume() const
	{
		return m_state.speaker_vol;
	};

	void set_speaker_volume(speaker_volume_type vol)
	{
		m_state.speaker_vol = vol;
	};

	template <typename S>
	void speaker_stream(S&& _strm);

	struct input_state
	{
		// TODO: assumed atomic
		core_button_t button;
		//std::shared_ptr<std::vector<u8>> last_data_report;

		
		std::array<calibrated_int<u16>, 3> accel;

		const std::array<calibrated_int<u16>, 3>& get_accel() const
		{
			return accel;
		}
	};

	typedef core_button_t input_button_type;
	const input_button_type& get_input_button() const
	{
		return m_input_state.button;
	}

	typedef std::array<calibrated_int<u16>, 3> input_accel_type;
	const input_accel_type& get_input_accel() const
	{
		return m_input_state.accel;
	}

	typedef void* input_ir_type;
	const input_ir_type& get_input_ir() const;

	typedef void* input_ext_type;
	const input_ext_type& get_input_ext() const;

	typedef void* input_gyro_type;
	const input_gyro_type& get_input_gyro() const;

	bool get_present_ext() const
	{ return m_state.present_ext.load(); }

	bool get_present_gyro() const
	{ return false; }

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

	worker_thread::clock::time_point speaker_start_time;
	size_t speaker_sample_number;

	template <typename S>
	void speaker_stream_some(S stream);

	// sets the rumble bit on the given output report and sends it
	template <typename R>
	void send_report(report<R>& _report)
	{
		static_assert(std::is_base_of<output_report<R::payload::rpt_id>, R>::value, "bad report type");

		_report.motor = m_state.remote_rumble = m_state.rumble.load();

		m_device.write(reinterpret_cast<const u8*>(&_report), sizeof(_report));
	}

	struct report_handler
	{
		enum remove_handler {};
		enum fully_handled {};

		virtual void handle_report(const std::vector<u8>&) = 0;
	};

	template <typename R>
	struct specific_report_handler : report_handler
	{
		void handle_report(const std::vector<u8>& _rpt)
		{
			if (auto result = report_cast<R>(_rpt))
			{
				handle_report(*result);
				throw fully_handled();
			}
		}

		virtual void handle_report(const report<R>&) = 0;
	};

	template <typename R>
	struct ack_reply_handler : specific_report_handler<rpt::ack>
	{
		void handle_report(const report<rpt::ack>& reply)
		{
			if (reply.ack_id == R::payload::rpt_id && 0 == --counter)
			{
				promise.set_value(reply.error);
				throw remove_handler();
			}
		}

		ack_reply_handler(size_t _counter = 1)
			: counter(_counter)
		{}

		size_t counter;
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
		bool remote_rumble;
		std::atomic<u8> battery;
		std::atomic<bool> present_ext;
		std::atomic<extid_t> extid;
	
		volatile speaker_rate_type speaker_rate;
		volatile speaker_format_type speaker_fmt;
		volatile speaker_volume_type speaker_vol;

		std::future<ack_error> speaker_data_ack;
		report<rpt::speaker_data> speaker_report;
	
		// features
		std::atomic<bool>
			report_button,
			report_accel,
			report_ir,
			report_gyro,
			report_ext;

	} m_state;

	input_state m_input_state;

	void request_status()
	{
		auto status_handler = [this](const report<rpt::status>& status)
		{
			//printf("got status\n");
			m_state.battery.store(status.battery);
			m_state.present_ext.store(status.extension);
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

				//data = read_register(0xa400fa, 6).get();

				//extid_t val = extid::nothing;
				//if (!data.empty())
				//	val = (data[0] << 8) | data[5];

				//m_state.extid.store(val);
			}

			throw wiimote::report_handler::remove_handler();
		};

		add_specific_report_handler<rpt::status>(std::move(status_handler));

		m_worker.schedule_job([this]
		{
			send_report(report<rpt::status_request>());
		});
	}

	void add_report_handler(std::unique_ptr<report_handler>&& _handler)
	{
		std::lock_guard<std::mutex> lk(m_read_handler_lock);
		m_read_handlers.push_back(std::move(_handler));
	}

	void add_report_handler(const std::function<void(const std::vector<u8>&)>& _func)
	{
		struct func_handler : report_handler
		{
			void handle_report(const std::vector<u8>& _rpt)
			{
				func(_rpt);
			}

			std::function<void(const std::vector<u8>&)> func;
		};

		std::unique_ptr<func_handler> handler(new func_handler);
		handler->func = _func;

		add_report_handler(std::move(handler));
	}

	template <typename R>
	void add_specific_report_handler(const std::function<void(const report<R>&)>& _func)
	{
		struct func_handler : specific_report_handler<R>
		{
			void handle_report(const report<R>& _rpt)
			{
				return func(_rpt);
			}

			std::function<void(const report<R>&)> func;
		};

		std::unique_ptr<func_handler> handler(new func_handler);
		handler->func = _func;

		add_report_handler(std::move(handler));
	}

	void update_reporting_mode()
	{
		// TODO: ir, gyro and stuff

		u8 mode = rpt::data::button::payload::rpt_id;

		if (m_state.report_accel.load() && m_state.report_ext.load())
			mode = rpt::data::button_accel_ext16::payload::rpt_id;
		else if (m_state.report_accel.load())
			mode = rpt::data::button_accel::payload::rpt_id;

		m_worker.schedule_job([this, mode]
		{
			report<rpt::report_mode> mode_rpt;
			mode_rpt.continuous = true;
			mode_rpt.mode = mode;

			send_report(mode_rpt);
		});
	}

	void update_accel_calibration()
	{
		auto data = read_accel_calibration_data().get();
		if (data.empty())
			return;		// TODO: error handling

		// TODO: check checksum

		// zero G
		m_input_state.accel[0].zero = data[0] << 2;
		m_input_state.accel[1].zero = data[1] << 2;
		m_input_state.accel[2].zero = data[2] << 2;

		set_bits(m_input_state.accel[0].zero, 0, 2, get_bits(data[3], 4, 2));
		set_bits(m_input_state.accel[1].zero, 0, 2, get_bits(data[3], 2, 2));
		set_bits(m_input_state.accel[2].zero, 0, 2, get_bits(data[3], 0, 2));

		// one G
		m_input_state.accel[0].pos = data[4] << 2;
		m_input_state.accel[1].pos = data[5] << 2;
		m_input_state.accel[2].pos = data[6] << 2;

		set_bits(m_input_state.accel[0].pos, 0, 2, get_bits(data[7], 4, 2));
		set_bits(m_input_state.accel[1].pos, 0, 2, get_bits(data[7], 2, 2));
		set_bits(m_input_state.accel[2].pos, 0, 2, get_bits(data[7], 0, 2));
	}

	void handle_button_data_report(const std::vector<u8>& _rpt);
	void handle_accel_data_report(const std::vector<u8>& _rpt);

	std::future<std::vector<u8>> read_accel_calibration_data()
	{
		return read_eeprom(0x16, 0x14);
	};

	// region is 0xa4 or 0xa6
	std::future<std::vector<u8>> read_ext_calibration_data(u8 _region)
	{
		return read_register((_region << 0x10) | 0x20, 0x20);
	};

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

inline void wiimote::handle_button_data_report(const std::vector<u8>& _rpt)
{
	if (_rpt.size() >= 4 && _rpt[1] != rpt::data::ext21::payload::rpt_id)
	{
		//printf("button\n");
		m_input_state.button = *reinterpret_cast<const core_button_t*>(&_rpt[2]);
		//printf("%d", m_state.button.load());
	}
}

inline void wiimote::handle_accel_data_report(const std::vector<u8>& _rpt)
{
	auto const set_accel = [this, _rpt](const rpt::data::accel_base& _accel)
	{
		// x
		m_input_state.accel[0].val = _accel.accel.x << 2;
		set_bits(m_input_state.accel[0].val, 0, 2, get_bits(_rpt[2], 5, 2));
		// y
		m_input_state.accel[1].val = _accel.accel.y << 2;
		set_bit(m_input_state.accel[1].val, 1, get_bit(_rpt[3], 5));
		// z
		m_input_state.accel[2].val = _accel.accel.z << 2;
		set_bit(m_input_state.accel[2].val, 1, get_bit(_rpt[3], 6));
	};

	if (auto rpt = report_cast<rpt::data::button_accel>(_rpt))
		set_accel(*rpt);
	else if (auto rpt = report_cast<rpt::data::button_accel_ir12>(_rpt))
		set_accel(*rpt);
	else if (auto rpt = report_cast<rpt::data::button_accel_ext16>(_rpt))
		set_accel(*rpt);
	else if (auto rpt = report_cast<rpt::data::button_accel_ir10_ext6>(_rpt))
		set_accel(*rpt);
}

}	// namespace

#endif
