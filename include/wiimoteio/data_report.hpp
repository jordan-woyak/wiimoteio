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

#include "report.hpp"

namespace wio
{

#pragma pack(push,1)

namespace rpt
{
namespace data
{

struct button_base
{
	u8 button[2];
};

// 0x30: Button
struct button : input_report<0x30>, button_base {};
static_assert(2 == sizeof(button), "bad size");

struct accel_base
{
	struct
	{
		u8 x, y, z;
	} accel;
};

// 0x31: Button, Accel
struct button_accel : input_report<0x31>, button_base, accel_base {};
static_assert(5 == sizeof(button_accel), "bad size");

// 0x32: Button, Ext8
struct button_ext8 : input_report<0x32>, button_base
{
	u8 ext[8];
};
static_assert(10 == sizeof(button_ext8), "bad size");

// 0x33: Button, Accel, Ir12
struct button_accel_ir12 : input_report<0x33>, button_base, accel_base
{
	u8 ir[12];	// TODO:
};
static_assert(17 == sizeof(button_accel_ir12), "bad size");

// 0x34: Button, Ext19
struct button_ext19 : input_report<0x34>, button_base
{
	u8 ext[19];
};
static_assert(21 == sizeof(button_ext19), "bad size");

// 0x35: Button, Accel, Ext16
struct button_accel_ext16 : input_report<0x35>, button_base, accel_base
{
	u8 ext[16];
};
static_assert(21 == sizeof(button_accel_ext16), "bad size");

// 0x36: Button, Ir10, Ext9
struct button_ir10_ext9 : input_report<0x36>, button_base
{
	u8 ir[10];	// TODO:
	u8 ext[9];
};
static_assert(21 == sizeof(button_ir10_ext9), "bad size");

// 0x37: Button, Accel, Ir10, Ext6
struct button_accel_ir10_ext6 : input_report<0x37>, button_base, accel_base
{
	u8 ir[10];	// TODO:
	u8 ext[6];
};
static_assert(21 == sizeof(button_accel_ir10_ext6), "bad size");

// 0x3d: Ext21
struct ext21 : input_report<0x3d>
{
	u8 ext[21];
};
static_assert(21 == sizeof(ext21), "bad size");

}	// namespace
}	// namespace

#pragma pack(pop)

}	// namespace

