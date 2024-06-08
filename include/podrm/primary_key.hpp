#pragma once

#include <podrm/api.hpp>

#include <string_view>

#include <boost/pfr/core.hpp>
#include <boost/pfr/core_name.hpp>

namespace podrm {

template <DatabaseEntity T>
using PrimaryKeyType =
    boost::pfr::tuple_element_t<EntityRegistration<T>.id.get(), T>;

template <DatabaseEntity T>
constexpr std::string_view PrimaryKeyName =
    boost::pfr::get_name<EntityRegistration<T>.id.get(), T>();

}; // namespace podrm
