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

	TEST(DefaultContainerTests, stdMapFromIniString) {
		Ini ini{
			IniMap {
				{"str1", Ini("str1", "0")},
				{"str2", Ini("str2", "1")},
				{"str3", Ini("str3", "2")}
			}
		};

		std::map<std::string, std::string> mapTest{
			{"str1", "0"},
			{"str2", "1"},
			{"str3", "2"}
		};
		std::map<std::string, std::string> map;

		ini.get_to(map);

		ASSERT_EQ(map, mapTest);
	}

	TEST(DefaultContainerTests, stdMapFromIniNum) {
		Ini ini{
			IniMap {
				{"7", Ini("7", "0")},
				{"5", Ini("5", "1")},
				{"1", Ini("1", "2")}
			}
		};

		std::map<uint16_t, std::string> mapTest{
			{7, "0"},
			{5, "1"},
			{1, "2"}
		};
		std::map<uint16_t, std::string> map;

		ini.get_to(map);

		ASSERT_EQ(map, mapTest);
	}

	enum class testEnum {
		t1,
		t2,
		t3
	};

	MODERN_INI_SERIALIZE_ENUM(testEnum, t1, t2, t3)

	TEST(DefaultContainerTests, stdMapFromIniEnum) {
		Ini ini{
			IniMap {
				{"t1", Ini("7", "0")},
				{"t2", Ini("5", "1")},
				{"t3", Ini("1", "2")}
			}
		};

		std::map<testEnum, std::string> mapTest{
			{testEnum::t1, "0"},
			{testEnum::t2, "1"},
			{testEnum::t3, "2"}
		};
		std::map<testEnum, std::string> map;

		ini.get_to(map);

		ASSERT_EQ(map, mapTest);
	}

	enum class testEnum2 {
		t1,
		t2,
		t3
	};

	TEST(DefaultContainerTests, stdMapFromIniEnumNum) {
		Ini ini{
			IniMap {
				{"0", Ini("0", "0")},
				{"1", Ini("1", "1")},
				{"2", Ini("2", "2")}
			}
		};

		std::map<testEnum2, std::string> mapTest{
			{testEnum2::t1, "0"},
			{testEnum2::t2, "1"},
			{testEnum2::t3, "2"}
		};
		std::map<testEnum2, std::string> map;

		ini.get_to(map);

		ASSERT_EQ(map, mapTest);
	}

	TEST(DefaultContainerTests, stdMapToIniString) {
		Ini iniTest{
			IniMap {
				{"0", Ini("0", "5")},
				{"5", Ini("5", "15")},
				{"174", Ini("174", "52")}
			}
		};

		std::map<std::string, std::string> map{
			{"0", "5"},
			{"5", "15"},
			{"174", "52"}
		};

		Ini ini(map);

		ASSERT_EQ(ini, iniTest);
	}

	TEST(DefaultContainerTests, stdMapToIniNum) {
		Ini iniTest{
			IniMap {
				{"0", Ini("0", "5")},
				{"5", Ini("5", "15")},
				{"174", Ini("174", "52")}
			}
		};

		std::map<uint16_t, std::string> map{
			{0, "5"},
			{5, "15"},
			{174, "52"}
		};

		Ini ini(map);

		ASSERT_EQ(ini, iniTest);
	}

	TEST(DefaultContainerTests, stdMapToIniEnum) {
		Ini iniTest{
			IniMap {
				{"t1", Ini("t1", "5")},
				{"t2", Ini("t2", "15")},
				{"t3", Ini("t3", "52")}
			}
		};

		std::map<testEnum, std::string> map{
			{testEnum::t1, "5"},
			{testEnum::t2, "15"},
			{testEnum::t3, "52"}
		};

		Ini ini(map);

		ASSERT_EQ(ini, iniTest);
	}

	TEST(DefaultContainerTests, stdMapToIniEnumNum) {
		Ini iniTest{
			IniMap {
				{"0", Ini("0", "5")},
				{"1", Ini("1", "15")},
				{"2", Ini("2", "52")}
			}
		};

		std::map<testEnum2, std::string> map{
			{testEnum2::t1, "5"},
			{testEnum2::t2, "15"},
			{testEnum2::t3, "52"}
		};

		Ini ini(map);

		ASSERT_EQ(ini, iniTest);
	}
}