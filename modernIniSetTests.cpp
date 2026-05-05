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

TEST(ModernIniSetTests, SetTwice) {
	modernIni::Ini ini;
	ini["key"] = 42;
	ASSERT_EQ(ini.at("key").value().get().get<int>(), 42);
	ini["key"] = "value";
	ASSERT_EQ(ini.at("key").value().get().get<std::string>(), "value");
}

TEST(ModernIniSetTests, SetCategory) {
	modernIni::Ini ini;
	ini["category"]["key"] = "value";
	ASSERT_EQ(ini.at("category").value().get().at("key").value().get().get<std::string>(), "value");
}

TEST(ModernIniSetTests, Contains) {
	modernIni::Ini ini;
	ini["key"] = "value";
	ASSERT_TRUE(ini.contains("key").value());
	ASSERT_FALSE(ini.contains("nonexistent").value());

	// fails if called on value
	auto wrongType = ini.at("key").value().get().contains("wrongtype");
	ASSERT_FALSE(wrongType.has_value());
	ASSERT_EQ(wrongType.error(), modernIni::Error::WrongType);
}

TEST(ModernIniSetTests, Erase) {
	modernIni::Ini ini;
	ini["key"] = "value";

	// fails if called on value
	auto wrongType = ini.at("key").value().get().erase("wrongtype");
	ASSERT_FALSE(wrongType.has_value());
	ASSERT_EQ(wrongType.error(), modernIni::Error::WrongType);

	// actual erasure test
	auto res = ini.erase("key");
	ASSERT_TRUE(res);
	ASSERT_FALSE(ini.contains("key").value());

}
