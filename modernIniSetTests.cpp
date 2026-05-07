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

TEST(ModernIniSetTests, EnumDefault) {
	modernIni::Ini ini;
	enum class Test {
		Test1,
		Test2,
	};
	ini["key"] = Test::Test1;

	auto str = ini.at("key").value().get().get<std::string_view>();
	ASSERT_EQ(str.value(), "Test1");
}

enum class TestEnumMagic {
	Test1,
	Test2,
};
template<>
struct modernIni::customize::IniEnumTypeOverride<TestEnumMagic> {
	static constexpr auto type = IniEnumType::MagicEnum;
};
TEST(ModernIniSetTests, EnumMagic) {
	modernIni::Ini ini;

	ini["key"] = TestEnumMagic::Test2;

	auto str = ini.at("key").value().get().get<std::string_view>();
	ASSERT_EQ(str.value(), "Test2");
}

enum class TestEnumNum {
	Test1,
	Test2,
};
template<>
struct modernIni::customize::IniEnumTypeOverride<TestEnumNum> {
	static constexpr auto type = IniEnumType::Underlying;
};
TEST(ModernIniSetTests, EnumNum) {
	modernIni::Ini ini;

	ini["key"] = TestEnumNum::Test2;

	auto str = ini.at("key").value().get().get<std::string_view>();
	ASSERT_EQ(str.value(), "1");
}

TEST(ModernIniSetTests, Char) {
	modernIni::Ini ini;
	ini["key"] = 'a';

	auto str = ini.at("key").value().get().get<std::string_view>();
	ASSERT_EQ(str.value(), "a");
}

// WE DO NOT SUPPORT THESE AT THE MOMENT
// TEST(ModernIniSetTests, Char8) {
// 	modernIni::Ini ini;
// 	ini["key"] = u8'a';
//
// 	auto str = ini.at("key").value().get().get<std::string_view>();
// 	ASSERT_EQ(str.value(), "a");
// }
//
// TEST(ModernIniSetTests, Char16) {
// 	modernIni::Ini ini;
// 	ini["key"] = u'a';
//
// 	auto str = ini.at("key").value().get().get<std::string_view>();
// 	ASSERT_EQ(str.value(), "a");
// }
//
// TEST(ModernIniSetTests, Char32) {
// 	modernIni::Ini ini;
// 	ini["key"] = U'a';
//
// 	auto str = ini.at("key").value().get().get<std::string_view>();
// 	ASSERT_EQ(str.value(), "a");
// }
//
// TEST(ModernIniSetTests, WideChar) {
// 	modernIni::Ini ini;
// 	ini["key"] = L'a';
//
// 	auto str = ini.at("key").value().get().get<std::string_view>();
// 	ASSERT_EQ(str.value(), "a");
// }

TEST(ModernIniSetTests, SetOptionalValue) {
	modernIni::Ini ini;
	ini["key"] = std::optional<int>(42);


	auto value = ini.at("key")->get().get<std::string_view>();
	ASSERT_TRUE(value.has_value());
	ASSERT_EQ(value.value(), "42");
}

TEST(ModernIniSetTests, SetOptionalEmpty) {
	modernIni::Ini ini;
	ini["key"] = std::optional<int>(std::nullopt);

	auto value = ini.at("key")->get().get<std::string_view>();
	ASSERT_TRUE(value.has_value());
	ASSERT_TRUE(value->empty());
}
