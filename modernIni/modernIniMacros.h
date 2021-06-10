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
	inline void from_ini(Type& obj, const modernIni::Ini& ini) { \
		MAP(MODERN_INI_GET_TO_SINGLE_THROWING, __VA_ARGS__) \
	} \
	inline void to_ini(const Type& obj, modernIni::Ini& ini) { \
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
	inline void from_ini(Type& obj, const modernIni::Ini& ini) noexcept { \
		MAP(MODERN_INI_GET_TO_SINGLE_NO_EXCEPT, __VA_ARGS__) \
	} \
	inline void to_ini(const Type& obj, modernIni::Ini& ini) { \
		MAP(MODERN_INI_ASSIGN_SINGLE, __VA_ARGS__) \
	}


#define MODERN_INI_SERIALIZE_ENUM_SINGLE_FROM(value, key) {#value, key::value}
#define MODERN_INI_SERIALIZE_ENUM_SINGLE_TO(value, key) {key::value, #value}
#define MODERN_INI_SERIALIZE_ENUM(ENUM_TYPE, ...) \
	static_assert(std::is_enum_v<ENUM_TYPE>, #ENUM_TYPE " must be an enum!"); \
	inline void from_ini(ENUM_TYPE& e, const modernIni::Ini& ini) { \
		std::map<std::string, ENUM_TYPE> enumIniLookup{ \
			MAP_LIST_UD(MODERN_INI_SERIALIZE_ENUM_SINGLE_FROM, ENUM_TYPE, __VA_ARGS__) \
		}; \
		auto found = enumIniLookup.find(ini.get<std::string>()); \
		if (found != enumIniLookup.end()) { \
			e = found->second; \
		} \
	} \
	inline void to_ini(const ENUM_TYPE& e, modernIni::Ini& ini) { \
		std::map<ENUM_TYPE, std::string> enumIniLookup{ \
			MAP_LIST_UD(MODERN_INI_SERIALIZE_ENUM_SINGLE_TO, ENUM_TYPE, __VA_ARGS__) \
		}; \
		auto found = enumIniLookup.find(e); \
		if (found != enumIniLookup.end()) { \
			ini = found->second; \
		} \
	}
