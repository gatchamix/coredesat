#pragma once

#include "../pch.h"
#include "../audio/wav.h"
#include "../audio/xa.h"

void process_mchdata_file()
{
	auto const data = load_file("MCHDATA.PAK;1");
	if (data.empty())
		return;

	auto const base_dir = fs::current_path();
	auto const mchdata_dir = base_dir / "mchdata";
	fs::create_directory(mchdata_dir);

	auto tracks = std::unordered_map<size_t, data_file>{};
	for (auto it = data.begin(); it != data.end(); it += 0x918)
	{
		it += 0x10; // skip sync + header
		it += 0x01;	// skip number
		auto const index = (*it++) & 0xF;
		auto const header = *it++;
		it += 0x01;	// skip format
		it += 0x04; // skip subheader copy

		if (header & 0b0000'0100)
		{
			auto& track = tracks[index];
			track.insert(track.end(), it, it + 0x900);
		}
	}

	for (auto const& [id, track] : tracks)
	{
		auto const pcm = xa_to_pcm(track);
		auto const save_path = mchdata_dir / (std::stringstream{} << "track " << id << ".wav").str();
		save_wav_file(save_path, pcm);
	}
}