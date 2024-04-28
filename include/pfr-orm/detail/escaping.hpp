#pragma once

#include <string>
#include <string_view>

namespace pfrorm::detail {

std::string addIdentifierPrefix(std::string_view prefix,
                                std::string_view identifier);

}
