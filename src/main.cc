#include "backward.hpp"

import std;
import core_heart;

int main(int argc, char *argv[]) {
    backward::SignalHandling sh;

    auto plugin_dir = argc > 0 ? std::filesystem::path(argv[0]).parent_path() : std::filesystem::path{};

    return core::heart::App{plugin_dir}.run("~/cc/congelado/src/congelado.toml");
}
