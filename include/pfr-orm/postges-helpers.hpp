#pragma once

#include <arpa/inet.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <postgres_ext.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/format.h>
#include <libpq-fe.h>

namespace pfrorm::postgres {

using Deleter = decltype([](void* ptr) { PQfreemem(ptr); });

template<typename T>
using Uptr = std::unique_ptr<T, Deleter>;

using Str = Uptr<char>;

inline Str escapeIdentifier(PGconn& connection, const std::string_view identifier) {
    return Str { PQescapeIdentifier(&connection, identifier.data(), identifier.size()) };
}

using Result = std::unique_ptr<PGresult, decltype([](PGresult* result) { PQclear(result); })>;

struct ParameterTraits {
    struct Parameter {
        std::string data;
        bool isBinary;    // TODO: pass everything as binary
    };

    template<typename T>
    static Parameter toParam(const T& value) = delete;
};

template<>
inline ParameterTraits::Parameter ParameterTraits::toParam(const uint64_t& value) {
    return {
        .data = std::to_string(value),
        .isBinary = false,
    };
}

template<>
inline ParameterTraits::Parameter ParameterTraits::toParam(const std::string& value) {
    return {
        .data = value,
        .isBinary = true,
    };
}

template<typename T>
concept AsParameter = requires { ParameterTraits::toParam<T>; };

static_assert(not AsParameter<std::function<void()>>);
static_assert(AsParameter<std::string>);

inline Result execute(PGconn& connection, const std::string& statement) {
    Result result { PQexec(&connection, statement.c_str()) };
    if (PQresultStatus(result.get()) != PGRES_COMMAND_OK) {
        throw std::runtime_error {
            fmt::format("Error when executing a statement: {}", PQerrorMessage(&connection)),
        };
    }
    return result;
}

inline Result query(PGconn& connection, const std::string& statement) {
    Result result { PQexec(&connection, statement.c_str()) };
    if (PQresultStatus(result.get()) != PGRES_TUPLES_OK) {
        throw std::runtime_error {
            fmt::format("Error when executing a query: {}", PQerrorMessage(&connection)),
        };
    }
    return result;
}

using Connection = std::unique_ptr<PGconn, decltype([](PGconn* conn) { PQfinish(conn); })>;

inline Connection connect(const std::string& connectionStr) {
    Connection conn { PQconnectdb(connectionStr.c_str()) };
    if (PQstatus(conn.get()) != CONNECTION_OK) {
        throw std::runtime_error { fmt::format("Failed to connect to db: {}",
                                               PQerrorMessage(conn.get())) };
    }
    return conn;
}

}    // namespace pfrorm::postgres

template<>
struct fmt::formatter<pfrorm::postgres::Str> : formatter<const char*> {
public:
    template<typename FormatContext>
    constexpr auto format(const pfrorm::postgres::Str& str, FormatContext& ctx) const
        -> decltype(ctx.out()) {
        return formatter<const char*>::format(str.get(), ctx);
    }
};
