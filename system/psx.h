#pragma once

#include "../pch.h"
#include "../util/io.h"

auto find_system_binary()
-> std::optional<std::string>
{
	auto const data = load_file("SYSTEM.CNF;1");
	if (data.empty())
		return std::nullopt;

	auto it = data.begin();
	it += 0x26; // jump to boot file name

	return std::string{ it, it + 13 }; // always in the form abcd_123.45;1
}

void process_system_file()
{
	auto const bootfile_path = find_system_binary();
	if (!bootfile_path.has_value())
		return;

	auto const data = load_file(bootfile_path.value());
	if (data.empty())
		return;
}