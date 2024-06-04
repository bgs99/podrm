#pragma once

#include <podrm/api.hpp>
#include <podrm/definitions.hpp>
#include <podrm/sqlite/utils.hpp>

namespace podrm::sqlite::detail {

void createTable(Connection &connection, const EntityDescription &entity);

bool exists(Connection &connection, const EntityDescription &entity);

void persist(Connection &connection, const EntityDescription &description,
             void *entity);

/// @param[out] result pointer to the result structure, filled if found
bool find(Connection &connection, const EntityDescription &description,
          const AsImage &key, void *result);

void erase(Connection &connection, EntityDescription description,
           const AsImage &key);

void update(Connection &connection, EntityDescription description,
            const void *entity);

} // namespace podrm::sqlite::detail
