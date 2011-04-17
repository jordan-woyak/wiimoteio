
#include "wiimote.h"

namespace wio
{

std::vector<wiimote> find_Wiimotes(size_t max_wiimotes)
{
	auto devs = find_devices(max_wiimotes);

	std::vector<wiimote> wiimotes;
	wiimotes.reserve(devs.size());
	
	std::for_each(devs.begin(), devs.end(), [&wiimotes](device& dev)
	{
		//u8 foo[22];
		//if (dev.read(foo, 22) != -1)	// TODO: ugly
			wiimotes.push_back(wiimote(basic_wiimote(std::move(dev))));
	});

	return wiimotes;
}

}	// namespace
