#pragma once

extern "C" struct EngineAPI {
    void (*register_system)(void (*fn)(void*), const char* name, const char* query);
};
