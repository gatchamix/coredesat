// coredesat.cpp : Defines the entry point for the application.
//

#include "pch.h"

#include "util/iso9660.h"
#include "system/psx.h"

#include "sysdata/sysdata.h"
#include "bmdata/bmdata.h"
#include "mchdata/mchdata.h"

auto main(int argc, char** argv)
-> int
{
	auto args = std::span{ argv, argv + argc };
	if (args.size() < 2)
	{
		auto const exe_name = fs::path{ args[0] }.filename();
		std::cout
			<< "usage: " << exe_name << " files...\n"
			<< "currently supported:\n"
			<< "- PSX CD images"
			<< std::endl;
		return 1;
	}

	auto const base_dir = fs::current_path();
	for (auto it = args.begin() + 1; it != args.end(); ++it)
	{
		fs::current_path(base_dir);
		auto const working_dir = fs::path{ *it }.parent_path();
		fs::current_path(working_dir);

		unpack_iso(*it);

		process_system_file();
		process_sysdata_file();
		process_bmdata_file();
		process_mchdata_file();
	}
}