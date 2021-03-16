#pragma once

#include "../pch.h"

#include "coefficients.h"
#include "pcm.h"

auto xa_to_pcm(data_view data)
-> pcm_audio
{
	auto pcm = pcm_audio
	{
		.sample_frequency = 37'800,
		.bits_per_sample = 16,
		.channels = 2
	};

	auto history = std::array<std::pair<double, double>, 2>{};
	for (auto it = data.begin(); it != data.end(); it += 0x80)
	{
		auto const frame = data_view{ it, it + 0x80 };
		auto samples = std::array<uint16_t, 224>{};
		for (size_t i = 0; i < 8; ++i)
		{
			auto const channel = i & 1;
			auto& [s1, s2] = history[channel];

			uint8_t const val = frame[i + 4];
			uint8_t const shift_value = (val >> 0) & 0x0F;
			uint8_t const coefficient = (val >> 4) & 0x0F;

			for (size_t j = 0; j < 28; ++j)
			{
				auto const index = 0x10 + (i >> 1) + (j << 2);
				auto const shift = channel << 2;
				auto const nibble = (frame[index] >> shift) & 0xF;

				int16_t s = (nibble << 12) & 0xF000;

				auto s0 = static_cast<double>(s >> shift_value);
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

				auto const sample_index = channel + (j << 1) + (56 * (i >> 1));
				samples[sample_index] = static_cast<uint16_t>(s0 + 0.5);
			}
		}

		pcm.data.insert(pcm.data.end(), samples.begin(), samples.end());
	}

	return pcm;
}