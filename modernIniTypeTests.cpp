#include <gtest/gtest.h>

#include "modernIniMacros.h"

class ServerConfig {
	MODERN_INI_DEFINE_TYPE_INTRUSIVE(ServerConfig, host, port, tls);

public:
	std::string host;
	int port = 0;
	bool tls = false;
};
// MODERN_INI_DEFINE_TYPE(ServerConfig, host, port, tls);

TEST(ModernIniTypeTests, Test) {
	modernIni::Ini ini;
	std::istringstream input("host = localhost\nport = 80\ntls = true");
	input >> ini;

	auto config = ini.get<ServerConfig>();
	ASSERT_TRUE(config);
	ASSERT_EQ(config->host, "localhost");
	ASSERT_EQ(config->port, 80);
	ASSERT_EQ(config->tls, true);

	modernIni::Ini output;
	output = *config;
	ASSERT_EQ(output.at("host").value().get().get<std::string_view>().value(), "localhost");
	ASSERT_EQ(output.at("port").value().get().get<int>().value(), 80);
	ASSERT_EQ(output.at("tls").value().get().get<bool>().value(), true);
}

TEST(ModernIniTypeTests, TestMissing) {
	modernIni::Ini ini;
	std::istringstream input("host = localhost\ntls = true");
	input >> ini;

	auto config = ini.get<ServerConfig>();
	ASSERT_TRUE(config);
	ASSERT_EQ(config->host, "localhost");
	ASSERT_EQ(config->port, 0);
	ASSERT_EQ(config->tls, true);
}
