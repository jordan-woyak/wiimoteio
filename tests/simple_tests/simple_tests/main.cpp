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

#include "wiimoteio/extension.h"
#include "wiimoteio/report.h"
#include "wiimoteio/wiimote.h"

#include <iostream>
#include <iomanip>
#include <functional>
#include <thread>

// TODO: remove
using namespace wio;

template <typename C, typename F>
void ForEach(C&& c, F&& f)
{
	std::for_each(c.begin(), c.end(), std::forward<F>(f));
}

int main()
{
	auto found_wiimotes = find_Wiimotes(4);
	std::cout << "found " << found_wiimotes.size() << " wiimotes\n";
	
	if (!found_wiimotes.empty())
	{
		int leds;
		while (std::cin >> leds && leds >= 0)
		{
			//ForEach(found_wiimotes, std::bind2nd(std::mem_fun_ref(&wio::wiimote::set_leds), leds));
			//ForEach(found_wiimotes, std::bind2nd(std::mem_fun_ref(&wio::wiimote::set_rumble), leds & 1));

			ForEach(found_wiimotes, [leds](wio::wiimote& wm)
			{
				wm.set_leds(leds);
				wm.set_rumble(leds & 1);
			});
		}

		ForEach(found_wiimotes, [](wio::wiimote& wm)
		{
			//auto result = wm.basic().read_register(0xa400fa, 6);
			auto result = wm.basic().read_eeprom(0x0000, 42);

			ForEach(result.get(), [](int x) { std::cout << std::setfill('0') << std::setw(2) << std::hex << x; });
			std::cout << '\n';
		});

		//ForEach(found_wiimotes, std::mem_fun_ref(&wio::wiimote::disconnect));
		std::cout << "disconnected\n";
	}

	std::cin.get();
}
