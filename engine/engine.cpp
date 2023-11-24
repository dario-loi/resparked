#include "loguru.hpp"
#include "resparked.hpp"
#include <iostream>

auto main(int argc, char** argv) -> int
{

    loguru::init(argc, argv);
    loguru::add_file("resparked_engine.log", loguru::Truncate, loguru::Verbosity_INFO);

    resparked::engine engine;

    engine.run();

    return EXIT_SUCCESS;
}