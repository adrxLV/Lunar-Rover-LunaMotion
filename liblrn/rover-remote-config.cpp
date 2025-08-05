#include <liblrn/rover-remote-config.hpp>
#include <liblrn/json-extract.hpp>

#include <boost/json/value.hpp>
#include <boost/json/object.hpp>
#include <boost/json/conversion.hpp>
#include <string_view>

namespace lrn {

RoverRemoteConfig tag_invoke( boost::json::value_to_tag< RoverRemoteConfig > /*unused*/, boost::json::value const& json_value )
{
    boost::json::object const& obj = json_value.as_object();
    RoverRemoteConfig config;
    extract( obj, config.enabled, RoverRemoteConfig::CONFIG_ENABLED);
    extract( obj, config.port, RoverRemoteConfig::CONFIG_PORTNAME);

    return config;
}

} // namespace lrn