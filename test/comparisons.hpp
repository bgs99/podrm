#pragma once

#include "podrm/definitions.hpp"

#include <cstddef>

namespace podrm {

constexpr bool operator==(const PrimitiveFieldDescription lhs,
                          const PrimitiveFieldDescription rhs) {
  return lhs.nativeType == rhs.nativeType;
}

constexpr bool operator==(FieldDescription lhs, FieldDescription rhs);

constexpr bool operator==(const CompositeFieldDescription lhs,
                          const CompositeFieldDescription rhs) {
  if (lhs.fields.size() != rhs.fields.size()) {
    return false;
  }

  for (std::size_t i = 0; i < lhs.fields.size(); ++i) {
    if (lhs.fields[i] != rhs.fields[i]) {
      return false;
    }
  }

  return true;
}

constexpr bool operator==(const FieldDescription lhs,
                          const FieldDescription rhs) {
  return lhs.name == rhs.name && lhs.field == rhs.field;
};

constexpr bool operator==(const EntityDescription lhs,
                          const EntityDescription rhs) {
  if (lhs.name != rhs.name) {
    return false;
  }

  if (lhs.primaryKey != rhs.primaryKey) {
    return false;
  }

  if (lhs.fields.size() != rhs.fields.size()) {
    return false;
  }

  for (std::size_t i = 0; i < lhs.fields.size(); ++i) {
    if (lhs.fields[i] != rhs.fields[i]) {
      return false;
    }
  }

  return true;
}

} // namespace podrm
