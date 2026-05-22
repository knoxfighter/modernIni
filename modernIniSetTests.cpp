#include "modernIni.h"

#include <array>
#include <deque>
#include <expected>
#include <forward_list>
#include <gtest/gtest.h>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <vector>

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

TEST(ModernIniSetTests, SetVector) {
	modernIni::Ini ini;
	ini["key"] = std::vector<int>{1, 2, 3};

	auto& key = ini.at("key").value().get();
	ASSERT_EQ(key.getType(), modernIni::Type::Object);

	auto& children = key.getChildren().value().get();
	ASSERT_EQ(children.size(), 3);

	ASSERT_EQ(children.at("0").get<int>(), 1);
	ASSERT_EQ(children.at("1").get<int>(), 2);
	ASSERT_EQ(children.at("2").get<int>(), 3);
}

void testMap(auto&& t) {
	modernIni::Ini ini;
	ini["key"] = t;

	auto& key = ini.at("key").value().get();
	ASSERT_EQ(key.getType(), modernIni::Type::Object);

	auto& children = key.getChildren().value().get();
	ASSERT_EQ(children.size(), 2);

	ASSERT_EQ(children.at("a").get<std::string_view>(), "1");
	ASSERT_EQ(children.at("b").get<std::string_view>(), "2");
}

TEST(ModernIniSetTest, SetMaps) {
	testMap(std::map<std::string, int>{{"a", 1}, {"b", 2}});
	testMap(std::unordered_map<std::string, int>{{"a", 1}, {"b", 2}});
	testMap(std::multimap<std::string, int>{{"a", 1}, {"b", 2}});
	testMap(std::unordered_multimap<std::string, int>{{"a", 1}, {"b", 2}});
	testMap(std::array<std::pair<std::string, int>, 2>{{{"a", 1}, {"b", 2}}});

	testMap(std::map<std::string, std::string>{{"a", "1"}, {"b", "2"}});

	modernIni::Ini ini;
	ini["key"] = std::map<int, int>{{1, 2}, {3, 4}};

	auto& key = ini.at("key").value().get();
	ASSERT_EQ(key.getType(), modernIni::Type::Object);

	auto& children = key.getChildren().value().get();
	ASSERT_EQ(children.size(), 2);

	ASSERT_EQ(children.at("1").get<std::string_view>(), "2");
	ASSERT_EQ(children.at("3").get<std::string_view>(), "4");
}

void testArray(auto&& t) {
	modernIni::Ini ini;
	ini["key"] = t;

	auto& key = ini.at("key").value().get();
	ASSERT_EQ(key.getType(), modernIni::Type::Object);

	auto& children = key.getChildren().value().get();
	ASSERT_EQ(children.size(), 2);

	ASSERT_EQ(children.at("0").get<int>(), 1);
	ASSERT_EQ(children.at("1").get<int>(), 2);
}
TEST(ModernIniSetTests, SetArrays) {
	testArray(std::array{1, 2});
	testArray(std::vector{1, 2});
	testArray(std::deque{1, 2});
	testArray(std::list{1, 2});
	testArray(std::forward_list{1, 2});
	testArray(std::set{1, 2});
	testArray(std::multiset{1, 2});
	testArray(std::unordered_set{1, 2});
	testArray(std::valarray{1, 2});

	std::array<int, 2> arr = {1, 2};
	testArray(std::span(arr));
}
