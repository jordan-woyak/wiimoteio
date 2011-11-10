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

#include "wiimoteio/wiimoteio.hpp"

#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
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

		std::cout << "led: " << (int)wm.get_leds() << '\n';
		std::cout << "bat: " << wm.get_battery_level() << '\n';
		std::cout << "ext: " << (int)wm.get_ext_id() << '\n';

		//wm.set_speaker_format(wio::wiimote::format_adpcm);
		//wm.set_speaker_volume(0.25f);
		//wm.set_speaker_rate(6000);
		//
		//wm.speaker_stream(std::ifstream("rmtdump.bin", std::ios::binary));
		//std::cout << "streamin, yo\n";

		wm.set_report_button(true);
		wm.set_report_accel(true);
		wm.set_report_ext(true);
		wm.set_report_gyro(true);

		std::vector<wio::u8> data(1);
		data[0] = 0x55;
		wm.write_register(0xa400f0, data).get();

		std::cout << "get_present_gyro: " << wm.get_present_gyro() << '\n';

		// testing
		data[0] = 0x55;
		wm.write_register(0xa600f0, data);
		data[0] = 0x05;
		wm.write_register(0xa600fe, data);

		std::cout << "\n" "press <enter> to start reading input" "\n";
		std::cin.get();
		std::cout << "\n" "press <enter> to stop" "\n";

		std::promise<int> thrd_ender;

		std::thread thrd([&wm, &thrd_ender]
		{
			auto fut = thrd_ender.get_future();

			//while (fut.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout)
			while (true)
			{
				auto& button = wm.get_input_button();
				auto& accel = wm.get_input_accel();
				auto& gyro = wm.get_input_gyro();

				std::cout << '\r';

				std::cout << "Button: "
					<< std::fixed << std::hex << std::setw(4) << std::setfill('0') << (int)button << ' ';

				//std::cout << "Accel: "
				//	<< "X: " << std::fixed << accel[0].value() << ' '
				//	<< "Y: " << accel[1].value() << ' '
				//	<< "Z: " << accel[2].value() << ' ';

				//std::cout << "Gyro: "
				//	<< "Yaw: " << std::fixed << gyro[0].value() << ' '
				//	<< "Roll: " << gyro[1].value() << ' '
				//	<< "Pitch: " << gyro[2].value() << ' ';
			}
		});

		std::cin.get();

		thrd_ender.set_value(0);
		thrd.join();
	}

	std::cout << "\n" "press <enter> to quit" "\n";
	std::cin.get();
}
