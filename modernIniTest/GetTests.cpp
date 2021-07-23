/**
 * This file is a copy of getToTests!
 * Only thing changed here, is the funtion called (`get` instead of `get_to`)
 * Make sure, the two testfiles have the same tests.
 *
 * Slight changes are added, cause default values of definition is used.
 */
#include "pch.h"

#include <filesystem>
#include <fstream>

#include "../modernIni/modernIniMacros.h"

import modernIni;

using std::string_literals::operator ""s;

typedef modernIni::Ini Ini;
typedef std::map<std::string, Ini> IniMap;

namespace {

	template <typename T>
	void test(Ini& ini, std::string key, T checkVal) {
		T val = ini.at(key).get<T>();
		ASSERT_EQ(val, checkVal) << " failed with key: " << key;
	};

	TEST(getTests, Nums) {
		std::string iniString = R"(
a=5
b=1.5
c=-10
d=-5.1
e=tätärä
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		// number
		test(ini, "a", static_cast<int8_t>(5));
		test(ini, "a", static_cast<int16_t>(5));
		test(ini, "a", static_cast<int32_t>(5));
		test(ini, "a", static_cast<int64_t>(5));
		test(ini, "a", static_cast<uint8_t>(5));
		test(ini, "a", static_cast<uint16_t>(5));
		test(ini, "a", static_cast<uint32_t>(5));
		test(ini, "a", static_cast<uint64_t>(5));
		test(ini, "a", 5.f); // float
		test(ini, "a", static_cast<double>(5)); // double

		// float
		test(ini, "b", static_cast<int8_t>(1));
		test(ini, "b", static_cast<int16_t>(1));
		test(ini, "b", static_cast<int32_t>(1));
		test(ini, "b", static_cast<int64_t>(1));
		test(ini, "b", static_cast<uint8_t>(1));
		test(ini, "b", static_cast<uint16_t>(1));
		test(ini, "b", static_cast<uint32_t>(1));
		test(ini, "b", static_cast<uint64_t>(1));
		test(ini, "b", 1.5f); // float
		test(ini, "b", static_cast<double>(1.5)); // double

		// negative int
		test(ini, "c", static_cast<int8_t>(-10));
		test(ini, "c", static_cast<int16_t>(-10));
		test(ini, "c", static_cast<int32_t>(-10));
		test(ini, "c", static_cast<int64_t>(-10));
		test(ini, "c", static_cast<uint8_t>(0)); // fail to parse
		test(ini, "c", static_cast<uint16_t>(0)); // fail to parse
		test(ini, "c", static_cast<uint32_t>(0)); // fail to parse
		test(ini, "c", static_cast<uint64_t>(0)); // fail to parse
		test(ini, "c", -10.f); // float
		test(ini, "c", static_cast<double>(-10)); // double

		// negative float
		test(ini, "d", static_cast<int8_t>(-5));
		test(ini, "d", static_cast<int16_t>(-5));
		test(ini, "d", static_cast<int32_t>(-5));
		test(ini, "d", static_cast<int64_t>(-5));
		test(ini, "d", static_cast<uint8_t>(0)); // fail to parse
		test(ini, "d", static_cast<uint16_t>(0)); // fail to parse
		test(ini, "d", static_cast<uint32_t>(0)); // fail to parse
		test(ini, "d", static_cast<uint64_t>(0)); // fail to parse
		test(ini, "d", -5.1f); // float
		test(ini, "d", static_cast<double>(-5.1)); // double

		// invalid
		test(ini, "e", static_cast<int8_t>(0));
		test(ini, "e", static_cast<int16_t>(0));
		test(ini, "e", static_cast<int32_t>(0));
		test(ini, "e", static_cast<int64_t>(0));
		test(ini, "e", static_cast<uint8_t>(0));
		test(ini, "e", static_cast<uint16_t>(0));
		test(ini, "e", static_cast<uint32_t>(0));
		test(ini, "e", static_cast<uint64_t>(0));
		test(ini, "e", 0.f); // float
		test(ini, "e", static_cast<double>(0)); // double
	}

	TEST(getTests, String) {
		std::string iniString = R"(
a=5
b=1.5
c=-10
d=-5.1
e=tätärä
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		test(ini, "a", "5"s);
		test(ini, "b", "1.5"s);
		test(ini, "c", "-10"s);
		test(ini, "d", "-5.1"s);
		test(ini, "e", "tätärä"s);
	}

	TEST(getToTests, StringNewLine) {
		std::string iniString = R"(
e=tä\ntärä
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;
		test(ini, "e", "tä\ntärä"s);
	}

	TEST(getTests, boolean) {
		std::string iniString = R"(
a=true
b=TRUE
c=True
d=on
e=1
q=false
r=FALSE
s=False
t=off
u=0
x=invalid
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		test(ini, "a", true);
		test(ini, "b", true);
		test(ini, "c", true);
		test(ini, "d", true);
		test(ini, "e", true);
		test(ini, "q", false);
		test(ini, "r", false);
		test(ini, "s", false);
		test(ini, "t", false);
		test(ini, "u", false);
		test(ini, "x", false);
	}

	TEST(getTests, enumNum) {
		std::string iniString = R"(
a=0
b=5
c=3
x=invalid
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		enum class EnumTest {
			T0,
			T1,
			T2,
			T3,
			T4,
			T5
		};

		test(ini, "a", EnumTest::T0);
		test(ini, "b", EnumTest::T5);
		test(ini, "c", EnumTest::T3);
		test(ini, "x", EnumTest::T0);
	}

	enum class EnumTest2 {
		T0,
		T1,
		T2,
		T3,
		T4,
		T5
	};

	MODERN_INI_SERIALIZE_ENUM(EnumTest2, T0, T1, T2, T3, T4, T5)

	TEST(getTests, enumName) {
		std::string iniString = R"(
a=T0
b=T5
c=T3
x=invalid
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		test(ini, "a", EnumTest2::T0);
		test(ini, "b", EnumTest2::T5);
		test(ini, "c", EnumTest2::T3);
		test(ini, "x", EnumTest2::T0);
	}

	struct object {
		std::string a;
		float b;
		int32_t c;
		double d;
		std::string e;

		bool operator==(const object& other) const {
			return a == other.a && b == other.b && c == other.c && d == other.d && e == other.e;
		}
	};

	MODERN_INI_DEFINE_TYPE_NON_INTRUSIVE(object, a, b, c, d, e)

	struct objectNonThrowing {
		std::string a;
		float b = 5.f;
		int32_t c = 32;
		double d = -5;
		std::string e;

		bool operator==(const objectNonThrowing& other) const {
			return a == other.a && b == other.b && c == other.c && d == other.d && e == other.e;
		}
	};

	MODERN_INI_DEFINE_TYPE_NON_INTRUSIVE_NO_EXCEPT(objectNonThrowing, a, b, c, d, e)

	struct objectIntrusive {
		std::string v;
		float w;
		int32_t x;
		double y;
		std::string z;

		bool operator==(const objectIntrusive& other) const {
			return v == other.v && w == other.w && x == other.x && y == other.y && z == other.z;
		}

		MODERN_INI_DEFINE_TYPE_INTRUSIVE(objectIntrusive, v, w, x, y, z)
	};

	struct objectIntrusiveNonThrowing {
		std::string v;
		float w = 5.f;
		int32_t x = 32;
		double y = -5.f;
		std::string z;

		bool operator==(const objectIntrusiveNonThrowing& other) const {
			return v == other.v && w == other.w && x == other.x && y == other.y && z == other.z;
		}

		MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(objectIntrusiveNonThrowing, v, w, x, y, z)
	};

	TEST(getTests, NonIntrusiveStruct) {
		object objTest{
			"5",
			1.5f,
			-10,
			-5.1,
			"tätärä"
		};

		std::string iniString = R"(
a=5
b=1.5
c=-10
d=-5.1
e=tätärä
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		auto obj = ini.get<object>();

		ASSERT_EQ(obj, objTest);
	}

	TEST(getTests, IntrusiveStruct) {
		objectIntrusive objTest{
			"7",
			3.5f,
			-27,
			-12.74,
			"ich trage gern Tütü"
		};

		std::string iniString = R"(
v=7
w=3.5
x=-27
y=-12.74
z=ich trage gern Tütü
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		auto obj = ini.get<objectIntrusive>();

		ASSERT_EQ(obj, objTest);
	}

	TEST(getTests, NonIntrusiveStructMissingValuesException) {
		std::string iniString = "";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		ASSERT_THROW(ini.get<object>(), std::out_of_range);
	}

	TEST(getTests, IntrusiveStructMissingValuesException) {
		objectIntrusive obj{
			"7",
			3.5f,
			-27,
			-12.74,
			"ich trage gern Tütü"
		};

		std::string iniString = "";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		ASSERT_THROW(ini.get<objectIntrusive>(), std::out_of_range);
	}

	TEST(getTests, NonIntrusiveStructMissingValues) {
		objectNonThrowing objTest{
			"",
			5.f,
			-102,
			-5.f,
			""
		};

		std::string iniString = "c=-102";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		auto obj = ini.get<objectNonThrowing>();

		ASSERT_EQ(obj, objTest);
	}

	TEST(getTests, IntrusiveStructMissingValues) {
		objectIntrusiveNonThrowing objTest{
			"",
			5.f,
			-102,
			-5.f,
			""
		};

		std::string iniString = "x=-102";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		auto obj = ini.get<objectIntrusiveNonThrowing>();

		ASSERT_EQ(obj, objTest);
	}

	TEST(getTests, WrongType) {
		objectIntrusiveNonThrowing objTest{
			"",
			5.f,
			32,
			-5,
			""
		};

		std::string iniString = R"(
w=abc
x=def
y=ghi
	)";

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		auto obj = ini.get<objectIntrusiveNonThrowing>();

		ASSERT_EQ(obj, objTest);
	}

	struct sub1 {
		float x = 0.f;
		float y = 0.f;

		MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(sub1, x, y)

		bool operator==(const sub1& rhs) const {
			return x == rhs.x
				&& y == rhs.y;
		}
	};

	struct sub2 {
		int a = 0;
		sub1 b;

		MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(sub2, a, b)

		bool operator==(const sub2& rhs) const {
			return a == rhs.a
				&& b == rhs.b;
		}
	};

	struct sub3 {
		sub2 a;

		MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(sub3, a)

		bool operator==(const sub3& rhs) const {
			return a == rhs.a;
		}
	};

	struct RootElement {
		std::string a = "huhu";
		uint16_t b = 5;
		sub1 c;
		sub3 d;

		MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(RootElement, a, b, c, d)

		bool operator==(const RootElement& rhs) const {
			return a == rhs.a
				&& b == rhs.b
				&& c == rhs.c
				&& d == rhs.d;
		}
	};

	struct RootElement2 {
		std::string a = "huhu";
		uint16_t b = 5;
		sub1 c;
		sub2 d;

		MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(RootElement2, a, b, c, d)

		bool operator==(const RootElement2& rhs) const {
			return a == rhs.a
				&& b == rhs.b
				&& c == rhs.c
				&& d == rhs.d;
		}
	};

	TEST(getTests, MultiObjects) {
		std::string iniString = R"(
b=8

[c]
x=0.5
y=0.685

[d][a]
a=28

[d][a][b]
x=1.75
y=12.385
)";

		RootElement objTest = {
			"huhu",
			8,
			{
				0.5,
				0.685
			},
			{
				28,
				{
					1.75,
					12.385
				}
			}
		};

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		auto obj = ini.get<RootElement>();

		ASSERT_EQ(obj, objTest);
	}

	TEST(getTests, MultiObjectsInvalidConversion) {
		std::string iniString = R"(
b=8

[c]
x=0.5
y=0.685

[d][a]
a=28

[d][a][b]
x=1.75
y=12.385
)";

		RootElement2 objTest = {
			"huhu",
			8,
			{
				0.5,
				0.685
			},
			{
				0,
				{
					0.f,
					0.f
				}
			}
		};

		std::istringstream iniStream(iniString);

		Ini ini;

		iniStream >> ini;

		auto obj = ini.get<RootElement2>();

		ASSERT_EQ(obj, objTest);
	}

	TEST(getTests, Optional) {
		std::optional<std::string> t;
		std::optional<std::string> t2 = "test";

		Ini ini = "test"s;

		ini.get_to(t);

		ASSERT_EQ(t, t2);
	}
}
