#pragma once

#include <boost/json.hpp>

namespace lrn {

// This helper function deduces the type and assigns the value with the matching key
template<class T>
void extract( boost::json::object const& obj, T& result, boost::json::string_view key )
{
    result = boost::json::value_to<T>( obj.at( key ) );
}

template<class T>
void extract_optional( boost::json::object const& obj, T& result, boost::json::string_view key )
{
    if(obj.contains(key)) {
        extract(obj, result, key);
    }
}

} // namespace lrn