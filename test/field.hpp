#pragma once

#include <pfr-orm/api.hpp>

namespace pfrorm::test {

template <typename T, const auto MemberPtr>
constexpr auto Field =
#ifdef PFR_TEST_USE_FIELD_OF
    ::pfrorm::FieldOf<T, MemberPtr>;
#else
    ::pfrorm::Field<MemberPtr>;
#endif

} // namespace pfrorm::test
