#include <liblrn/rover-config.hpp>
#include <liblrn/json-extract.hpp>

#include <boost/json/value.hpp>
#include <boost/json/object.hpp>
#include <boost/json/conversion.hpp>
#include <string_view>

namespace lrn {

RoverConfig tag_invoke( boost::json::value_to_tag< RoverConfig > /*unused*/, boost::json::value const& json_value )
{
    boost::json::object const& obj = json_value.as_object();
    RoverConfig config;
    extract( obj, config.wheelbase, RoverConfig::CONFIG_WHEELBASE);
    extract( obj, config.wheel_radius, RoverConfig::CONFIG_WHEEL_RADIUS);
    extract( obj, config.platform, RoverConfig::CONFIG_PLATFORM);
    extract_optional( obj, config.remote, RoverConfig::CONFIG_REMOTE_API);

    return config;
}

} // namespace lrn