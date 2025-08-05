#include <liblrn/rover-executor.hpp>
#include <liblrn/rover.hpp>
#include <liblrn/potential-field-navigator.hpp>
#include <liblrn/rover-config.hpp>
#include <liblrn/potential-field-navigator-config.hpp>
#include <fmt/format.h>
#include <chrono>

namespace lrn {

void RoverExecutor::run(RoverConfig &rover_config, PotentialFieldNavigatorConfig &navigator_config) {
    Rover rover;
    rover.init(rover_config);
    PotentialFieldNavigator navigator(rover, navigator_config);

    while (!should_stop_) {
        using namespace std::chrono_literals;
        navigator.step();
        // Add dt sleep if running in real-time
        std::this_thread::sleep_for(50ms);
    }
}

void RoverExecutor::stop() {
    should_stop_ = true;
}
}