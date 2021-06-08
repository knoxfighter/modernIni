#pragma once

import modernIni;

#include "map.h"

#define MODERN_INI_GET_TO_SINGLE_THROWING(key) ini.at(#key).get_to(obj.key);
#define MODERN_INI_ASSIGN_SINGLE(key) ini[#key] = obj.key;

#define MODERN_INI_DEFINE_TYPE_INTRUSIVE(Type, ...) \
	friend void from_ini(Type& obj, const modernIni::Ini& ini) { \
		MAP(MODERN_INI_GET_TO_SINGLE_THROWING, __VA_ARGS__) \
	} \
	friend void to_ini(const Type& obj, modernIni::Ini& ini) { \
		MAP(MODERN_INI_ASSIGN_SINGLE, __VA_ARGS__) \
	}

#define MODERN_INI_DEFINE_TYPE_NON_INTRUSIVE(Type, ...) \
	void from_ini(Type& obj, const modernIni::Ini& ini) { \
		MAP(MODERN_INI_GET_TO_SINGLE_THROWING, __VA_ARGS__) \
	} \
	void to_ini(const Type& obj, modernIni::Ini& ini) { \
		MAP(MODERN_INI_ASSIGN_SINGLE, __VA_ARGS__) \
	}

#define MODERN_INI_GET_TO_SINGLE_NO_EXCEPT(key) if (ini.has(#key)) ini.at(#key).get_to(obj.key);

#define MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(Type, ...) \
	friend void from_ini(Type& obj, const modernIni::Ini& ini) noexcept { \
		MAP(MODERN_INI_GET_TO_SINGLE_NO_EXCEPT, __VA_ARGS__) \
	} \
	friend void to_ini(const Type& obj, modernIni::Ini& ini) { \
		MAP(MODERN_INI_ASSIGN_SINGLE, __VA_ARGS__) \
	}

#define MODERN_INI_DEFINE_TYPE_NON_INTRUSIVE_NO_EXCEPT(Type, ...) \
	void from_ini(Type& obj, const modernIni::Ini& ini) noexcept { \
		MAP(MODERN_INI_GET_TO_SINGLE_NO_EXCEPT, __VA_ARGS__) \
	} \
	void to_ini(const Type& obj, modernIni::Ini& ini) { \
		MAP(MODERN_INI_ASSIGN_SINGLE, __VA_ARGS__) \
	}
