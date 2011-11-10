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

#include "../wiimote.hpp"

namespace wio
{

template <typename S>
void wiimote::speaker_stream(S&& _strm)
{
	auto stream = std::make_shared<typename std::remove_all_extents<S>::type>(std::forward<S>(_strm));

	// speaker initialization

	// disable+enable speaker
	{
	report<rpt::speaker_enable> enb;
	enb.request_ack = true;
	enb.enable = true;
	send_report(enb);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(50));	// hax

	// mute speaker
	{
	report<rpt::speaker_mute> mut;
	mut.request_ack = true;
	mut.enable = true;
	send_report(mut);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(50));	// hax

	// some nonsense
	std::vector<u8> data(1);
	data[0] = 0x01;
	write_register(0xa20009, data).wait();
	data[0] = 0x80;
	write_register(0xa20001, data).wait();	// this seems to supress all sound

	// speaker configuration
	{
	std::vector<u8> conf(7);
		
	// dunno
	conf[0] = 0x00;	// seems to re-enable sound

	// format
	conf[1] = m_state.speaker_fmt;
	bool const using_pcm = !!conf[1];

	// sample rate
	u16 const rate = (using_pcm ? 12000000 : 12000000) / m_state.speaker_rate;
	conf[2] = u8(rate >> 0x0);
	conf[3] = u8(rate >> 0x8);

	// volume
	conf[4] = u8((using_pcm ? 0xff : 0x7f) * m_state.speaker_vol);
		
	// decoder state
	conf[5] = 0x0c;
	conf[6] = 0x0e;

	write_register(0xa20001, conf).wait();
	}

	// unmute speaker
	{
	report<rpt::speaker_mute> mut;
	mut.request_ack = true;
	mut.enable = false;
	send_report(mut);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(50));	// hax

	// more nonsense
	data[0] = 0x01;
	write_register(0xa20008, data).wait();

	// commence the streamin
	speaker_sample_number = 0;
	// TODO: hax
	speaker_start_time = worker_thread::clock::now();
	m_worker.schedule_job_at(std::bind(&wiimote::speaker_stream_some<decltype(stream)>, this, stream),
		speaker_start_time, job_type_speaker);
}

template <typename S>
void wiimote::speaker_stream_some(S stream)
{
	// read up to 20 more bytes
	report<rpt::speaker_data> speaker_report;

	stream->read(reinterpret_cast<char*>(speaker_report.data), 20);
	speaker_report.size = stream->gcount();

	if (speaker_report.size)
	{
		send_report(speaker_report);
		speaker_sample_number += speaker_report.size * (m_state.speaker_fmt == format_adpcm ? 2 : 1);

#if 0	// absolute time
			
		// TODO: make better
		auto packet_time = speaker_start_time +
			std::chrono::milliseconds(std::chrono::seconds(speaker_sample_number)) / (int64_t)m_state.speaker_rate;

		// testing hax
		//packet_time += std::chrono::milliseconds(1);

		m_worker.schedule_job_at(std::bind(&wiimote::speaker_stream_some<S>, this, stream),
			packet_time,
			job_type_speaker);

#else	// relative time

		auto delay =
			std::chrono::milliseconds(std::chrono::seconds(speaker_sample_number)) / (int64_t)m_state.speaker_rate;

		// testing hax
		delay += std::chrono::milliseconds(3);

		m_worker.schedule_job_in(std::bind(&wiimote::speaker_stream_some<S>, this, stream),
			delay,
			job_type_speaker);

		speaker_sample_number = 0;
#endif
	}
}

}

