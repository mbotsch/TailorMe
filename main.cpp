#include <argparse/argparse.hpp>
#include <fmt/core.h>

#include "src/Constants.h"
#include "src/Globals.h"
#include "src/TailorMeViewer.h"

auto main(int argc, const char* argv[]) -> int {
    // parse arguments
    argparse::ArgumentParser program("TailorMe Viewer", "0.3.0");

    program.add_argument("--models")
        .default_value<std::string>(
            fmt::format("{}{}{}",
                        std::filesystem::current_path().c_str(),
                        std::filesystem::path::preferred_separator,
                        MODEL_DATA_DIR
            )
        ).help("Sets of model files. Should not end with '/'.");
    program.add_argument("--data")
        .default_value<std::string>(
            fmt::format("{}{}{}",
                        std::filesystem::current_path().c_str(),
                        std::filesystem::path::preferred_separator,
                        RESOURCE_DATA_DIR
            )
        );

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << '\n';
        std::cerr << program << '\n';
        std::exit(1);
    }

    globals::model_dir = program.get("models");
    globals::data_dir = program.get("data");
    std::cout << "Model directory: " << globals::model_dir << '\n';


    // create main window
    TailorMeViewer window("TailorMe Viewer", 1400, 900);
    return window.run();
}
