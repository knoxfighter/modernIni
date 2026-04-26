#include "modernIni.h"

#include <string>
#include <print>

#include <gtest/gtest.h>

using namespace std::string_literals;

template<typename Value, typename Type = Value>
void test(const std::string& key, Value value) {
	modernIni::Ini ini;
	ini[key] = value;
	ASSERT_EQ(ini.at(key).value().get().get<Type>(), value);
}

TEST(ModernIniSetTests, SetString) {
	test<std::string, std::string>("key", "value");
}

TEST(ModernIniSetTests, SetBool) {
	test("key", true);
}

TEST(ModernIniSetTests, SetInt) {
	test("key", 42);
}

TEST(ModernIniSetTests, SetFloat) {
	test("key", 42.5f);
	test("key", 42.5);
}
