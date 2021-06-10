#include "pch.h"

#include <filesystem>
#include <fstream>

#include "../modernIni/modernIniMacros.h"

import modernIni;

using std::string_literals::operator ""s;

typedef modernIni::Ini Ini;
typedef std::map<std::string, Ini> IniMap;

namespace {
	TEST(DefaultContainerTests, stdArrayToIni) {
		std::array arr = { 1, 2, 3 };

		Ini iniTest = {
			IniMap {
				{"0", Ini("0", "1")},
				{"1", Ini("1", "2")},
				{"2", Ini("2", "3")}
			}
		};

		Ini ini(arr);

		ASSERT_EQ(ini, iniTest);
	}

	TEST(DefaultContainerTests, stdArrayFromIni) {
		Ini iniTest = {
			IniMap {
				{"0", Ini("0", "1")},
				{"1", Ini("1", "2")},
				{"2", Ini("2", "3")}
			}
		};
		std::array arrTest = { 1, 2, 3 };
		std::array<int, 3> arr;

		iniTest.get_to(arr);

		ASSERT_EQ(arr, arrTest);
	}
}