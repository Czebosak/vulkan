#pragma once

#include <vector>
#include <string>

enum class ModuleType {
    Native,
    Wasm,
};

struct ModuleSettings {
    ModuleType type;
};

struct Module {
    ModuleSettings settings;
    std::vector<std::string> defined_namespaces;
};
