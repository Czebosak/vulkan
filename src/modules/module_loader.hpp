#pragma once

#include <string_view>
#include <span>
#include <vector>
#include <optional>

#include <modules/module.hpp>

#include <nlohmann/json.hpp>

#include <registry.hpp>

class Engine;

class ModuleLoader {
public:
    enum class ParseError {
        KeyMissing,
        UnexpectedType,
        NamespaceConflict,
        IDConflict
    };
private:
    ModuleLoader();

    static std::vector<Module> modules;

    friend Engine;

    static std::optional<ParseError> parse_definitions(nlohmann::json j, registry::Registry& registry);

    static std::optional<Module> load_module(std::string_view path);

    // Returns true if all modules loaded successfully
    static bool load_modules(std::span<std::string_view> module_dirs);
};
