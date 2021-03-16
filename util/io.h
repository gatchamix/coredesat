#pragma once

#include "..\pch.h"

using data_file = std::vector<uint8_t>;
using data_view = std::span<uint8_t const>;
using data_span = std::span<uint8_t>;

template <std::integral T>
constexpr auto parseBE(std::input_or_output_iterator auto& it)
{
	auto val = T{};
	for (size_t i = 0; i < sizeof(T); ++i)
		val |= (static_cast<T>(*it++) << (i * 8));
	return val;
}

template <std::integral T>
constexpr auto parseLE(std::input_or_output_iterator auto& it)
{
	auto val = T{};
	for (auto i = sizeof(T); i > 0; --i)
		val |= (static_cast<T>(*it++) << ((i - 1) * 8));
	return val;
}

template <std::integral T>
constexpr void writeBE(std::ostream& stream, T val)
{
	for (auto i = 0; i < sizeof(T); ++i)
		stream.put((val >> (i * 8)) & 0xFF);
}

template <std::integral T>
constexpr void writeLE(std::ostream& stream, T val)
{
	for (auto i = sizeof(T); i > 0; --i)
		stream.put((val >> (i * 8)) & 0xFF);
}

auto load_file(fs::path filepath)
-> data_file
{
	auto data = std::vector<uint8_t>{};
	auto file = std::ifstream{ filepath, std::ios_base::binary };
	if (!file)
		return data;

	auto const filesize = fs::file_size(filepath);
	data.resize(filesize);
	file.read(reinterpret_cast<char *>(&data[0]), filesize);

	return data;
}