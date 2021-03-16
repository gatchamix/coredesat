#pragma once

#include "..\pch.h"
#include "..\util\io.h"

enum class chart_note_type
{
	player1_button1 = 0x0,
	player2_button1 = 0x1,
	player1_button2 = 0x2,
	player2_button2 = 0x3,
	player1_button3 = 0x4,
	player2_button3 = 0x5,
	player1_button4 = 0x6,
	player2_button4 = 0x7,
	player1_button5 = 0x8,
	player2_button5 = 0x9,
	player1_turntable = 0xA,
	player2_turntable = 0xB,
	player1_measure = 0xC,
	player2_measure = 0xD,
	player1_free_turn = 0xE,
	player2_free_turn = 0xF,
};

enum class chart_event_type
{
	button = 0x0,
	operation = 0x1,
	tempo = 0x2,
	beat = 0x3,
	music_end = 0x4,
	sound_req = 0x5,
	judge_range = 0x6,
	judge_sound = 0x7,
	judge_timing = 0x8, // judge_trigger ?
	phrase_set = 0x9
};

struct chart_event
{
	uint16_t offset;
	chart_event_type type;
};

struct chart_note
	: chart_event
{
	chart_note_type type;
};

struct chart_operation
	: chart_event
{
	chart_note_type type;
	uint16_t keysound_id;
};

struct chart_bpm_change
	: chart_event
{
	uint16_t bpm;
};

struct bmdata_chart
{
	uint32_t player1_notecount;
	uint32_t player2_notecount;
	std::vector<std::unique_ptr<chart_event>> events;
};

auto extract_bmdata_chart(data_view data)
-> bmdata_chart
{
	auto it = data.begin();

	auto chart = bmdata_chart{};

	for (;;)
	{
		auto const count = parseLE<uint32_t>(it) & 0xFF;
		chart.player1_notecount += count;
		if (count != 0xFA)
			break;
	}

	for (;;)
	{
		auto const count = parseLE<uint32_t>(it) & 0xFF;
		chart.player2_notecount += count;
		if (count != 0xFA)
			break;
	}

	std::cout << "notecount : " << chart.player1_notecount << " - " << chart.player2_notecount << std::endl;

	auto& events = chart.events;
	while (it != data.end())
	{
		auto const offset = parseBE<uint16_t>(it);
		if (offset == 0x7FFF)
			break;

		auto const event_data = parseBE<uint16_t>(it);

		auto const event_type = static_cast<chart_event_type>(event_data & 0xF);
		switch (event_type)
		{
		case chart_event_type::button:
		{
			auto const button = static_cast<chart_note_type>((event_data >> 4) & 0xF);
			auto event = std::make_unique<chart_note>(chart_event { offset, event_type }, button);
			events.push_back(std::move(event));
			break;
		}

		case chart_event_type::operation:
		{
			auto const button = static_cast<chart_note_type>((event_data >> 4) & 0xF);
			auto const keysound_id = (event_data >> 12);
			auto event = std::make_unique<chart_operation>(chart_event { offset, event_type }, button, keysound_id);
			events.push_back(std::move(event));
			break;
		}

		case chart_event_type::tempo:
		{
			auto const bpm = (event_data >> 8);
			auto event = std::make_unique<chart_bpm_change>(chart_event { offset, event_type }, bpm);
			events.push_back(std::move(event));
			break;
		}
		
		case chart_event_type::beat:
			break;

		case chart_event_type::music_end:
			break;

		case chart_event_type::sound_req:
			break;

		case chart_event_type::judge_sound:
			break;

		case chart_event_type::judge_timing:
			break;

		case chart_event_type::phrase_set:
			break;

		case chart_event_type::judge_range:
		default:
			break;
		}
	}

	return chart;
}