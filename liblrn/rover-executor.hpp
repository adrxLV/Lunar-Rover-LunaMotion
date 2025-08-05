#pragma once

#include <atomic>
#include <liblrn/rover-config.hpp>
#include <liblrn/potential-field-navigator-config.hpp>

namespace lrn {

class RoverExecutor {
public:
    void run(RoverConfig &rover_config, PotentialFieldNavigatorConfig &navigator_config);
    void stop();

private:
    std::atomic<bool> should_stop_ {false};
    RoverConfig rover_config_;
    PotentialFieldNavigatorConfig navigator_config_;
};

} // namespace lrn