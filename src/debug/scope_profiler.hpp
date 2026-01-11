#pragma once

#include <chrono>

namespace debug {
    class ScopeProfiler {
    private:
        std::chrono::steady_clock::time_point start;
    public:
        ScopeProfiler();
        ~ScopeProfiler();
    };
}
