#include "backward.hpp"

#include <stdio.h>

import std;
import core_logger;
import congelado;
import core_server;

int main() {
    backward::SignalHandling sh;

    auto my_logger = std::make_shared<app::MyCustomFileLogger>("app.log");
    std::string init_response = core::logger::LoggerRegistry::register_logger(my_logger);

    core::logger::info("LoggerRegistry", "Logger ready. Handshake: {}", init_response);


    // core::logger::error("Critical Failure in tasks: {}", std::views::iota(1, 5)); // Passing a range view

    app::Server server{};

    std::promise<void>().get_future().wait();
    //
    return 0;
}
