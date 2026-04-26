#include "modernIni.h"

#include <string>

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

	testGet(ini, "test", 42);
	testGetTo(ini, "test", 42);

	testGet(ini, "test", 42ull);
	testGetTo(ini, "test", 42ull);

	testGet(ini, "test", 42ll);
	testGetTo(ini, "test", 42ll);

	testGet(ini, "test", 42u);
	testGetTo(ini, "test", 42u);

	testGet(ini, "test", 42ul);
	testGetTo(ini, "test", 42ul);

	testGet(ini, "test", 42l);
	testGetTo(ini, "test", 42l);
}

TEST(ModernIniGetTests, GetIntNegative) {
	modernIni::Ini ini;
	std::istringstream input("test = -42");
	input >> ini;

	testGet(ini, "test", -42);
	testGetTo(ini, "test", -42);
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

TEST(ModernIniGetTests, GetIntError) {
	modernIni::Ini ini;
	std::istringstream input("test = askjhdf");
	input >> ini;

	testGet<int>(ini, "test", modernIni::Error::InvalidValue);
	testGetTo<int>(ini, "test", modernIni::Error::InvalidValue);
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
}

TEST(ModernIniGetTests, GetFloatError) {
	modernIni::Ini ini;
	std::istringstream input("test = askjhdf");
	input >> ini;

	testGet<float>(ini, "test", modernIni::Error::InvalidValue);
	testGetTo<float>(ini, "test", modernIni::Error::InvalidValue);
}