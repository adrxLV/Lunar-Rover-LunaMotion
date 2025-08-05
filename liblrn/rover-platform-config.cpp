#include <liblrn/rover-platform-config.hpp>
#include <liblrn/json-extract.hpp>

#include <boost/json/value.hpp>
#include <boost/json/object.hpp>
#include <boost/json/conversion.hpp>
#include <string_view>

namespace lrn {

RoverPlatformConfig tag_invoke( boost::json::value_to_tag< RoverPlatformConfig > /*unused*/, boost::json::value const& json_value )
{
    boost::json::object const& obj = json_value.as_object();
    RoverPlatformConfig config;
    extract( obj, config.port, RoverPlatformConfig::CONFIG_PORTNAME);

    return config;
}

} // namespace lrn