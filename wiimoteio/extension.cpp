
#include "extension.h"

namespace wio
{

namespace extension_foo
{

// NUNCHUK
namespace nunchuk
{
struct datafmt
{
	u8	stick_x;

	u8	stick_y;

	u8	accel_x_high;

	u8	accel_y_high;

	u8	accel_z_high;

	u8	button_z : 1;
	u8	button_c : 1;
	u8	accel_x_low : 2;
	u8	accel_y_low : 2;
	u8	accel_z_low : 2;
};
}

// CLASSIC_CONTROLLER
namespace classic_controller
{
struct datafmt
{
	u8	stick_lx : 6;
	u8	stick_rx_high : 2;

	u8	stick_ly : 6;
	u8	stick_rx_mid : 2;

	u8	stick_ry_low : 5;
	u8	trigger_l_high : 2;
	u8	stick_rx_low : 1;

	u8	trigger_r : 5;
	u8	trigger_l_low : 3;

	u8 : 1;
	u8	button_rt : 1;
	u8	button_plus : 1;
	u8	button_home : 1;
	u8	button_minus : 1;
	u8	button_lt : 1;
	u8	button_pad_down : 1;
	u8	button_pad_right : 1;

	u8	button_pad_up : 1;
	u8	button_pad_left : 1;
	u8	button_zr : 1;
	u8	button_x : 1;
	u8	button_a : 1;
	u8	button_y : 1;
	u8	button_b : 1;
	u8	button_zl : 1;
};
}

// MOTION_PLUS
namespace motion_plus
{
struct datafmt
{
	u8	yaw_low;

	u8	roll_low;

	u8	pitch_low;

	u8	pitch_slow: 1;
	u8	yaw_slow: 1;
	u8	yaw_high : 6;

	u8	extension_attached : 1;
	u8	roll_slow : 1;
	u8	roll_high : 6;

	u8 : 2;
	u8	pitch_high : 6;
};
}

}

}	// namespace
