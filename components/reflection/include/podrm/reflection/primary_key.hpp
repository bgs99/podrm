#pragma once

#include <podrm/metadata.hpp>
#include <podrm/reflection/api.hpp>

#include <string_view>

#include <boost/pfr/core.hpp>
#include <boost/pfr/core_name.hpp>

namespace podrm {

template <RegisteredEntity Entity> struct PrimaryKey<Entity> {
  using Type =
      boost::pfr::tuple_element_t<EntityRegistration<Entity>.id.get(), Entity>;

  constexpr static std::string_view Name =
      boost::pfr::get_name<EntityRegistration<Entity>.id.get(), Entity>();
};

}; // namespace podrm
