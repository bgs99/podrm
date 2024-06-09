#pragma once

#include <podrm/api.hpp>
#include <podrm/reflection.hpp>
#include <podrm/span.hpp>
#include <podrm/sqlite/detail/cursor.hpp>
#include <podrm/sqlite/detail/result.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string_view>

struct sqlite3;

namespace podrm::sqlite::detail {

class Connection {
public:
  //---------------- Constructors ------------------//

  static Connection fromRaw(sqlite3 &connection);

  static Connection inMemory(const char *name);

  static Connection inFile(const std::filesystem::path &path);

  //---------------- Operations ------------------//

  void createTable(const EntityDescription &entity);

  bool exists(const EntityDescription &entity);

  void persist(const EntityDescription &description, void *entity);

  /// @param[out] result pointer to the result structure, filled if found
  bool find(const EntityDescription &description, const AsImage &key,
            void *result);

  void erase(EntityDescription description, const AsImage &key);

  void update(EntityDescription description, const void *entity);

  Cursor iterate(EntityDescription description, span<const std::size_t> fields,
                 span<const AsImage> filters);

private:
  std::unique_ptr<sqlite3, int (*)(sqlite3 *)> connection;

  std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();

  explicit Connection(sqlite3 &connection);

  /// @returns number of affected entries
  std::uint64_t execute(std::string_view statement,
                        span<const AsImage> args = {});

  Result query(std::string_view statement, span<const AsImage> args = {});
};

} // namespace podrm::sqlite::detail
