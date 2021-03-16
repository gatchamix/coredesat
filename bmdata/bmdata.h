#pragma once

#include "../audio/wav.h"
#include "../audio/vag.h"
#include "../fspak/directory.h"

#include "chart.h"
#include "keysound.h"

enum class directory_type
{
	charts,
	keysounds,
	unknown
};

auto get_directory_type(data_view data)
-> directory_type
{
	// read last 4 bytes of entry
	auto it = data.begin() + data.size() - 4;
	auto type = parseBE<uint32_t>(it);

	switch (type)
	{
	case 0x0000'7FFF:
		return directory_type::charts;

	case 0x7777'7777:
		return directory_type::keysounds;

	default:
		return directory_type::unknown;
	}
}

void process_bmdata_file()
{
	auto const data = load_file("BMDATA.PAK;1");
	if (data.empty())
		return;

	auto const raw_data = strip_iso_headers(data);

	auto const base_dir = fs::current_path();
	auto const bmdata_dir = base_dir / "bmdata";
	fs::create_directory(bmdata_dir);

	auto const directory = parse_fspak_file(raw_data);
	for (size_t index = 0; auto const& entry : directory.entries)
	{
		auto const type = get_directory_type(entry);
		switch (type)
		{
		case directory_type::keysounds:
		{
			auto const dir_path = bmdata_dir / (std::stringstream{} << index).str();
			fs::create_directory(dir_path);

			auto const keysounds = extract_bmdata_keysounds(entry);
			for (auto keysound_index = 0; auto const& keysound : keysounds)
			{
				auto const pcm = vag_to_pcm(keysound.data);
				auto const keysound_path = dir_path / (std::stringstream{} << keysound_index << ".wav").str();
				save_wav_file(keysound_path, pcm);
				++keysound_index;
			}
			break;
		}

		case directory_type::charts:
		{
			auto const chart = extract_bmdata_chart(entry);
			break;
		}

		default:
		{
			auto const filename = bmdata_dir / (std::stringstream{} << index).str();
			auto out_file = std::ofstream{ filename, std::ios_base::binary };
			out_file.write(reinterpret_cast<char const*>(&entry[0]), entry.size());
			break;
		}
		}

		++index;
	}
}