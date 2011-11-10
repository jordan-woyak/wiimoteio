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

#include "common_types.h"
#include "util.h"

namespace wio
{

// high byte:	0xaX00fa
// low byte:	0xaX00ff
typedef u16 extid_t;

namespace extid
{
enum : extid_t
{
	/*extid_*/ nunchuk = 0x0000,
	/*extid_*/ classic_controller = 0x0001,
	/*extid_*/ balance_board = 0x0002,
	/*extid_*/ guitar = 0x0003,
	/*extid_*/ motion_plus = 0x0005,
	/*extid_*/ drums = 0x0103,
	/*extid_*/ turntable = 0x0303,

	/*extid_*/ nothing = 0x002e,
	/*extid_*/ partial = 0x00ff,
};
}

class extension
{

};

inline void undo_mp_passthrough(u8* _data, u8 _mode)
{
	if (0x05 == _mode)
	{
		// nunchuk passthrough
		set_bit(_data[5], 0, get_bit(_data[5], 2)); set_bit(_data[5], 2, false);
		set_bit(_data[5], 1, get_bit(_data[5], 3));
		set_bit(_data[5], 3, get_bit(_data[5], 4)); set_bit(_data[5], 4, false);
		set_bit(_data[4], 0, get_bit(_data[5], 7));
		set_bit(_data[5], 7, get_bit(_data[5], 6)); set_bit(_data[5], 6, false);
	}
	else if (0x07 == _mode)
	{
		// other passthrough
		set_bit(_data[5], 0, get_bit(_data[0], 0)); set_bit(_data[0], 0, false);
		set_bit(_data[5], 1, get_bit(_data[1], 0)); set_bit(_data[1], 0, false);
		set_bit(_data[4], 0, false);
	}
}

namespace ext
{

namespace nunchuk
{
struct datafmt
{
	u8 stick_x;

	u8 stick_y;

	u8 accel_x_high;

	u8 accel_y_high;

	u8 accel_z_high;

	u8 button_z : 1;
	u8 button_c : 1;
	u8 accel_x_low : 2;
	u8 accel_y_low : 2;
	u8 accel_z_low : 2;
};
static_assert(6 == sizeof(datafmt), "bad size");
}

namespace classic_controller
{
struct datafmt
{
	u8 stick_lx : 6;
	u8 stick_rx_high : 2;

	u8 stick_ly : 6;
	u8 stick_rx_mid : 2;

	u8 stick_ry_low : 5;
	u8 trigger_l_high : 2;
	u8 stick_rx_low : 1;

	u8 trigger_r : 5;
	u8 trigger_l_low : 3;

	u8 : 1;
	u8 button_rt : 1;
	u8 button_plus : 1;
	u8 button_home : 1;
	u8 button_minus : 1;
	u8 button_lt : 1;
	u8 button_pad_down : 1;
	u8 button_pad_right : 1;

	u8 button_pad_up : 1;
	u8 button_pad_left : 1;
	u8 button_zr : 1;
	u8 button_x : 1;
	u8 button_a : 1;
	u8 button_y : 1;
	u8 button_b : 1;
	u8 button_zl : 1;
};
static_assert(6 == sizeof(datafmt), "bad size");
}

namespace motion_plus
{
struct datafmt
{
	u8 yaw_low;

	u8 roll_low;

	u8 pitch_low;

	u8 pitch_slow: 1;
	u8 yaw_slow: 1;
	u8 yaw_high : 6;

	u8 is_extension_attached : 1;
	u8 roll_slow : 1;
	u8 roll_high : 6;

	u8 : 1;
	u8 is_motion_plus_data : 1;
	u8 pitch_high : 6;
};
static_assert(6 == sizeof(datafmt), "bad size");
}

}

}	// namespace

