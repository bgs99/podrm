#pragma once

#include <cstdint>
#include <string>
#include <string_view>

struct pg_conn;
struct pg_result;

namespace podrm::postgres {

class Str {
public:
  Str(char *str) : str(str) {}
  ~Str();

  [[nodiscard]] std::string_view view() const { return str; }
  operator std::string_view() const { return str; }

  Str(const Str &) = delete;
  Str(Str &&) = delete;
  Str &operator=(const Str &) = delete;
  Str &operator=(Str &&) = delete;

private:
  char *str;
};

class Result {
public:
  Result(pg_result *result) : result(result) {}
  ~Result();

  [[nodiscard]] int status() const;
  [[nodiscard]] std::string_view value(int row, int column) const;

  Result(const Result &) = delete;
  Result(Result &&) noexcept;
  Result &operator=(const Result &) = delete;
  Result &operator=(Result &&) = delete;

private:
  pg_result *result;
};

class Connection {
public:
  Connection(const std::string &connectionStr);
  ~Connection();

  Connection(const Connection &) = delete;
  Connection(Connection &&) noexcept;
  Connection &operator=(const Connection &) = delete;
  Connection &operator=(Connection &&) = delete;

  [[nodiscard]] Str escapeIdentifier(std::string_view identifier) const;

  Result execute(const std::string &statement);

  Result query(const std::string &statement);

private:
  pg_conn *connection;
};

struct ParameterTraits {
  struct Parameter {
    std::string data;
    bool isBinary; // TODO: pass everything as binary
  };

  template <typename T> static Parameter toParam(const T &value) = delete;
};

template <>
inline ParameterTraits::Parameter
ParameterTraits::toParam(const int64_t &value) {
  return {
      .data = std::to_string(value),
      .isBinary = false,
  };
}

template <>
inline ParameterTraits::Parameter
ParameterTraits::toParam(const std::string &value) {
  return {
      .data = value,
      .isBinary = true,
  };
}

template <typename T>
concept AsParameter = requires { ParameterTraits::toParam<T>; };

} // namespace podrm::postgres
