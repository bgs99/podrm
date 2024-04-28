#include <pfr-orm/detail/escaping.hpp>

#include <fmt/core.h>

namespace pfrorm::detail {

std::string addIdentifierPrefix(const std::string_view prefix,
                                const std::string_view identifier) {

  return fmt::format("{}_{}", prefix, identifier);
}

} // namespace pfrorm::detail
