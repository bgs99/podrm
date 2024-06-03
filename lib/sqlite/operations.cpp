#include "../detail/multilambda.hpp"

#include <podrm/api.hpp>
#include <podrm/definitions.hpp>
#include <podrm/detail/span.hpp>
#include <podrm/sqlite/detail/operations.hpp>
#include <podrm/sqlite/utils.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>

namespace podrm::sqlite::detail {

namespace {

std::string_view toString(const NativeType type) {
  switch (type) {
  case NativeType::BigInt:
    return "INTEGER";
  case NativeType::String:
    return "TEXT";
  }
  throw std::runtime_error{
      "Unsupported native type " + std::to_string(static_cast<int>(type)),
  };
}

void createTableFields(const FieldDescription &description,
                       const bool isPrimaryKey, fmt::appender &appender,
                       std::vector<std::string_view> prefixes, bool &first) {
  prefixes.push_back(description.name);

  const auto createPrimitiveField =
      [isPrimaryKey, &prefixes, &appender,
       &first](const PrimitiveFieldDescription &descr) {
        fmt::format_to(appender, "{}'{}' '{}'{}", first ? "" : ",",
                       fmt::to_string(fmt::join(prefixes, "_")),
                       toString(descr.nativeType),
                       isPrimaryKey ? " PRIMARY KEY" : "");
        first = false;
      };

  const auto createCompositeField =
      [&prefixes, &appender, &first](const CompositeFieldDescription &descr) {
        for (const FieldDescription &field : descr.fields) {
          createTableFields(field, false, appender, prefixes, first);
        }
      };

  const podrm::detail::MultiLambda createField{
      createPrimitiveField,
      createCompositeField,
  };

  static_assert(std::is_invocable_r_v<void, decltype(createField),
                                      const PrimitiveFieldDescription &>);
  static_assert(std::is_invocable_r_v<void, decltype(createField),
                                      const CompositeFieldDescription &>);

  std::visit(createField, description.field);
}

void createParamPlaceholders(const FieldDescription description,
                             fmt::appender &appender, bool &first) {
  const auto createPrimitive = [&appender,
                                &first](const PrimitiveFieldDescription) {
    fmt::format_to(appender, "{}?", first ? "" : ",");
    first = false;
  };

  const auto createComposite = [&appender,
                                &first](const CompositeFieldDescription descr) {
    for (const FieldDescription field : descr.fields) {
      createParamPlaceholders(field, appender, first);
    }
  };

  std::visit(podrm::detail::MultiLambda{createPrimitive, createComposite},
             description.field);
}

std::vector<Value> intoArgs(const FieldDescription description, void *field) {
  const auto createPrimitive =
      [field](
          const PrimitiveFieldDescription description) -> std::vector<Value> {
    switch (description.nativeType) {
    case NativeType::BigInt:
      return {*static_cast<const std::int64_t *>(field)};
    case NativeType::String:
      return {*static_cast<const std::string *>(field)};
    }
    assert(false);
  };

  const auto createComposite =
      [field](const CompositeFieldDescription descr) -> std::vector<Value> {
    std::vector<Value> values;
    for (const FieldDescription fieldDescr : descr.fields) {
      for (const Value value :
           intoArgs(fieldDescr, fieldDescr.memberPtr(field))) {
        values.emplace_back(value);
      }
    }

    return values;
  };

  return std::visit(
      podrm::detail::MultiLambda{createPrimitive, createComposite},
      description.field);
}

void init(const FieldDescription description, const Row row, int &currentColumn,
          void *field) {
  const auto initPrimitive = [field, row, &currentColumn](
                                 const PrimitiveFieldDescription description) {
    switch (description.nativeType) {
    case NativeType::BigInt:
      *static_cast<std::int64_t *>(field) = row.get(currentColumn).bigint();
      ++currentColumn;
      return;
    case NativeType::String:
      *static_cast<std::string *>(field) =
          static_cast<std::string>(row.get(currentColumn).text());
      ++currentColumn;
      return;
    }
    assert(false);
  };

  const auto initComposite = [field, row, &currentColumn](
                                 const CompositeFieldDescription description) {
    for (const FieldDescription fieldDescr : description.fields) {
      init(fieldDescr, row, currentColumn, fieldDescr.memberPtr(field));
    }
  };

  std::visit(podrm::detail::MultiLambda{initPrimitive, initComposite},
             description.field);
}

} // namespace

void createTable(Connection &connection, const EntityDescription &entity) {
  connection.execute(fmt::format("DROP TABLE IF EXISTS '{}'", entity.name));

  fmt::memory_buffer buf;
  fmt::appender appender{buf};
  fmt::format_to(appender, "CREATE TABLE '{}' (", entity.name);
  bool first = true;
  for (std::size_t i = 0; i < entity.fields.size(); ++i) {
    createTableFields(entity.fields[i], entity.primaryKey == i, appender, {},
                      first);
  }
  fmt::format_to(appender, ")");
  connection.execute(fmt::to_string(buf));
}

bool exists(Connection &connection, const EntityDescription &entity) {
  const Result result = connection.query(
      fmt::format("SELECT EXISTS(SELECT 1 FROM '{}')", entity.name));
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access): fixed query
  return result.getRow().value().get(0).boolean();
}

void persist(Connection &connection, const EntityDescription &description,
             void *entity) {
  fmt::memory_buffer buf;
  fmt::appender appender{buf};

  fmt::format_to(appender, "INSERT INTO '{}' VALUES (", description.name);

  bool first = true;
  std::vector<Value> values;
  for (std::size_t i = 0; i < description.fields.size(); ++i) {
    if (description.idMode == IdMode::Auto && i == description.primaryKey) {
      // TODO: support auto ids
    }
    createParamPlaceholders(description.fields[i], appender, first);

    for (const Value value : intoArgs(
             description.fields[i], description.fields[i].memberPtr(entity))) {
      values.emplace_back(value);
    }
  }
  fmt::format_to(appender, ")");

  connection.execute(fmt::to_string(buf), values);
}

bool find(Connection &connection, const EntityDescription &description,
          const Value key, void *result) {
  const std::string queryStr =
      fmt::format("SELECT * FROM '{}' WHERE {} = ?", description.name,
                  description.fields[description.primaryKey].name);

  const Result query =
      connection.query(queryStr, podrm::detail::span<const Value, 1>{&key, 1});
  std::optional<Row> row = query.getRow();
  if (!row.has_value()) {
    return false;
  }

  int currentColumn = 0;
  for (const FieldDescription description : description.fields) {
    init(description, row.value(), currentColumn,
         description.memberPtr(result));
  }

  return true;
}

} // namespace podrm::sqlite::detail
