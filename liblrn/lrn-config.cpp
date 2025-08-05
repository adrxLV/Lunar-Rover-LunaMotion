#include <liblrn/lrn-config.hpp>
#include <liblrn/json-extract.hpp>

#include <boost/json/value.hpp>
#include <boost/json/object.hpp>
#include <boost/json/conversion.hpp>

namespace lrn {

LRnConfig tag_invoke( boost::json::value_to_tag< LRnConfig > /*unused*/, boost::json::value const& json_value )
{
    boost::json::object const& obj = json_value.as_object();
    LRnConfig config;
    extract( obj, config.rover, LRnConfig::CONFIG_ROVER );
    extract( obj, config.navigator, LRnConfig::CONFIG_NAVIGATOR );

    return config;
}

} // namespace lrn