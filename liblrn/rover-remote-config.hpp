#pragma once

#include <string_view>
#include <boost/json.hpp>

namespace lrn {

class RoverRemoteConfig
{
public:
    static constexpr std::string_view CONFIG_PORTNAME = {"port"};  
    static constexpr std::string_view CONFIG_ENABLED = {"enabled"};   

    bool enabled = true;
    int port = 5555;
};

RoverRemoteConfig tag_invoke( boost::json::value_to_tag< RoverRemoteConfig > /*unused*/, boost::json::value const& json_value );

} // namespace lrn