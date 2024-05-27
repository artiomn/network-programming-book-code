#pragma once

#include <memory>
#include <type_traits>
#include <utility>

template <typename F>
auto scope_guard(F&& f)
{
    return std::unique_ptr<void, typename std::decay<F>::type>{reinterpret_cast<void*>(1), std::forward<F>(f)};
}
