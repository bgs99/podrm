#pragma once

#include <podrm/odbc/detail/row.hpp>

#include <memory>
#include <optional>

namespace podrm::odbc::detail {

class Result {
public:
  [[nodiscard]] std::optional<Row> getRow() const {
    if (!this->statement.has_value()) {
      return std::nullopt;
    }
    return Row{statement->get(), this->columnCount};
  }

  bool nextRow();

  [[nodiscard]] bool valid() const;

  [[nodiscard]] int getColumnCount() const { return this->columnCount; }

private:
  using Statement = std::unique_ptr<void, void (*)(void *)>;

  std::optional<Statement> statement;

  int columnCount = 0;

  friend class Connection;

  explicit Result(Statement statement);
};

} // namespace podrm::odbc::detail
