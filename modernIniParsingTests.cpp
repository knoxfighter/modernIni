#include "modernIni.h"
#include "modernIniTestAccessor.h"

#include <gtest/gtest.h>

TEST(ModernIniParsingTests, ParseSimple) {
	modernIni::Ini ini;
	std::istringstream input("test = 1");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 1, nullptr);
	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test");
	ModernIniTestAccessor::check(elem, "test", "1", &ini);
}

TEST(ModernIniParsingTests, ParseTrims) {
	modernIni::Ini ini;
	std::istringstream input("  test = 1  ");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 1, nullptr);
	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test");
	ModernIniTestAccessor::check(elem, "test", "1", &ini);
}

TEST(ModernIniParsingTests, ParseEmpty) {
	modernIni::Ini ini;
	std::istringstream input("  ");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 0, nullptr);
}

TEST(ModernIniParsingTests, ParseEmptyValue) {
	modernIni::Ini ini;
	std::istringstream input("test = ");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 1, nullptr);
	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test");
	ModernIniTestAccessor::check(elem, "test", "", &ini);
}

TEST(ModernIniParsingTests, ParseWithComment) {
	modernIni::Ini ini;
	std::istringstream input("test = 1 ; comment");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 1, nullptr);
	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test");
	ModernIniTestAccessor::check(elem, "test", "1", &ini);
}

TEST(ModernIniParsingTests, ParseEmptyComment) {
	modernIni::Ini ini;
	std::istringstream input("; comment");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 0, nullptr);
}

TEST(ModernIniParsingTests, ParseEscaped) {
	modernIni::Ini ini;
	std::istringstream input(R"(test = \n\;\\)");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 1, nullptr);
	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test");
	ModernIniTestAccessor::check(elem, "test", "\n;\\", &ini);
}

TEST(ModernIniParsingTests, ParseMultiple) {
	modernIni::Ini ini;
	std::istringstream input("test = 1\nkey = value");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 2, nullptr);
	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test");
	ModernIniTestAccessor::check(elem, "test", "1", &ini);
	const auto& elem2 = children.at("key");
	ModernIniTestAccessor::check(elem2, "key", "value", &ini);
}

TEST(ModernIniParsingTests, ParseCategory) {
	modernIni::Ini ini;
	std::istringstream input("[test]\nkey = value");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 1, nullptr);
	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test");
	ModernIniTestAccessor::check(elem, "test", 1, &ini);
	const auto& children2 = ModernIniTestAccessor::getPrivateChildren(elem);
	const auto& elem2 = children2.at("key");
	ModernIniTestAccessor::check(elem2, "key", "value", &elem);
}

TEST(ModernIniParsingTests, ParseCategoryWithWhitespace) {
	modernIni::Ini ini;
	std::istringstream input(" [ test   ] \nkey = value");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 1, nullptr);
	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test");
	ModernIniTestAccessor::check(elem, "test", 1, &ini);
	const auto& children2 = ModernIniTestAccessor::getPrivateChildren(elem);
	const auto& elem2 = children2.at("key");
	ModernIniTestAccessor::check(elem2, "key", "value", &elem);
}

TEST(ModernIniParsingTests, ParseMultipleCategories) {
	// multiple categories with multiple subcategories
	modernIni::Ini ini;
	std::istringstream input("[test2]\nkey = value\n[test2] [test3]\nkey = value");
	input >> ini;

	ModernIniTestAccessor::check(ini, "", 1, nullptr);

	const auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	const auto& elem = children.at("test2");
	ModernIniTestAccessor::check(elem, "test2", 2, &ini);

	const auto& children2 = ModernIniTestAccessor::getPrivateChildren(elem);
	const auto& elem2 = children2.at("key");
	ModernIniTestAccessor::check(elem2, "key", "value", &elem);

	const auto& elem3 = children2.at("test3");
	ModernIniTestAccessor::check(elem3, "test3", 1, &elem);
	const auto& children3 = ModernIniTestAccessor::getPrivateChildren(elem3);

	const auto& elem4 = children3.at("key");
	ModernIniTestAccessor::check(elem4, "key", "value", &elem3);
}
