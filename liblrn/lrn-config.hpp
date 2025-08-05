#pragma once

#include <string_view>
#include <boost/json.hpp>

#include <liblrn/rover-config.hpp>
#include <liblrn/potential-field-navigator-config.hpp>

namespace lrn {

class LRnConfig
{
public:
    static constexpr std::string_view CONFIG_ROVER = {"rover"};
    static constexpr std::string_view CONFIG_NAVIGATOR = {"navigator"};

    RoverConfig rover;
    PotentialFieldNavigatorConfig navigator;

};

LRnConfig tag_invoke( boost::json::value_to_tag< LRnConfig > /*unused*/, boost::json::value const& json_value );

} // namespace lrn