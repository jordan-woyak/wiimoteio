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
#include <fstream>

template <typename C, typename F>
void ForEach(C&& c, F&& f)
{
	std::for_each(c.begin(), c.end(), std::forward<F>(f));
}

template <typename C, typename F>
void ForEachDeref(C&& c, F&& f)
{
	auto iter = c.begin(), iterend = c.end();
	for (; iter != iterend; ++iter)
		f(**iter);
}

int main()
{
	auto found_wiimotes = wio::find_Wiimotes(4);
	std::cout << "found " << found_wiimotes.size() << " wiimotes\n";
	
	if (!found_wiimotes.empty())
	{
		auto& wm = *found_wiimotes.front();
		
		for (wio::u8 leds = wio::leds::player(0); leds != wio::leds::player(4); leds <<= 1)
		{
			wm.set_leds(leds);
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
		wm.set_leds(wio::leds::player(0));

		//wm.set_features(wio::wiimote::feat_button);

		std::cout << "led: " << (int)wm.get_leds() << '\n';
		std::cout << "bat: " << wm.get_battery_level() << '\n';
		std::cout << "ext: " << (int)wm.get_extension_id() << '\n';

		wm.set_speaker_format(wio::wiimote::format_adpcm);
		wm.set_speaker_volume(0.25f);
		wm.set_speaker_frequency(3000);
		
		wm.speaker_stream(std::ifstream("rmtdump.bin", std::ios::binary));

		//while (true)
		//{
		//	std::cout << "speaker reg: ";

		//	ForEach(wm.read_register(0xa20000, 0xa).get(), [](int b)
		//	{
		//		std::cout << std::hex << b << ' ';
		//	});

		//	std::cout << '\n';
		//	std::this_thread::sleep_for(std::chrono::milliseconds(250));
		//}
			

		std::cout << "streamin, yo\n";
	}

	std::cout << "\n" "press <enter> to quit" "\n";
	std::cin.get();
}
