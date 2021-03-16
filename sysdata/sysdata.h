#pragma once

#include "../pch.h"
#include "../fspak/directory.h"

auto load_sysdata_file()
-> data_file
{
	constexpr auto filenames =
	{
		"SYSDATA.PAK;1",	// bm, 3rdmini, 3rd, 4th, 5th, got, got2, club, eu
		"SYS6TH.PAK;1",		// 6th+cr
		"SYSBST.PAK;1",		// best hits
		"SYSDCT.PAK;1",		// dreams come true
		"SYSREADY.PAK;1"	// the sound of tokyo
	};

	for (auto const& filename : filenames)
	{
		if (!fs::exists(filename))
			continue;

		auto const data = load_file(filename);
		if (data.empty())
			break;

		return data;
	}

	return {};
}

void process_sysdata_file()
{
	auto const data = load_sysdata_file();

	auto const raw_data = strip_iso_headers(data);

	auto const base_dir = fs::current_path();
	auto const sysdata_dir = base_dir / "sysdata";
	fs::create_directory(sysdata_dir);

	auto const directory = parse_fspak_file(raw_data);
	for (size_t index = 0; auto const& entry : directory.entries)
	{
		auto const filename = sysdata_dir / (std::stringstream{} << index).str();
		auto out_file = std::ofstream{ filename, std::ios_base::binary };
		out_file.write(reinterpret_cast<char const*>(&entry[0]), entry.size());
		++index;
	}
}