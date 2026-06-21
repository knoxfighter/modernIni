#include "modernIni.h"
#include "modernIniMacros.h"
#include "modernIniTestAccessor.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <expected>
#include <flat_map>
#include <flat_set>
#include <forward_list>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

using namespace std::string_literals;

template<typename T>
void testGet(const modernIni::Ini& ini, const std::string& key, const T& expected) {
	auto res = ini.at(key).transform([&](const modernIni::Ini& value) -> modernIni::Result<T> {
		return value.get<T>();
	});
	ASSERT_TRUE(res.has_value());
	ASSERT_EQ(res.value(), expected);
}
template<typename T>
void testGet(const modernIni::Ini& ini, const std::string& key, modernIni::Error err) {
	auto res = ini.at(key).and_then([&](const modernIni::Ini& value) -> modernIni::Result<T> {
		return value.get<T>();
	});
	ASSERT_FALSE(res.has_value());
	ASSERT_EQ(res.error(), err);
}

template<typename T>
void testGetTo(const modernIni::Ini& ini, const std::string& key, const T& expected) {
	T out;
	auto res = ini.at(key).and_then([&](const modernIni::Ini& value) -> modernIni::Result<void> { return value.get_to(out); });
	ASSERT_TRUE(res.has_value());
	ASSERT_EQ(out, expected);
}
template<typename T>
void testGetTo(const modernIni::Ini& ini, const std::string& key, modernIni::Error err) {
	T out;
	auto res = ini.at(key).and_then([&](const modernIni::Ini& value) -> modernIni::Result<void> { return value.get_to(out); });
	ASSERT_FALSE(res.has_value());
	ASSERT_EQ(res.error(), err);
}

TEST(ModernIniGetTests, GetString) {
	modernIni::Ini ini;
	std::istringstream input("test = value");
	input >> ini;

	testGet(ini, "test", "value"s);
	testGetTo(ini, "test", "value"s);
}

TEST(ModernIniGetTests, GetStringView) {
	modernIni::Ini ini;
	std::istringstream input("test = value");
	input >> ini;

	testGet<std::string_view>(ini, "test", "value"s);
	testGetTo<std::string_view>(ini, "test", "value"s);
}

TEST(ModernIniGetTests, GetStringViewLifetime) {
	modernIni::Ini ini;
	std::istringstream input("test = value");
	input >> ini;

	auto value = ini.at("test").value().get().get<std::string_view>().value();
	ASSERT_EQ(value, "value");
	auto& child = ModernIniTestAccessor::getPrivateChildren(ini).at("test");
	const auto& val = ModernIniTestAccessor::getPrivateValue(child);
	// check if the string points to the same memory than the view.
	ASSERT_EQ(val.data(), value.data());
}

TEST(ModernIniGetTests, KeyNotFoundError) {
	modernIni::Ini ini;
	std::istringstream input("test = value\ntest2 = askjhdf");
	input >> ini;

	testGet<std::string>(ini, "test3", modernIni::Error::KeyNotFound);
	testGetTo<std::string>(ini, "test3", modernIni::Error::KeyNotFound);
}

TEST(ModernIniGetTests, WrongTypeError) {
	modernIni::Ini ini;
	std::istringstream input("test1 = value\n[test]\ntest2 = askjhdf");
	input >> ini;

	testGet<std::string>(ini, "test", modernIni::Error::WrongType);
	testGetTo<std::string>(ini, "test", modernIni::Error::WrongType);

	modernIni::Ini& test = ini.at("test1").value();
	auto t = test.at("test");
	ASSERT_FALSE(t.has_value());
	ASSERT_EQ(t.error(), modernIni::Error::WrongType);
}

TEST(ModernIniGetTests, GetBool) {
	modernIni::Ini ini;
	std::istringstream input("test = 1\ntest2 = true\ntest3 = on\ntest4 = 0\ntest5 = false\ntest6 = off");
	input >> ini;

	testGet(ini, "test", true);
	testGetTo(ini, "test", true);

	testGet(ini, "test2", true);
	testGetTo(ini, "test2", true);

	testGet(ini, "test3", true);
	testGetTo(ini, "test3", true);

	testGet(ini, "test4", false);
	testGetTo(ini, "test4", false);

	testGet(ini, "test5", false);
	testGetTo(ini, "test5", false);

	testGet(ini, "test6", false);
	testGetTo(ini, "test6", false);
}

TEST(ModernIniGetTests, GetBoolError) {
	modernIni::Ini ini;
	std::istringstream input("test1 = 2\ntest2 = askjhdf");
	input >> ini;

	testGet<bool>(ini, "test1", modernIni::Error::InvalidValue);
	testGetTo<bool>(ini, "test1", modernIni::Error::InvalidValue);

	testGet<bool>(ini, "test2", modernIni::Error::InvalidValue);
	testGetTo<bool>(ini, "test2", modernIni::Error::InvalidValue);
}

TEST(ModernIniGetTests, GetInt) {
	modernIni::Ini ini;
	std::istringstream input("test = 42");
	input >> ini;

	testGet(ini, "test", static_cast<int8_t>(42));
	testGetTo(ini, "test", static_cast<int8_t>(42));

	testGet(ini, "test", static_cast<int16_t>(42));
	testGetTo(ini, "test", static_cast<int16_t>(42));

	testGet(ini, "test", static_cast<int32_t>(42));
	testGetTo(ini, "test", static_cast<int32_t>(42));

	testGet(ini, "test", static_cast<int64_t>(42));
	testGetTo(ini, "test", static_cast<int64_t>(42));

	testGet(ini, "test", static_cast<uint8_t>(42));
	testGetTo(ini, "test", static_cast<uint8_t>(42));

	testGet(ini, "test", static_cast<uint16_t>(42));
	testGetTo(ini, "test", static_cast<uint16_t>(42));

	testGet(ini, "test", static_cast<uint32_t>(42));
	testGetTo(ini, "test", static_cast<uint32_t>(42));

	testGet(ini, "test", static_cast<uint64_t>(42));
	testGetTo(ini, "test", static_cast<uint64_t>(42));

	testGet(ini, "test", static_cast<float>(42));
	testGetTo(ini, "test", static_cast<float>(42));

	testGet(ini, "test", static_cast<double>(42));
	testGetTo(ini, "test", static_cast<double>(42));
}

TEST(ModernIniGetTests, GetIntNegative) {
	modernIni::Ini ini;
	std::istringstream input("test = -42");
	input >> ini;

	testGet(ini, "test", -42);
	testGetTo(ini, "test", -42);

	testGet<uint32_t>(ini, "test", modernIni::Error::InvalidValue);
	testGetTo<uint32_t>(ini, "test", modernIni::Error::InvalidValue);
}

TEST(ModernIniGetTests, GetIntTypes) {
	modernIni::Ini ini;
	std::istringstream input("test = 0x1F\ntest2 = 0o10\ntest3 = 0b10\ntest4 = -0x1F\ntest5 = -0o10\ntest6 = -0b10\n");
	input >> ini;

	testGet(ini, "test", 0x1F);
	testGetTo(ini, "test", 0x1F);

	testGet(ini, "test2", 010);
	testGetTo(ini, "test2", 010);

	testGet(ini, "test3", 0b10);
	testGetTo(ini, "test3", 0b10);

	testGet(ini, "test4", -0x1F);
	testGetTo(ini, "test4", -0x1F);

	testGet(ini, "test5", -010);
	testGetTo(ini, "test5", -010);

	testGet(ini, "test6", -0b10);
	testGetTo(ini, "test6", -0b10);
}

TEST(ModernIniGetTests, GetIntPartialString) {
	modernIni::Ini ini;
	std::istringstream input("test = 42abc");
	input >> ini;

	// Document whether partial parses are accepted or rejected
	testGet<int>(ini, "test", 42);
	testGetTo<int>(ini, "test", 42);
}

TEST(ModernIniGetTests, GetIntError) {
	modernIni::Ini ini;
	std::istringstream input("test = askjhdf\ntest2 = 99999999999999999999999\ntest3 = -1\ntest4 = ");
	input >> ini;

	testGet<int>(ini, "test", modernIni::Error::InvalidValue);
	testGetTo<int>(ini, "test", modernIni::Error::InvalidValue);

	testGet<int>(ini, "test2", modernIni::Error::InvalidValue);
	testGetTo<int>(ini, "test2", modernIni::Error::InvalidValue);

	testGet<int>(ini, "test4", modernIni::Error::InvalidValue);
	testGetTo<int>(ini, "test4", modernIni::Error::InvalidValue);
}

TEST(ModernIniGetTests, GetFloat) {
	modernIni::Ini ini;
	std::istringstream input("test = 42.5\ntest2 = -42.5\n");
	input >> ini;

	// Float
	testGet(ini, "test", 42.5f);
	testGetTo(ini, "test", 42.5f);

	// Float negative
	testGet(ini, "test2", -42.5f);
	testGetTo(ini, "test2", -42.5f);

	// double
	testGet(ini, "test", 42.5);
	testGetTo(ini, "test", 42.5);

	// double negative
	testGet(ini, "test2", -42.5);
	testGetTo(ini, "test2", -42.5);

	// as int
	testGet(ini, "test", static_cast<int>(42));
	testGetTo(ini, "test", static_cast<int>(42));
}

TEST(ModernIniGetTests, GetFloatSpecialValues) {
	modernIni::Ini ini;
	std::istringstream input("test = inf\ntest2 = nan\ntest3 = -inf");
	input >> ini;

	testGet<float>(ini, "test",  std::numeric_limits<float>::infinity());
	testGetTo<float>(ini, "test",  std::numeric_limits<float>::infinity());
	ASSERT_TRUE(std::isnan(ini.at("test2").value().get().get<float>().value()));
	float f = 0.f;
	ASSERT_TRUE(ini.at("test2").value().get().get_to(f));
	ASSERT_TRUE(std::isnan(f));
	testGet<float>(ini, "test3", -std::numeric_limits<float>::infinity());
	testGetTo<float>(ini, "test3", -std::numeric_limits<float>::infinity());
}

TEST(ModernIniGetTests, GetFloatError) {
	modernIni::Ini ini;
	std::istringstream input("test = askjhdf");
	input >> ini;

	testGet<float>(ini, "test", modernIni::Error::InvalidValue);
	testGetTo<float>(ini, "test", modernIni::Error::InvalidValue);
}

TEST(ModernIniGetTests, GetObjectError) {
	modernIni::Ini ini;
	std::istringstream input("[test]");
	input >> ini;

	testGet<std::string>(ini, "test", modernIni::Error::WrongType);
	testGetTo<std::string>(ini, "test", modernIni::Error::WrongType);
}

TEST(ModernIniGetTests, GetToUnchangedOnError) {
	modernIni::Ini ini;
	std::istringstream input("test = abc");
	input >> ini;

	int test = 42;
	auto t = ini.at("test").and_then([&](const modernIni::Ini& value) -> modernIni::Result<void> { return value.get_to(test); });
	ASSERT_FALSE(t.has_value());
	ASSERT_EQ(t.error(), modernIni::Error::InvalidValue);
	ASSERT_EQ(test, 42);
}

enum class TestEnumDefault {
	Test1,
	Test2,
};
TEST(ModernIniGetTests, EnumMagic) {
	modernIni::Ini ini;
	std::istringstream input("test = Test1\n");
	input >> ini;

	TestEnumDefault test;
	ASSERT_TRUE(ini.at("test").value().get().get_to(test));
	ASSERT_EQ(test, TestEnumDefault::Test1);
}

TEST(ModernIniGetTests, EnumNum) {
	modernIni::Ini ini;
	std::istringstream input("test = 1\n");
	input >> ini;

	TestEnumDefault test;
	ASSERT_TRUE(ini.at("test").value().get().get_to(test));
	ASSERT_EQ(test, TestEnumDefault::Test2);
}

TEST(ModernIniGetTests, MapUnknownKey) {
	modernIni::Ini ini;
	std::istringstream input("Test1 = 1\nTest2 = 2\nTest10 = 10\nTest11 = 11\n");
	input >> ini;

	std::unordered_map<TestEnumDefault, uint32_t> defaultMap;

	std::unordered_map<TestEnumDefault, uint32_t> expectedMap;
	expectedMap[TestEnumDefault::Test1] = 1;
	expectedMap[TestEnumDefault::Test2] = 2;

	auto res = ini.get_to(defaultMap);
	ASSERT_FALSE(res);
	ASSERT_EQ(res.error(), modernIni::Error::InvalidValue);
}

TEST(ModernIniGetTests, Char) {
	modernIni::Ini ini;
	std::istringstream input("test = a\n");
	input >> ini;

	auto str = ini.at("test").value().get().get<char>();
	ASSERT_TRUE(str.has_value());
	ASSERT_EQ(str.value(), 'a');
}

TEST(ModernIniGetTests, GetOptionalValue) {
	modernIni::Ini ini;
	std::istringstream input("test = 42\n");
	input >> ini;

	auto value = ini.at("test").value().get().get<std::optional<int>>();
	ASSERT_TRUE(value.value().has_value());
	ASSERT_EQ(value.value().value(), 42);
}

TEST(ModernIniGetTests, GetOptionalEmpty) {
	modernIni::Ini ini;
	std::istringstream input("test = \n");
	input >> ini;

	auto value = ini.at("test").value().get().get<std::optional<int>>();
	ASSERT_FALSE(value.value().has_value());
}

struct OptionalTest {
	int x;
	int y;
	MODERN_INI_DEFINE_TYPE_INTRUSIVE(OptionalTest, x, y);
};
TEST(ModernIniGetTests, GetOptionalObject) {
	modernIni::Ini ini;
	std::istringstream input("[key]\nx = 5\ny = 5\n");
	input >> ini;

	auto value = ini.at("key").value().get().get<std::optional<OptionalTest>>();
	ASSERT_TRUE(value.value().has_value());
	ASSERT_EQ(value.value().value().x, 5);
	ASSERT_EQ(value.value().value().y, 5);
}

TEST(ModernIniGetTests, GetVectors) {
	auto testArray = []<typename T>() {
		modernIni::Ini ini;
		std::istringstream input("[test]\n 0 = 0\n1 = 1\n 2 = 2\n");
		input >> ini;

		auto value = ini.at("test").value().get().get<T>();
		ASSERT_TRUE(value.has_value());
		auto& vector = value.value();
		ASSERT_EQ(vector.size(), 3);
		for (auto&& [e, v] : vector | std::views::enumerate) {
			ASSERT_EQ(e, v);
		}
	};
	testArray.operator()<std::vector<int>>();
	testArray.operator()<std::deque<int>>();
	testArray.operator()<std::list<int>>();
	testArray.operator()<std::set<int>>();
	testArray.operator()<std::multiset<int>>();
	testArray.operator()<std::flat_set<int>>();
	testArray.operator()<std::flat_multiset<int>>();
}

TEST(ModernIniGetTests, GetForwardList) {
	modernIni::Ini ini;
	std::istringstream input("[test]\n 0 = 0\n1 = 1\n 2 = 2\n");
	input >> ini;

	auto value = ini.at("test").value().get().get<std::forward_list<int>>();
	ASSERT_TRUE(value.has_value());
	auto& vector = value.value();
	vector.reverse();
	for (auto&& [e, v] : vector | std::views::enumerate) {
		ASSERT_EQ(e, v);
	}
}

TEST(ModernIniGetTests, GetStack) {
	modernIni::Ini ini;
	std::istringstream input("[test]\n 0 = 0\n1 = 1\n 2 = 2\n");
	input >> ini;

	auto value = ini.at("test").value().get().get<std::stack<int>>();
	ASSERT_TRUE(value.has_value());
	auto& vector = value.value();
	for (auto i = vector.top(), j = 2; (i = vector.top()); --j) {
		vector.pop();
		ASSERT_EQ(i, j);
	}
}

TEST(ModernIniGetTests, GetArray) {
	auto testArray = []<typename T>(const std::string& str, std::size_t max = -1) {
		modernIni::Ini ini;
		std::istringstream input(str);
		input >> ini;

		auto value = ini.at("test").value().get().get<T>();
		ASSERT_TRUE(value.has_value());
		auto& vector = value.value();
		for (auto&& [e, v] : vector | std::views::enumerate) {
			if (e >= max)
				break;
			ASSERT_EQ(e, v);
		}
	};

	testArray.operator()<std::array<int, 3>>("[test]\n 0 = 0\n1 = 1\n 2 = 2\n");
	testArray.operator()<std::array<int, 2>>("[test]\n 0 = 0\n1 = 1\n 2 = 2\n");
	testArray.operator()<std::array<int, 4>>("[test]\n 0 = 0\n1 = 1\n 2 = 2\n", 3);
}

enum class KeyEnum {
	a,
	b,
	c,
};
template<typename T>
void testMap() {
	modernIni::Ini ini;
	std::istringstream input("[test]\n a = 1\n b = 2\n c = 3\n");
	input >> ini;

	auto value = ini.at("test").value().get().get<T>();
	ASSERT_TRUE(value.has_value());
	auto& map = value.value();
	ASSERT_EQ(map.size(), 3);
	ASSERT_EQ(map.find(KeyEnum::a)->second, 1);
	ASSERT_EQ(map.find(KeyEnum::b)->second, 2);
	ASSERT_EQ(map.find(KeyEnum::c)->second, 3);
}
TEST(ModernIniGetTests, GetMaps) {
	testMap<std::map<KeyEnum, int>>();
	testMap<std::unordered_map<KeyEnum, int>>();
	testMap<std::flat_map<KeyEnum, int>>();

	testMap<std::multimap<KeyEnum, int>>();
	testMap<std::unordered_multimap<KeyEnum, int>>();
	testMap<std::flat_multimap<KeyEnum, int>>();
}
