#include "pch.h"

#include "../modernIni/modernIniMacros.h"

import modernIni;

typedef modernIni::Ini Ini;
typedef std::map<std::string, Ini> IniMap;

namespace {
	template <typename T>
	void test(T var, const Ini& iniTest) {
		Ini ini(var);
		ASSERT_EQ(ini, iniTest);
	}

	TEST(ConstructTests, Nums) {
		Ini ini("5");

		test(static_cast<int8_t>(5), ini);
		test(static_cast<int16_t>(5), ini);
		test(static_cast<int32_t>(5), ini);
		test(static_cast<int64_t>(5), ini);
		test(static_cast<uint8_t>(5), ini);
		test(static_cast<uint16_t>(5), ini);
		test(static_cast<uint32_t>(5), ini);
		test(static_cast<uint64_t>(5), ini);
		test(5.f, ini); // float
		test(static_cast<double>(5), ini); // double
	}

	TEST(ConstructTests, string) {
		Ini ini("5");

		std::string t("5");

		test(t, ini);
	}

	struct object {
		std::string a;
		float b;
		int32_t c = 32;
		double d = 0.75;
		std::string e;

		bool operator==(const object& other) const {
			return a == other.a && b == other.b && c == other.c && d == other.d && e == other.e;
		}
	};

	MODERN_INI_DEFINE_TYPE_NON_INTRUSIVE(object, a, b, c, d, e)

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

	TEST(ConstructTests, NonIntrusiveStruct) {
		object obj{
			"5",
			1.5f,
			-10,
			-5.1,
			"tätärä"
		};

		Ini iniTest{
			IniMap{
				{"a", Ini("a", "5")},
				{"b", Ini("b", "1.5")},
				{"c", Ini("c", "-10")},
				{"d", Ini("d", "-5.1")},
				{"e", Ini("e", "tätärä")},
			}
		};

		Ini ini;

		ini = obj;

		ASSERT_EQ(ini, iniTest);
	}

	TEST(ConstructTests, IntrusiveStruct) {
		objectIntrusive obj{
			"5",
			1.5f,
			-10,
			-5.1,
			"tätärä"
		};

		Ini iniTest{
			IniMap{
				{"v", Ini("v", "5")},
				{"w", Ini("w", "1.5")},
				{"x", Ini("x", "-10")},
				{"y", Ini("y", "-5.1")},
				{"z", Ini("z", "tätärä")},
			}
		};

		Ini ini;

		ini = obj;

		ASSERT_EQ(ini, iniTest);
	}

	TEST(ConstructTests, NonIntrusiveStructMissingValues) {
		object obj{
			"5",
			1.5f,
			51,
		};

		Ini iniTest{
			IniMap{
				{"a", Ini("a", "5")},
				{"b", Ini("b", "1.5")},
				{"c", Ini("c", "51")},
				{"d", Ini("d", "0.75")},
				{"e", Ini("e", "")},
			}
		};

		Ini ini;

		ini = obj;

		ASSERT_EQ(ini, iniTest);
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

	TEST(ConstructTests, MultiObjects) {
		RootElement obj = {
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

		Ini iniTest{
			IniMap{
				{"a", Ini("a", "huhu")},
				{"b", Ini("b", "8")},
				{
					"c", Ini("c", IniMap{
						         {"x", Ini("x", "0.5")},
						         {"y", Ini("y", "0.685")}
					         })
				},
				{
					"d", Ini("d", IniMap{
						         {
							         "a", Ini("a", IniMap{
								                  {"a", Ini("a", "28")},
								                  {
									                  "b", Ini("b", IniMap{
										                           {"x", Ini("x", "1.75")},
										                           {"y", Ini("y", "12.385")}
									                           })
								                  }
							                  })
						         }
					         })
				}
			}
		};

		Ini ini;

		ini = obj;

		ASSERT_EQ(ini, iniTest);
	}
}
