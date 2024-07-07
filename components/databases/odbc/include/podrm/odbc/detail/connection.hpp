#pragma once

#include "podrm/odbc/environment.hpp"

#include <podrm/metadata.hpp>
#include <podrm/odbc/detail/cursor.hpp>
#include <podrm/odbc/detail/result.hpp>
#include <podrm/span.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

namespace podrm::odbc::detail {

class Connection {
public:
  //---------------- Constructors ------------------//

  static Connection fromRaw(void *connection);

  static Connection fromConnectionString(Environment &environment,
                                         std::string_view connectionString);

  //---------------- Operations ------------------//

  void createTable(const EntityDescription &entity);

  bool exists(const EntityDescription &entity);

  void persist(const EntityDescription &description, void *entity);

  /// @param[out] result pointer to the result structure, filled if found
  bool find(const EntityDescription &description, const AsImage &key,
            void *result);

  void erase(EntityDescription description, const AsImage &key);

  void update(EntityDescription description, const void *entity);

  Cursor iterate(EntityDescription description);

private:
  std::unique_ptr<void, void (*)(void *)> connection;

  std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();

  explicit Connection(void *connection);

  /// @returns number of affected entries
  std::uint64_t execute(std::string_view statement,
                        span<const AsImage> args = {});

  Result query(std::string_view statement, span<const AsImage> args = {});
};

} // namespace podrm::odbc::detail
