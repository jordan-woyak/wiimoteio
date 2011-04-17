
#ifndef WMLIB_BASIC_WIIMOTE_H_
#define WMLIB_BASIC_WIIMOTE_H_

#include <list>
#include <future>
#include <memory>

#include "device.h"
#include "report.h"
#include "worker_thread.h"

namespace wio
{

// TODO: better name
class basic_wiimote
{
public:
	typedef u32 address_type;

	basic_wiimote()
	{}

	explicit basic_wiimote(device&& _dev)
		: m_worker(new worker_thread)
	{
		m_state.reset(new shared_state);
		m_state->dev = std::move(_dev);
		m_state->run = true;

		m_reader = std::thread(&basic_wiimote::read_thread_func, m_state.get());
	}

	basic_wiimote(basic_wiimote&& other)
	{
		*this = std::move(other);
	}

	basic_wiimote& operator=(basic_wiimote&& other)
	{
		if (is_connected())
			disconnect();

		swap(other);

		return *this;
	}

	~basic_wiimote()
	{
		if (m_state)
		{
			m_state->run = false;
			m_reader.join();
		}
	}

	bool is_connected() const
	{
		return m_state && m_state->dev.is_connected();
	}

	void disconnect()
	{
		m_state->dev.disconnect();
	}

	void swap(basic_wiimote& other)
	{
		m_state.swap(other.m_state);
		m_worker.swap(other.m_worker);
		m_reader.swap(other.m_reader);
	}

	template <typename R>
	void send_report(const report<R>& _report)
	{
		//static_assert(std::is_base_of<output_report<>, R>::value, "bad report type");

		auto const state = m_state.get();

		// TODO: don't like this _report copy here
		m_worker->schedule_job([state, _report]
		{
			state->dev.write(reinterpret_cast<const u8*>(&_report), sizeof(_report));
		});
	}

	enum address_space : u8
	{
		space_eeprom = 0,
		space_register = 1,
		//space_eeprom_alt = 2,
	};

	std::future<std::vector<u8>> read_data(address_space _space, address_type _address, u16 _length);
	void write_data(address_space _space, address_type _address, const std::vector<u8>& data);

	// eeprom
	std::future<std::vector<u8>> read_eeprom(u16 _address, u16 _length)
	{ return read_data(space_eeprom, _address, _length); }

	void write_eeprom(u16 _address, const std::vector<u8>& _data)
	{ /*return*/ write_data(space_eeprom, _address, _data); }

	// register
	std::future<std::vector<u8>> read_register(address_type _address, u16 _length)
	{ return read_data(space_register, _address, _length); }

	void write_register(address_type _address, const std::vector<u8>& _data)
	{ /*return*/ write_data(space_register, _address, _data); }

private:
	basic_wiimote(const basic_wiimote&);
	basic_wiimote& operator=(const basic_wiimote&);

	struct report_handler
	{
		virtual bool handle_report(const std::vector<u8>& rpt) = 0;
	};

	// data shared between threads
	struct shared_state
	{
		device dev;

		std::mutex handler_lock;
		//std::condition_variable handler_cond;
		
		// TODO: timeouts
		std::list<std::unique_ptr<report_handler>> report_handlers;

		// TODO: ugly
		volatile bool run;
	};

	void add_report_handler(std::unique_ptr<report_handler>&& _handler)
	{
		{
		std::lock_guard<std::mutex> lk(m_state->handler_lock);
		m_state->report_handlers.push_back(std::move(_handler));
		}

		//m_handler_condvar.second->notify_one();
	}

	static void read_thread_func(shared_state*);

	std::unique_ptr<worker_thread> m_worker;
	std::thread m_reader;

	std::unique_ptr<shared_state> m_state;
};
	
}	// namespace

#endif
