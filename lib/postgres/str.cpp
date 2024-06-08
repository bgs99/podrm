#include <podrm/postgres/detail/str.hpp>

#include <libpq-fe.h>

namespace podrm::postgres::detail {

Str::~Str() { PQfreemem(this->str); }

} // namespace podrm::postgres::detail
