
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

int lame()
{
	report<rpt::ack> test1;
	//HidReport<orpt::Status> test2;

	report<rpt::camera_clock> test2;

	std::cout << std::hex << (int)rpt::camera_clock::RPT_ID;

	hid_report rpt(new report<rpt::camera_clock>);

	std::cout << "test1 hid_cmd: " << std::hex << (int)test1.ack_id << std::endl;
	std::cout << "test2 hid_cmd: " << (int)test2._enable << std::endl;

	std::cout << "test: " << (int)test2._enable << std::endl;

	extid_t nothin = extid::nothing;

	//std::cout << extid::nothing;

	//std::cout << "sizeof orpt::WriteData: " << sizeof(testrpt) << std::endl;


	std::cin.get();
	return 0;
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
