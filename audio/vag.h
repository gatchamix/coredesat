#pragma once

#include "..\pch.h"

#include "coefficients.h"
#include "pcm.h"

auto vag_to_pcm(data_view data)
-> pcm_audio
{
	auto pcm = pcm_audio
	{
		.sample_frequency = 37'800,
		.bits_per_sample = 16,
		.channels = 1
	};

	auto s1 = 0.0;
	auto s2 = 0.0;
	auto samples = std::array<double, 28>{};
	for (auto it = data.begin(); it < (data.end() - 16);)
	{
		uint8_t const val = *it++;
		uint8_t const shift_value = (val >> 0) & 0x0F;
		uint8_t const coefficient = (val >> 4) & 0x0F;

		if (*it++ == 7)
			break;

		for (size_t i = 0; i < 28;)
		{
			auto const nibble = *it++;
			int16_t const lo = (nibble & 0x0F) << 12;
			int16_t const hi = (nibble & 0xF0) << 8;

			samples[i++] = static_cast<double>(lo >> shift_value);
			samples[i++] = static_cast<double>(hi >> shift_value);
		}

		for (size_t i = 0; i < 28; ++i)
		{
			auto s0 = samples[i];
			s0 += s1 * ADPCM_GAIN[coefficient][0]
				+ s2 * ADPCM_GAIN[coefficient][1];

			auto const min = std::numeric_limits<uint16_t>::min();
			auto const max = std::numeric_limits<uint16_t>::max();
			if (std::isinf(s0) || std::isnan(s0))
				if (s0 > 0)
					s0 = static_cast<double>(max);
				else
					s0 = static_cast<double>(min);

			s2 = s1;
			s1 = s0;

			auto const d = static_cast<uint16_t>(s0 + 0.5);
			pcm.data.push_back(d);
		}
	}

	return pcm;
}