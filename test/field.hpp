#pragma once

#include <podrm/api.hpp>

namespace podrm::test {

template <typename T, const auto MemberPtr>
constexpr auto Field =
#ifdef PFR_TEST_USE_FIELD_OF
    ::podrm::FieldOf<T, MemberPtr>;
#else
    ::podrm::Field<MemberPtr>;
#endif

} // namespace podrm::test
