#pragma once

#include "..\pch.h"
#include "..\util\io.h"

struct bmdata_keysound
{
	std::vector<unsigned char> data;
};

auto extract_bmdata_keysounds(data_view data)
-> std::vector<bmdata_keysound>
{
	auto it = data.begin();

	it += 4; // skip padding / unknown data
	auto const count = parseLE<uint32_t>(it) / 0x10;
	it += 8; // skip padding / unknown data

	auto offsets = std::vector<uint32_t>{};
	offsets.reserve(static_cast<size_t>(count) + 1);
	for (size_t i = 0; i < count; ++i)
	{
		offsets.emplace_back(parseBE<uint32_t>(it));
		it += 12; // skip padding / unknown data
	}

	auto const data_offset = parseLE<uint32_t>(it);
	auto const data_length = parseLE<uint32_t>(it);

	auto const data_start = it + 8;
	offsets.push_back(data_length + 0x1000);

	auto keysounds = std::vector<bmdata_keysound>{};
	keysounds.reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		auto const length = offsets[i + 1] - offsets[i];

		auto const beg = data_start + (offsets[i] - data_offset);
		auto const end = beg + length;

		keysounds.emplace_back(std::vector<unsigned char>(beg + 0x10, end));
	}

	return keysounds;
}