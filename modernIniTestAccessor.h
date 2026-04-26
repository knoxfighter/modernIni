#pragma once

#include "modernIni.h"

#include <gtest/gtest.h>

class ModernIniTestAccessor {
public:
	static const modernIni::Ini::ValueStorage& getPrivateValue(const modernIni::Ini& ini) {
		return std::get<modernIni::Ini::ValueStorage>(ini.data);
	}
	static const modernIni::Ini::ObjectStorage& getPrivateChildren(const modernIni::Ini& ini) {
		return std::get<modernIni::Ini::ObjectStorage>(ini.data);
	}
	static modernIni::Ini::ObjectStorage& getPrivateChildren(modernIni::Ini& ini) {
		return std::get<modernIni::Ini::ObjectStorage>(ini.data);
	}
	static modernIni::Ini* getPrivateParent(const modernIni::Ini& ini) {
		return ini.parent;
	}
	static const std::string& getPrivateKey(const modernIni::Ini& ini) {
		return ini.key;
	}

	// static void setPrivateValue(modernIni::Ini& ini, std::string value) {
	// 	ini.data = std::move(value);
	// }
	// static void setPrivateChildren(modernIni::Ini& ini, modernIni::Ini::ObjectStorage children) {
	// 	ini.data = std::move(children);
	// }
	// static void setPrivateParent(modernIni::Ini& ini, modernIni::Ini* parent) {
	// 	ini.parent = parent;
	// }

	static void check(const modernIni::Ini& ini, const std::string& key, const std::string& value, const modernIni::Ini* parent = nullptr) {
		ASSERT_EQ(ini.type, modernIni::Type::Value);
		ASSERT_EQ(getPrivateKey(ini), key);
		ASSERT_EQ(getPrivateValue(ini), value);
		ASSERT_EQ(getPrivateParent(ini), parent);
	}
	static void check(const modernIni::Ini& ini, const std::string& key, size_t childSize, const modernIni::Ini* parent = nullptr) {
		ASSERT_EQ(ini.type, modernIni::Type::Object);
		ASSERT_EQ(getPrivateKey(ini), key);
		ASSERT_EQ(getPrivateChildren(ini).size(), childSize);
		ASSERT_EQ(getPrivateParent(ini), parent);
	}
};
