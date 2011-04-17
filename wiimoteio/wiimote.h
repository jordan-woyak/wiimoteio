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
