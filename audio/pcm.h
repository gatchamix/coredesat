#pragma once

#include "../pch.h"

struct pcm_audio
{
	size_t sample_frequency;
	size_t bits_per_sample;
	size_t channels;
	std::vector<uint16_t> data;
};