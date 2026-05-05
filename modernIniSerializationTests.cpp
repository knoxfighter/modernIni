#include "modernIni.h"
#include "modernIniTestAccessor.h"

#include <gtest/gtest.h>

TEST(ModernIniSerializationTests, SerializeSimple) {
	modernIni::Ini ini;
	auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	children.try_emplace("test", "test", "value", &ini);

	std::ostringstream output;
	output << ini;

	output.flush();
	auto str = output.str();
	ASSERT_EQ(str, "test = value\n");
}

TEST(ModernIniSerializationTests, SerializeCategory) {
	modernIni::Ini ini;

	auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	children.try_emplace("test", "test", "value", &ini);
	auto& cat = children.try_emplace("cat", "cat", &ini).first.operator*().second;
	auto& children2 = ModernIniTestAccessor::getPrivateChildren(cat);
	children2.try_emplace("test2", "test2", "value", &cat);

	std::ostringstream output;
	output << ini;

	output.flush();
	auto str = output.str();
	ASSERT_EQ(str, "test = value\n\n[cat]\ntest2 = value\n");
}

TEST(ModernIniSerializationTests, SerializeMultiple) {
	modernIni::Ini ini;

	auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	children.try_emplace("test", "test", "value", &ini);

	auto& cat = children.try_emplace("cat", "cat", &ini).first.operator*().second;
	auto& children2 = ModernIniTestAccessor::getPrivateChildren(cat);

	auto& tree = children2.try_emplace("tree", "tree", &cat).first.operator*().second;
	auto& children3 = ModernIniTestAccessor::getPrivateChildren(tree);
	children3.try_emplace("test2", "test2", "value", &tree);

	std::ostringstream output;
	output << ini;

	output.flush();
	auto str = output.str();
	ASSERT_EQ(str, "test = value\n\n[cat][tree]\ntest2 = value\n");
}

TEST(ModernIniSerializationTests, SerializeEscaped) {
	modernIni::Ini ini;
	auto& children = ModernIniTestAccessor::getPrivateChildren(ini);
	children.try_emplace("test", "test", "\n;\\", &ini);

	std::ostringstream output;
	output << ini;

	output.flush();
	auto str = output.str();
	ASSERT_EQ(str, "test = \\n\\;\\\\\n");
}

TEST(ModernIniSerializationTests, SerializeEmptyCategory) {
	modernIni::Ini ini;
	ini["cat"]["sub"]["key"] = "value"; // deep nest, no values directly in "cat"
	std::ostringstream os;
	os << ini;
	// "cat" itself has no value children, only a sub-object, so no [cat] header alone
	ASSERT_EQ(os.str(), "\n[cat][sub]\nkey = value\n");
}
