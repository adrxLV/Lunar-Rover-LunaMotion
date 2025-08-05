#pragma once

#include <string_view>
#include <boost/json.hpp>

namespace lrn {

class RoverPlatformConfig
{
public:
    static constexpr std::string_view CONFIG_PORTNAME = {"port"};    

    std::string port;
};

RoverPlatformConfig tag_invoke( boost::json::value_to_tag< RoverPlatformConfig > /*unused*/, boost::json::value const& json_value );

} // namespace lrn