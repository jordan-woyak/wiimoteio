
#ifndef WMLIB_EXTENSION_H_
#define WMLIB_EXTENSION_H_

#include "common_types.h"

namespace wio
{

// high byte:	0xAX00FA
// low byte:	0xAX00FF
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

}	// namespace

#endif
