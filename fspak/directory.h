#pragma once

#include "../pch.h"
#include "../util/io.h"

struct fspak_directory
{
	std::vector<data_file> entries;
};

auto parse_fspak_file(data_view in)
-> fspak_directory
{
	auto it = in.begin();

	auto const directory_size = parseBE<uint32_t>(it);
	auto const directory_count = parseBE<uint32_t>(it);

	auto directory = fspak_directory{};
	directory.entries.reserve(directory_count);
	for (size_t i = 0; i < directory_count; ++i)
	{
		auto const offset = parseBE<uint32_t>(it);
		auto const length = parseBE<uint32_t>(it);

		auto const beg = in.begin() + (static_cast<size_t>(offset) * 0x800);
		auto const end = beg + length;

		directory.entries.emplace_back(beg, end);
	}

	return directory;
}