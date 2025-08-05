#pragma once

#include <string_view>
#include <optional>

#include <boost/json.hpp>
#include <liblrn/rover-platform-config.hpp>
#include <liblrn/rover-remote-config.hpp>

namespace lrn {

class RoverConfig
{
public:
    static constexpr std::string_view CONFIG_WHEELBASE = {"wheelbase"};
    static constexpr std::string_view CONFIG_WHEEL_RADIUS = {"wheel_radius"};
    static constexpr std::string_view CONFIG_PLATFORM = {"platform"};
    static constexpr std::string_view CONFIG_REMOTE_API = {"remote_api"};

    double wheelbase;
    double wheel_radius;
    RoverPlatformConfig platform;
    std::optional<RoverRemoteConfig> remote;
};

RoverConfig tag_invoke( boost::json::value_to_tag< RoverConfig > /*unused*/, boost::json::value const& json_value );

} // namespace lrn