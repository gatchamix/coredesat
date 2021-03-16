#pragma once

#include "../pch.h"

#include "io.h"

constexpr auto SECTOR_SIZE = 0x930;

enum class record_type
{
	file		= 0x0,
	directory	= 0x2
};

struct directory_record
{
	uint8_t size;
	uint32_t sector;
	uint32_t data_size;
	record_type type;
	std::string name;
};

auto read_directory_record(data_view data)
-> directory_record
{
	auto it = data.begin();

	auto record = directory_record{};

	record.size = *it++;
	it += 1; // skip extended attribute record length
	record.sector = parseBE<uint32_t>(it);
	it += 4; // skip LE sector
	record.data_size = parseBE<uint32_t>(it);
	it += 4; // skip LE sector count
	it += 7; // skip timestamp
	record.type = static_cast<record_type>(*it++);
	it += 1; // skip file unit size
	it += 1; // skip interleave gap size
	it += 4; // skip volume sequence number
	auto const name_length = *it++;
	record.name = std::string(it, it + name_length);

	return record;
}

auto is_psx_iso(data_view data)
-> bool
{
	auto it = data.begin();
	it += 0xF;	// skip sync + (most) of header
	auto const mode = *it++;

	return (mode == 0x2);
}

auto strip_iso_headers(data_view data)
-> data_file
{
	auto raw_data = data_file{};
	for (auto it = data.begin(); it != data.end();)
	{
		it += 0x0F; // skip sync + (most) of header
		auto const mode = *it++;

		switch (mode)
		{
		case 0x1: // CDROM
		{
			raw_data.insert(raw_data.end(), it, it + 0x800);
			it += 0x920;
			break;
		}

		case 0x2: // CD-XA
		{
			it += 0x01; // skip file number
			it += 0x01; // skip channel number
			auto const submode = *it++;
			it += 0x01; // skip coding info
			it += 0x04; // skip subheader copy

			if (submode & 0b0010'0000)
				raw_data.insert(raw_data.end(), it, it + 0x900); // form1
			else
				raw_data.insert(raw_data.end(), it, it + 0x800); // form2

			it += 0x918;
			break;
		}

		default:
		{
			it += 0x920;
			break;
		}
		}
	}

	return raw_data;
}

void unpack_iso(fs::path const& file_path)
{
	auto const data = load_file(file_path);
	if (data.empty())
		return;

	if (!is_psx_iso(data))
		return;

	auto it = data.begin();
	it += SECTOR_SIZE * 16;		// primary volume descriptor
	it += 0x18;					// skip sync + header
	it += 0x9C;					// jump to root directory record

	auto const root_record = read_directory_record({ it, data.end() });

	it = data.begin();
	it += (root_record.sector * SECTOR_SIZE);	// root_directory
	it += 0x18;									// skip sync + header

	auto records = std::vector<directory_record>{};
	while (*it)
	{
		auto record = read_directory_record({ it, data.end() });
		it += record.size;
		records.push_back(std::move(record));
	}

	auto const input_directory = file_path.stem();
	fs::create_directory(input_directory);
	if (!fs::exists(input_directory))
		return;
	fs::current_path(input_directory);

	for (auto const& record : records)
	{
		if (record.type == record_type::directory) // handle this better
			continue;

		auto const beg = data.begin() + (record.sector * SECTOR_SIZE);
		auto size = std::max((record.data_size / 0x800), 1u) * SECTOR_SIZE;

		auto out_file = std::ofstream{ record.name, std::ios_base::binary };
		out_file.write(reinterpret_cast<char const*>(&(*beg)), size);
	}
}