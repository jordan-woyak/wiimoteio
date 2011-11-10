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

#include <vector>

#include "common_types.hpp"

#pragma pack(push,1)

namespace wio
{

typedef u16 core_button_t;

//template <typename T>
//T convert_from_big_endian(const void* data, u8 size = sizeof(T))
//{
//	T n = 0;
//	// TODO:
//	return n;
//};
//
//template <typename T>
//T convert_to_big_endian(const void* data, u8 size = sizeof(T))
//{
//	T n = 0;
//	// TODO:
//	return n;
//};

//enum : u8
//{
//	led_1 = (1 << 0),
//	led_2 = (1 << 1),
//	led_3 = (1 << 2),
//	led_4 = (1 << 3),
//};

namespace hid_type
{
enum
{
	handshake =		0x0,
	set_report =	0x5,
	data =			0xa,
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

//struct hid_report
//{
//	template <typename R>
//	hid_report(R* rpt)
//		: data((u8*)rpt), size(sizeof(*rpt))
//	{}
//
//	const u8 *const data;
//	const u8 size;
//};

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
static_assert(2 == sizeof(hid_header), "bad size");

template <typename P>
struct report : hid_header, P
{
	report()
		: hid_header(P::payload::hid::type, P::payload::hid::param, P::payload::rpt_id)
	{}

	// TODO: move to report_cast, maybe
	bool is_sane() const
	{
		return hid_header::hid_cmd.type == P::payload::hid::type
			&& hid_header::hid_cmd.param == P::payload::hid::param
			&& hid_header::rpt_id == P::payload::rpt_id;
	}
};

template <typename P, typename C>
const report<P>* report_cast(C& _rpt)
{
	// TODO: use == ?
	if (_rpt.size() >= sizeof(report<P>))
	{
		auto ptr = reinterpret_cast<const report<P>*>(_rpt.data());
		if (ptr->is_sane())
			return ptr;
	}

	return nullptr;
}

// TODO: poor name
template <u8 T, u8 P, u8 I>
struct payload
{
	struct hid
	{
		static const u8 type = T;
		static const u8 param = P;
	};

	static const u8 rpt_id = I;
};

template <u8 I>
struct input_report
{
	typedef payload<hid_type::data, hid_param::input, I> payload;
};

template <u8 I>
struct output_report
{
	typedef payload<hid_type::set_report, hid_param::output, I> payload;
};

namespace rpt
{

// 0x20: Status Information
struct status : input_report<0x20>
{
	core_button_t core_button;

	u8 battery_low : 1;
	u8 extension : 1;
	u8 speaker : 1;
	u8 ir : 1;
	u8 leds : 4;

	u8 : 8;
		
	u8 : 8;

	u8 battery;
};
static_assert(6 == sizeof(status), "bad size");

// 0x21: Read  Memory and Registers Data
struct read_data_reply : input_report<0x21>
{
	//enum
	//{
	//	err_none = 0,
	//	err_cannot_read = 7,
	//	err_bad_address = 8,
	//};

	core_button_t core_button;

	u8 error : 4;
private:
	u8 size : 4;

	u8 address[2];
public:
	u8 data[16];

	u16 get_size() const
	{
		return (u16)size + 1;
	}

	u16 get_address() const
	{
		return (address[0] << 0x08) | address[1];
	}
};
static_assert(21 == sizeof(read_data_reply), "bad size");

// 0x22: Acknowledge output report, return function result
struct ack : input_report<0x22>
{
	core_button_t core_button;

	u8 ack_id;

	u8 error;
};
static_assert(4 == sizeof(ack), "bad size");

// 0x30 - 0x3f: Data reports
template <u8 D, u8 S>
struct input_data : input_report<D>
{
	u8 data[S];
};

// 0x13, 0x14, 0x19, 0x1a: Enable/Disable features
template <u8 E>
struct enable_feature : output_report<0x10 | E>
{
	u8 motor : 1;
	u8 request_ack : 1;
	u8 enable : 1;
	u8 : 5;
};

// 0x10: Rumble
struct rumble : output_report<0x10>
{
	u8 motor : 1;
	u8 : 7;
};
static_assert(1 == sizeof(rumble), "bad size");

// 0x11: Player LEDs
struct player_leds : output_report<0x11>
{
	u8 motor : 1;
	u8 : 3;
	u8 leds : 4;
};
static_assert(1 == sizeof(player_leds), "bad size");

// 0x12: Data Reporting mode
struct report_mode : output_report<0x12>
{
	u8 motor : 1;
	u8 : 1;
	u8 continuous : 1;
	u8 : 5;

	u8 mode;
};
static_assert(2 == sizeof(report_mode), "bad size");

// 0x13: IR Camera Enable Clock
struct camera_clock : enable_feature<0x3> {};
static_assert(1 == sizeof(camera_clock), "bad size");

// 0x14: Speaker Enable
struct speaker_enable : enable_feature<0x4> {};
static_assert(1 == sizeof(speaker_enable), "bad size");

// 0x15: Status Information Request
struct status_request : output_report<0x15>
{
	u8 motor : 1;
	u8 : 7;
};
static_assert(1 == sizeof(status_request), "bad size");

// 0x16: Write Memory and Registers
struct write_data : output_report<0x16>
{
	u8 motor : 1;
	u8 : 1;
	u8 space : 2;	// address space
	u8 : 4;
		
private:
	u8 address[3];
	u8 size;

public:
	u8 data[0x10];

	// func
	void set_address(u32 _address)
	{
		address[0] = u8(_address >> 0x10);
		address[1] = u8(_address >> 0x08);
		address[2] = u8(_address >> 0x00);		
	}

	void set_size(u8 _size)
	{
		size = _size;
	}
};
static_assert(21 == sizeof(write_data), "bad size");

// 0x17: Read Memory and Registers
struct read_data : output_report<0x17>
{
	u8 motor : 1;
	u8 : 1;
	u8 space : 2;	// address space
	u8 : 4;
		
private:
	u8 address[3];
	u8 size[2];

public:
	void set_address(u32 _address)
	{
		address[0] = u8(_address >> 0x10);
		address[1] = u8(_address >> 0x08);
		address[2] = u8(_address >> 0x00);
	}

	void set_size(u16 _size)
	{
		size[0] = u8(_size >> 0x08);
		size[1] = u8(_size >> 0x00);
	}
};
static_assert(6 == sizeof(read_data), "bad size");

// 0x18: SpeakerData
struct speaker_data : output_report<0x18>
{
	u8 motor : 1;
	u8 : 2;
	u8 size : 5;

	u8 data[20];
};
static_assert(21 == sizeof(speaker_data), "bad size");

// 0x19: Speaker Mute
struct speaker_mute : enable_feature<0x9> {};
static_assert(1 == sizeof(speaker_mute), "bad size");

// 0x1a: IR Camera Enable Logic
struct camera_enable : enable_feature<0xa> {};
static_assert(1 == sizeof(camera_enable), "bad size");

}	// namespace

}	// namespace

#pragma pack(pop)

