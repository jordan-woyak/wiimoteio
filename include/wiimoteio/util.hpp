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

namespace wio
{

template <typename I>
class calibrated_int
{
public:
	calibrated_int()
		: val(I())
		, zero(I())
		, pos(I())
		, neg(I())
	{}

	calibrated_int(I _zero, I _pos, I _neg)
		: val(I())
		, zero(_zero)
		, pos(_pos)
		, neg(_neg)
	{}

	float value() const
	{
		return (float)(val - zero) / (pos - zero);
	}

	I val;
	I zero, pos, neg;
};

template <typename V>
void set_bit(V& _val, size_t _bit_index, bool _bit)
{
	if (_bit)
		_val |= (1 << _bit_index);
	else
		_val &= ~(1 << _bit_index);
}

template <typename V, typename V2>
void set_bits(V& _val, size_t _bit_index, size_t _count, V2 _bit)
{
	//TODO:
}

template <typename V>
bool get_bit(const V& _val, size_t _bit_index)
{
	return (_val & (1 << _bit_index)) != 0;
}

template <typename V>
V get_bits(const V& _val, size_t _bit_index, size_t _count)
{
	// TODO:
	return 0;
}

}	// namespace

