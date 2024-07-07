#pragma once

#include <memory>

namespace podrm::odbc {

class Environment {
public:
  Environment();

  static Environment fromRaw(void *environment);

  [[nodiscard]] void *getRaw() { return this->impl.get(); }

private:
  std::unique_ptr<void, void (*)(void *)> impl;

  Environment(void *environment);
};

} // namespace podrm::odbc
