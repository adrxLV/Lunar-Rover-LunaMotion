#pragma once

#include <boost/json.hpp>
#include <string_view>

namespace lrn {

class PotentialFieldNavigatorConfig
{
public:
    static constexpr std::string_view CONFIG_K_ATT = {"K_ATT"};
    static constexpr std::string_view CONFIG_K_REP = {"K_REP"};
    static constexpr std::string_view CONFIG_RHO_0 = {"RHO_0"};
    static constexpr std::string_view CONFIG_K_THETA = {"K_THETA"};
    static constexpr std::string_view CONFIG_V_MAX = {"V_MAX"};
    static constexpr std::string_view CONFIG_W_MAX = {"W_MAX"};

    double K_ATT = 0.1;
    double K_REP = 50.0;
    double RHO_0 = 0.3;
    double K_THETA = 2.0;
    double V_MAX = 0.5;
    double W_MAX = 1.0;
};

PotentialFieldNavigatorConfig tag_invoke( boost::json::value_to_tag< PotentialFieldNavigatorConfig > /*unused*/, boost::json::value const& json_value );

} // namespace lrn