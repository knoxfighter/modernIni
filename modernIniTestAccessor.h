#pragma once

#include "modernIni.h"

#include <gtest/gtest.h>

class ModernIniTestAccessor {
public:
	static modernIni::Type getPrivateType(const modernIni::Ini& ini) {
		return ini.type;
	}
	static const std::string& getPrivateValue(const modernIni::Ini& ini) {
		return std::get<std::string>(ini.data);
	}
	static const std::map<std::string, modernIni::Ini>& getPrivateChildren(const modernIni::Ini& ini) {
		return std::get<modernIni::Ini::ObjectStorage>(ini.data);
	}
	static std::map<std::string, modernIni::Ini>& getPrivateChildren(modernIni::Ini& ini) {
		return std::get<modernIni::Ini::ObjectStorage>(ini.data);
	}
	static modernIni::Ini* getPrivateParent(const modernIni::Ini& ini) {
		return ini.parent;
	}
	static const std::string& getPrivateKey(const modernIni::Ini& ini) {
		return ini.key;
	}

	static void setPrivateType(modernIni::Ini& ini, modernIni::Type type) {
		ini.type = type;
	}
	static void setPrivateValue(modernIni::Ini& ini, std::string value) {
		ini.data = std::move(value);
	}
	static void setPrivateChildren(modernIni::Ini& ini, modernIni::Ini::ObjectStorage children) {
		ini.data = std::move(children);
	}
	static void setPrivateParent(modernIni::Ini& ini, modernIni::Ini* parent) {
		ini.parent = parent;
	}

	static void checkValue(const modernIni::Ini& ini, const std::string& key, const std::string& value, const modernIni::Ini* parent = nullptr) {
		ASSERT_EQ(getPrivateType(ini), modernIni::Type::Value);
		ASSERT_EQ(getPrivateKey(ini), key);
		ASSERT_EQ(getPrivateValue(ini), value);
		ASSERT_EQ(getPrivateParent(ini), parent);
	}
	static void checkObject(const modernIni::Ini& ini, const std::string& key, size_t childSize, const modernIni::Ini* parent = nullptr) {
		ASSERT_EQ(getPrivateType(ini), modernIni::Type::Object);
		ASSERT_EQ(getPrivateChildren(ini).size(), childSize);
		ASSERT_EQ(getPrivateKey(ini), key);
		ASSERT_EQ(getPrivateParent(ini), parent);
	}
};
