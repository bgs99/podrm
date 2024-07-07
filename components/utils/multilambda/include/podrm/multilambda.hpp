#pragma once

namespace podrm::detail {

/// Invocable that overloads its call operator using call operators of the
/// parent classes
/// @tparam Ts types of the parent classes
template <typename... Ts> struct MultiLambda : Ts... {
  using Ts::operator()...;
};

template <typename... Ts> MultiLambda(Ts...) -> MultiLambda<Ts...>;

} // namespace podrm::detail
