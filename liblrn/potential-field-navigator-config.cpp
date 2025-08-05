#include <liblrn/potential-field-navigator-config.hpp>
#include <liblrn/json-extract.hpp>

#include <boost/json/value.hpp>
#include <boost/json/object.hpp>
#include <boost/json/conversion.hpp>
#include <string_view>

namespace lrn {

PotentialFieldNavigatorConfig tag_invoke( boost::json::value_to_tag< PotentialFieldNavigatorConfig > /*unused*/, boost::json::value const& json_value )
{
    boost::json::object const& obj = json_value.as_object();
    PotentialFieldNavigatorConfig config;
    extract( obj, config.K_ATT, PotentialFieldNavigatorConfig::CONFIG_K_ATT);
    extract( obj, config.K_REP, PotentialFieldNavigatorConfig::CONFIG_K_REP);
    extract( obj, config.RHO_0, PotentialFieldNavigatorConfig::CONFIG_RHO_0);
    extract( obj, config.K_THETA, PotentialFieldNavigatorConfig::CONFIG_K_THETA);
    extract( obj, config.V_MAX, PotentialFieldNavigatorConfig::CONFIG_V_MAX);
    extract( obj, config.W_MAX, PotentialFieldNavigatorConfig::CONFIG_W_MAX);

    return config;
}

} // namespace lrn