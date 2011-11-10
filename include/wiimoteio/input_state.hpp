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

namespace wio
{

namespace button
{
enum : u16
{
	left =	(1 << 0x0),
	right =	(1 << 0x1),
	down =	(1 << 0x2),
	up =	(1 << 0x3),
	plus =	(1 << 0x4),

	two =	(1 << 0x8),
	one =	(1 << 0x9),
	b =		(1 << 0xa),
	a =		(1 << 0xb),
	minus =	(1 << 0xc),

	home =	(1 << 0xf),
};
}

class input_state
{
public:
	void get_features() const
	{

	}

	class
	{

		
	} button;

	class
	{

		
	} accel;

	class
	{


	} gyro;
};

}

