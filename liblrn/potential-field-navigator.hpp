#pragma once
#include <liblrn/rover.hpp>
#include <liblrn/potential-field-navigator-config.hpp>

namespace lrn {

class PotentialFieldNavigator {

public:
    explicit PotentialFieldNavigator(Rover& rover, PotentialFieldNavigatorConfig &cfg);
    
    void step();
    
private:
    Rover& rover_;
    PotentialFieldNavigatorConfig& config;
};

}
