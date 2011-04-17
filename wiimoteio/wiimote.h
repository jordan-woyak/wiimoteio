
#ifndef WMLIB_WIIMOTE_H_
#define WMLIB_WIIMOTE_H_

#include <vector>
#include <list>

extern "C"
{
#include <Rpc.h>
#include <Setupapi.h>
#include <Hidsdi.h>
}

#include "common_types.h"
#include "report.h"
#include "basic_wiimote.h"

namespace wio
{

class wiimote
{
public:
	wiimote()
	{}

	explicit wiimote(basic_wiimote&& _wm)
		: m_wiimote(std::move(_wm))
	{}

	wiimote(wiimote&& other)
	{
		*this = std::move(other);
	}

	wiimote& operator=(wiimote&& other)
	{
		if (is_connected())
			disconnect();

		swap(other);

		return *this;
	}

	basic_wiimote& basic()
	{
		return m_wiimote;
	}

	void disconnect()
	{
		m_wiimote.disconnect();
	}

	void swap(wiimote& other)
	{
		m_wiimote.swap(other.m_wiimote);
	}

	bool is_connected() const
	{
		return m_wiimote.is_connected();
	}

	// HLE stuff, would like to move elsewhere
	void set_leds(u8 _leds)
	{
		m_status.leds = _leds;

		report<rpt::leds> leds;
		leds.rumble = false;
		leds._leds = _leds;

		m_wiimote.send_report(leds);
	}

	void set_rumble(bool _rumble)
	{
		m_rumble = _rumble;

		report<rpt::leds> leds;
		leds.rumble = m_rumble;
		leds._leds = m_status.leds;

		m_wiimote.send_report(leds);
	}

	void request_status()
	{
		report<rpt::status_request> request;
		request.rumble = m_rumble;

		m_wiimote.send_report(request);
	}

private:
	wiimote(const wiimote&);
	wiimote& operator=(const wiimote&);

	//struct
	//{
	//	void enable(int features);
	//	int status();
	//	int available();

	//} features;

	//int m_flags;

	basic_wiimote m_wiimote;

	rpt::status m_status;
	bool m_rumble;
};

std::vector<wiimote> find_Wiimotes(size_t max_wiimotes);

}	// namespace

#endif
