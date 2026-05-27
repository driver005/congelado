#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>
import core_config;

TEST_CASE("Config default values") {
    core::config::Config cfg{};

    CHECK(cfg.server.host == "localhost");
    CHECK(cfg.server.port == 8080);
    CHECK(cfg.server.threads == 1);
    CHECK(cfg.server.max_connections == 1024);
    CHECK(cfg.server.timeout_ms == 30'000);
    CHECK(cfg.server.tls.cert == "server.crt");
    CHECK(cfg.server.tls.key == "server.key");
    CHECK(cfg.logger.file == "app.log");
    CHECK(cfg.logger.level == "info");
    CHECK(cfg.plugins.empty());
}
