#pragma once

namespace podrm::detail {

template <auto FunctionPtr> struct UnaryFunctionPtrClassImpl {};

template <typename Ret, typename Arg, Ret (*FunctionPtr)(Arg)>
struct UnaryFunctionPtrClassImpl<FunctionPtr> {
  using ReturnType = Ret;
  using ArgumentType = Arg;
};

template <auto FunctionPtr>
using UnaryFnReturnType = UnaryFunctionPtrClassImpl<FunctionPtr>::ReturnType;

template <auto FunctionPtr>
using UnaryFnArgumentType =
    UnaryFunctionPtrClassImpl<FunctionPtr>::ArgumentType;

} // namespace podrm::detail
