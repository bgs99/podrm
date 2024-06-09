#pragma once

#include <podrm/api.hpp>
#include <podrm/detail/field.hpp>
#include <podrm/span.hpp>

#include <cstddef>
#include <utility>
#include <vector>

namespace podrm::sqlite {

template <DatabaseEntity Entity> class SelectQuery {
public:
  template <auto Entity::*MemberPtr>
    requires(DatabasePrimitive<podrm::detail::MemberPtrField<MemberPtr>>)
  SelectQuery &&
  where(const podrm::detail::MemberPtrField<MemberPtr> &value) && {
    constexpr static std::size_t Field = FieldOf<Entity, MemberPtr>.get();
    this->filterFields.emplace_back(Field);
    this->filterValues.emplace_back(
        ValueRegistration<podrm::detail::MemberPtrField<MemberPtr>>::asImage(
            value));

    return std::move(*this);
  }

  [[nodiscard]] span<const std::size_t> getFilterFields() const {
    return this->filterFields;
  }
  [[nodiscard]] span<const AsImage> getFilterValues() const {
    return this->filterValues;
  }

private:
  std::vector<std::size_t> filterFields;
  std::vector<AsImage> filterValues;
};

template <DatabaseEntity Entity> SelectQuery<Entity> select() {
  return SelectQuery<Entity>{};
}

} // namespace podrm::sqlite
