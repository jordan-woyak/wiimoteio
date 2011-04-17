
#ifndef WMLIB_DATA_REPORT_H_
#define WMLIB_DATA_REPORT_H_

#include <map>

#include "common_types.h"

namespace wio
{

namespace rpt
{
enum
{
	/*rpt_*/ none =		0,

	/*rpt_*/ button =	(1 << 0),
	/*rpt_*/ accel =	(1 << 1),
	/*rpt_*/ ext =		(1 << 2),
	/*rpt_*/ ir =		(1 << 3),
	/*rpt_*/ gyro =		(1 << 4),

	/*rpt_*/ all =		button | accel | ext | ir | gyro,
};
}

namespace rptmode
{
enum
{
	/*rptmode_*/ button =		0x30,
	/*rptmode_*/ button_accel,
	/*rptmode_*/ button_ext8,
	/*rptmode_*/ button_accel_ir12,
	/*rptmode_*/ button_ext19,
	/*rptmode_*/ button_accel_ext16,
	/*rptmode_*/ button_ir10_ext9,
	/*rptmode_*/ button_accel_ir10_ext9 = 0x3d,
	/*rptmode_*/ ext21,
	/*rptmode_*/ button_accel_ir36,
};
}

// TODO: remove
using namespace rpt;

const std::pair<u8, u8> rpt_features[] =
{
	std::make_pair(button,				0x30),
	std::make_pair(button | accel,		0x31),
	std::make_pair(button | ext,		0x32),
	std::make_pair(button | gyro,		0x32),
	std::make_pair(button | ir,			0x32),


	std::make_pair(0x30, button),
	std::make_pair(0x31, button | accel),
	std::make_pair(0x32, button | ext),
	std::make_pair(0x33, button | accel | ir),
	std::make_pair(0x34, button | ext),
	std::make_pair(0x35, button | accel | ext),
	std::make_pair(0x36, button | ir | ext),
	std::make_pair(0x37, button | accel | ir | ext),
	std::make_pair(0x3d, ext),
	std::make_pair(0x3e, button | accel | ir | gyro),
	std::make_pair(0x3f, button | accel | ir | gyro),
};

std::map<u8, u8> rpt_features_map(rpt_features, rpt_features + sizeof(rpt_features));

}	// namespace

#endif
