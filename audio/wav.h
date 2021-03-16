#pragma once

#include "..\pch.h"
#include "..\util\io.h"

#include "pcm.h"

void save_wav_file(fs::path const& file_path, pcm_audio const& pcm)
{
	auto const sample_frequency = static_cast<uint32_t>(pcm.sample_frequency);
	auto const bits_per_sample = static_cast<uint16_t>(pcm.bits_per_sample);
	auto const channels = static_cast<uint16_t>(pcm.channels);

	auto const size = static_cast<uint32_t>(pcm.data.size() * 2 + 44);
	auto const byte_rate = static_cast<uint32_t>(sample_frequency * channels * bits_per_sample / 8);
	auto const align = static_cast<uint16_t>(channels * bits_per_sample / 8);

	/* write WAVE format *\
	* "RIFF"		(32b) *
	* filesize		(32b) *
	* "WAVE"		(32b) *
	* "fmt "		(32b) *
	* 0x10			(32b) *
	* 0x01			(16b) *
	* channels		(32b) *
	* sample_freq	(32b) *
	* byte_rate		(32b) *
	* align			(16b) *
	* bits_per		(16b) *
	* "data"		(32b) *
	* chunk size	(32b) *
	* [data]		(??b) *
	\* * * * * * * * * * */

	auto out = std::ofstream{ file_path, std::ios_base::binary };

	out.write("RIFF", 4);
	writeBE(out, size - 8);
	out.write("WAVE", 4);
	out.write("fmt ", 4);

	out.put(0x10);
	out.put(0x00);
	out.put(0x00);
	out.put(0x00);

	out.put(0x01);
	out.put(0x00);

	writeBE(out, channels);
	writeBE(out, sample_frequency);
	writeBE(out, byte_rate);
	writeBE(out, align);
	writeBE(out, bits_per_sample);

	out.write("data", 4);
	writeBE(out, size - 44);
	out.write(reinterpret_cast<char const*>(&pcm.data[0]), size - 44);
}