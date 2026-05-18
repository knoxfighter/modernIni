#pragma once

#include "modernIni.h"

// MACRO SOURCE: https://www.scs.stanford.edu/%7Edm/blog/va-opt.html
#define MODERN_INI_PARENS ()

// Expands the macro 342 times
#define MODERN_INI_EXPAND(arg) MODERN_INI_EXPAND1(MODERN_INI_EXPAND1(MODERN_INI_EXPAND1(MODERN_INI_EXPAND1(arg))))
#define MODERN_INI_EXPAND1(arg) MODERN_INI_EXPAND2(MODERN_INI_EXPAND2(MODERN_INI_EXPAND2(MODERN_INI_EXPAND2(arg))))
#define MODERN_INI_EXPAND2(arg) MODERN_INI_EXPAND3(MODERN_INI_EXPAND3(MODERN_INI_EXPAND3(MODERN_INI_EXPAND3(arg))))
#define MODERN_INI_EXPAND3(arg) MODERN_INI_EXPAND4(MODERN_INI_EXPAND4(MODERN_INI_EXPAND4(MODERN_INI_EXPAND4(arg))))
#define MODERN_INI_EXPAND4(arg) arg

#define MODERN_INI_FOR_EACH(macro, ...) \
	__VA_OPT__(MODERN_INI_EXPAND(MODERN_INI_FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define MODERN_INI_FOR_EACH_HELPER(macro, a1, ...) \
	macro(a1) __VA_OPT__(MODERN_INI_FOR_EACH_AGAIN MODERN_INI_PARENS(macro, __VA_ARGS__))
#define MODERN_INI_FOR_EACH_AGAIN() MODERN_INI_FOR_EACH_HELPER

#define MODERN_INI_FROM(v1) \
	std::ignore = ini.at(#v1).and_then([&](const modernIni::Ini& value) -> modernIni::Result<void> { return value.get_to(obj.v1); });
#define MODERN_INI_TO(v1) \
	ini[#v1] = obj.v1;

#define MODERN_INI_DEFINE_TYPE_INTRUSIVE(Type, ...)                                                              \
	friend void to_ini(modernIni::Ini& ini, const Type& obj) { MODERN_INI_FOR_EACH(MODERN_INI_TO, __VA_ARGS__) } \
	[[nodiscard]] friend modernIni::Result<void> from_ini(const modernIni::Ini& ini, Type& obj) { MODERN_INI_FOR_EACH(MODERN_INI_FROM, __VA_ARGS__) return {}; }

#define MODERN_INI_DEFINE_TYPE(Type, ...) \
	inline void to_ini(modernIni::Ini& ini, const Type& obj) { MODERN_INI_FOR_EACH(MODERN_INI_TO, __VA_ARGS__) } \
	[[nodiscard]] inline modernIni::Result<void> from_ini(const modernIni::Ini& ini, Type& obj) { MODERN_INI_FOR_EACH(MODERN_INI_FROM, __VA_ARGS__) return {}; }
