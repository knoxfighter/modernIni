#include "modernIni.h"

TEST(ModernIniGetTests, GetString) {
	modernIni::Ini ini;
	std::istringstream input("test = value");
	input >> ini;


	auto value = ini.get<std::string>();
	ASSERT_EQ(value, "value");
}
