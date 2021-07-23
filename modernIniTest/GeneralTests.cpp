#include "pch.h"

#include <filesystem>
#include <fstream>

import modernIni;

using std::string_literals::operator ""s;

typedef modernIni::Ini Ini;
typedef std::map<std::string, Ini> IniMap;

namespace {

	Ini testFileIni{
		IniMap{
			{"test1", Ini("test1", "baumhaus")},
			{"test2", Ini("test2", "haus\nbaum")},
			{
				"cat1", Ini("cat1", IniMap{
								{"test1", Ini("test1", "kuckuck")},
								{"test5", Ini("test5", "ich bin ein text")}
							})
			},
			{
				"cat2", Ini("cat2", IniMap{
								{"test1", Ini("test1", "Falke")},
								{
									"subcat1", Ini("subcat1", IniMap{
													   {"x", Ini("x", "15")},
													   {"y", Ini("y", "7")}
												   })
								},
								{
									"subcat2", Ini("subcat2", IniMap{
													   {"x", Ini("x", "2")},
													   {"y", Ini("y", "2")},
													   {"z", Ini("z", "3")}
												   })
								},
								{
									"subcat3", Ini("subcat3", IniMap{
													   {
														   "subsubcat1", Ini("subsubcat1", IniMap{
																				 {"x", Ini("x", "1")},
																				 {"y", Ini("y", "1")}
																			 })
													   }
												   })
								}
							})
			}
		}
	};

	TEST(modernIni, basicRead) {
		auto b = std::filesystem::current_path();
		b.append("test.ini");
		std::ifstream stream(b);
		if (!stream.is_open()) {
			FAIL() << "test.ini not opened";
		}

		Ini ini;

		stream >> ini;

		ASSERT_EQ(ini, testFileIni);
	}

	TEST(modernIni, basicReadSpaces) {
		auto b = std::filesystem::current_path();
		b.append("testSpaces.ini");
		std::ifstream stream(b);
		if (!stream.is_open()) {
			FAIL() << "testSpaces.ini not opened";
		}

		Ini ini;

		stream >> ini;

		ASSERT_EQ(ini, testFileIni);
	}

	TEST(modernIni, basicWrite) {
		auto inFile = std::filesystem::current_path();
		inFile.append("test.ini");

		auto outFile = std::filesystem::current_path();
		outFile.append("testOut.ini");

		{
			// read in test file
			std::ifstream stream(inFile);
			if (!stream.is_open()) {
				FAIL() << "test.ini not opened";
			}

			Ini ini;

			stream >> ini;

			// write out testfile
			// delete previous file
			if (exists(outFile))
				remove(outFile);

			// open file
			std::ofstream ostream(outFile);
			if (!ostream.is_open()) {
				FAIL() << "test.ini not opened";
			}

			ostream << ini;
		}

		std::ifstream t(inFile);
		std::string tFile((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		std::ifstream o(outFile);
		std::string oFile((std::istreambuf_iterator<char>(o)), std::istreambuf_iterator<char>());

		ASSERT_EQ(tFile, oFile);
	}
}
