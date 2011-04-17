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

#ifndef WMLIB_REPORT_H_
#define WMLIB_REPORT_H_

#include "common_types.h"

namespace wio
{

typedef u16 core_button_t;

template <typename T>
T convert_from_big_endian(const void* data, u8 size = sizeof(T))
{
	T n = 0;
	// TODO:
	return n;
};

template <typename T>
T convert_to_big_endian(const void* data, u8 size = sizeof(T))
{
	T n = 0;
	// TODO:
	return n;
};

enum : u8
{
	led_1 = (1 << 0),
	led_2 = (1 << 1),
	led_3 = (1 << 2),
	led_4 = (1 << 3),
};

namespace hid_type
{
enum
{
	handshake =		0x0,
	set_report =	0x5,
	data =			0xA,
};
}

namespace hid_param
{
enum
{
	input =		0x1,
	output =	0x2,
};
}

struct hid_report
{
	template <typename R>
	hid_report(R* rpt)
		: data((u8*)rpt), size(sizeof(*rpt))
	{}

	const u8 *const data;
	const u8 size;
};

struct hid_header
{
	hid_header(const u8 _type, const u8 _param, const u8 _rpt_id)
		: hid_cmd(_type, _param), rpt_id(_rpt_id)
	{}

	struct HidCmd
	{
		HidCmd(const u8 _type, const u8 _param)
			: type(_type), param(_param)
		{}

		u8	param : 4;
		u8	type : 4;

	} const hid_cmd;

	u8 const rpt_id;
};

template <typename P>
struct report : hid_header, P
{
	report(/*const bool zero = false*/)
		: hid_header(P::HID::TYPE, P::HID::PARAM, P::RPT_ID)
	{
		//if (zero)
			//memset(((u8*)this) + sizeof(hid_header), 0, sizeof(P));
	}

	bool is_sane() const
	{
		return hid_header::hid_cmd.type == P::HID::TYPE
			&& hid_header::hid_cmd.param == P::HID::PARAM
			&& hid_header::rpt_id == P::RPT_ID;
	}
};

template <u8 T, u8 P, u8 I>
struct payload
{
	struct HID
	{
		static const u8 TYPE = T;
		static const u8 PARAM = P;
	};

	static const u8 RPT_ID = I;
};

template <u8 I>
struct input_report : payload<hid_type::data, hid_param::input, I> {};
template <u8 I>
struct output_report : payload<hid_type::set_report, hid_param::output, I> {};

namespace rpt
{
// 0x20: Status Information
struct status : input_report<0x20>
{
	core_button_t	core_button;

	u8	battery_low : 1;
	u8	extension : 1;
	u8	speaker : 1;
	u8	ir : 1;
	u8	leds : 4;

	u8 : 8;
		
	u8 : 8;

	u8	battery;
};

// 0x21: Read  Memory and Registers Data
struct read_data_reply : input_report<0x21>
{
	core_button_t	core_button;

	u8	error : 4;
private:
	u8	size : 4;

	u8	address[2];
public:
	u8	data[16];

	u16 get_size() const
	{
		return (u16)size + 1;
	}

	u16 get_address() const
	{
		return (address[0] << 0x08) | address[1];
	}

	// enum
	enum
	{
		ERR_CANNOT_READ = 7,
		ERR_BAD_ADDRESS = 8,
	};
};

// 0x22: Acknowledge output report, return function result
struct ack : input_report<0x22>
{
	core_button_t	core_button;

	u8	ack_id;

	u8	error;
};

// 0x30-0x3f: Data reports
template <u8 D, u8 S>
struct data : input_report<D>
{
	u8	_data[S];
};

// 0x13,0x14,0x19,0x1A: Enable/Disable features
template <u8 E>
struct enable : output_report<0x10 | E>
{
	u8	rumble : 1;
	u8 : 1;
	u8	_enable : 1;
	u8 : 5;
};

// 0x10: Rumble
struct rumble : output_report<0x10>
{
	u8	_rumble : 1;
	u8 : 7;
};

// 0x11: Player LEDs
struct leds : output_report<0x11>
{
	u8	rumble : 1;
	u8 : 3;
	u8	_leds : 4;
};

// 0x12: Data Reporting mode
struct report_mode : output_report<0x12>
{
	u8	rumble : 1;
	u8 : 1;
	u8	continuous : 1;
	u8 : 5;

	u8	mode;
};

// 0x13: IR Camera Enable Clock
struct camera_clock : enable<0x3> {};

// 0x14: Speaker Enable
struct speaker_enable : enable<0x4> {};

// 0x15: Status Information Request
struct status_request : output_report<0x15>
{
	u8	rumble : 1;
	u8 : 7;
};

// 0x16: Write Memory and Registers
struct write_data : output_report<0x16>
{
	u8	rumble : 1;
	u8 : 1;
	u8	space : 2;
	u8 : 4;
		
private:
	u8	address[3];
	u8	size;

public:
	u8	data[0x10];

	// func
	void set_address(u32 _address)
	{
		address[0] = u8(_address >> 0x10);
		address[1] = u8(_address >> 0x08);
		address[2] = u8(_address << 0x00);		
	}

	void set_size(u16 _size)
	{
		size = (u8)(_size - 1);
	}
};

// 0x17: Read Memory and Registers
struct read_data : output_report<0x17>
{
	u8	rumble : 1;
	u8 : 1;
	u8	space : 2;
	u8 : 4;
		
private:
	u8	address[3];
	u8	size[2];

public:
	void set_address(u32 _address)
	{
		address[0] = u8(_address >> 0x10);
		address[1] = u8(_address >> 0x08);
		address[2] = u8(_address << 0x00);
	}

	void set_size(u16 _size)
	{
		size[0] = u8(_size >> 0x08);
		size[1] = u8(_size >> 0x00);
	}
};

// 0x18: SpeakerData
struct speaker_data : output_report<0x18>
{
	u8	rumble : 1;
	u8 : 2;
	u8	size : 5;

	u8	data[20];
};

// 0x19: Speaker Mute
struct speaker_mute : enable<0x9> {};

// 0x1a: IR Camera Enable Logic
struct camera_enable : enable<0xA> {};
}

}	// namespace

#endif
