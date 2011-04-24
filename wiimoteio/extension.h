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

#ifndef WMLIB_EXTENSION_H_
#define WMLIB_EXTENSION_H_

#include "common_types.h"

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

}	// namespace

#endif
