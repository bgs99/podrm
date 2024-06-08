#include "../detail/multilambda.hpp"

#include <podrm/reflection.hpp>
#include <podrm/span.hpp>
#include <podrm/sqlite/detail/cursor.hpp>
#include <podrm/sqlite/detail/result.hpp>
#include <podrm/sqlite/detail/row.hpp>

#include <cassert>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

namespace podrm::sqlite::detail {

namespace {

void init(const FieldDescription description, const Row row, int &currentColumn,
          void *field) {
  const auto initPrimitive = [field, row, &currentColumn](
                                 const PrimitiveFieldDescription description) {
    switch (description.imageType) {
    case ImageType::Int:
      description.fromImage(row.get(currentColumn).bigint(), field);
      ++currentColumn;
      return;
    case ImageType::Uint:
      description.fromImage(
          static_cast<std::uint64_t>(row.get(currentColumn).bigint()), field);
      ++currentColumn;
      return;
    case ImageType::String:
      description.fromImage(row.get(currentColumn).text(), field);
      ++currentColumn;
      return;
    case ImageType::Float:
      description.fromImage(row.get(currentColumn).real(), field);
      ++currentColumn;
      return;
    case ImageType::Bool:
      description.fromImage(row.get(currentColumn).boolean(), field);
      ++currentColumn;
      return;
    case ImageType::Bytes:
      description.fromImage(row.get(currentColumn).bytes(), field);
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

Cursor::Cursor(Result result, const span<const FieldDescription> description)
    : result(std::move(result)), description(description) {}

bool Cursor::extract(void *data) const {
  std::optional<Row> row = this->result.getRow();
  if (!row.has_value()) {
    return false;
  }

  int currentColumn = 0;
  for (const FieldDescription field : description) {
    init(field, row.value(), currentColumn, field.memberPtr(data));
  }

  return true;
}

bool Cursor::nextRow() { return this->result.nextRow(); }

bool Cursor::valid() const { return this->result.valid(); }

} // namespace podrm::sqlite::detail
