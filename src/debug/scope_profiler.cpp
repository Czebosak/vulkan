#include "scope_profiler.hpp"

#include <fmt/core.h>

debug::ScopeProfiler::ScopeProfiler() : start(std::chrono::steady_clock::now()) {}

debug::ScopeProfiler::~ScopeProfiler() {
    auto end = std::chrono::steady_clock::now();
    auto elapsed = duration_cast<std::chrono::microseconds>(end - start).count();
    fmt::println("{}", elapsed);
}
