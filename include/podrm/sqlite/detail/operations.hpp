#pragma once

#include <podrm/definitions.hpp>
#include <podrm/sqlite/utils.hpp>

namespace podrm::sqlite::detail {

void createTable(Connection &connection, const EntityDescription &entity);

bool exists(Connection &connection, const EntityDescription &entity);

void persist(Connection &connection, const EntityDescription &description,
             void *entity);

/// @param[out] result pointer to the result structure, filled if found
bool find(Connection &connection, const EntityDescription &description,
          Value key, void *result);

} // namespace podrm::sqlite::detail
