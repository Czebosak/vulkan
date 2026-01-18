#include "module_loader.hpp"

#include <fstream>
#include <filesystem>
#include <expected>

#include <dylib.hpp>

using namespace nlohmann;
using namespace registry;

namespace fs = std::filesystem;

constexpr std::string_view MODULE_DEFINITION_FILENAME = "module.json";
constexpr std::string_view MODULE_BINARY_FILENAME = "module";

std::vector<Module> ModuleLoader::modules;

template <typename T>
bool is_json_typez(const json& json);

template <>
bool is_json_typez<std::string>(const json& j) {
    return j.is_string();
}

template <>
bool is_json_typez<json::array_t>(const json& j) {
    return j.is_array();
}

template <typename T>
std::expected<T, ModuleLoader::ParseError> get_json_value(const json& j, const std::string& key) {
    T data;

    if (j.contains(key)) {
        const json& value = j[key];
        if (is_json_typez<T>(value)) {
            data = value;
        } else {
            return std::unexpected(ModuleLoader::ParseError::UnexpectedType);
        }
    } else {
        return std::unexpected(ModuleLoader::ParseError::KeyMissing);
    }

    return data;
}

std::optional<ModuleLoader::ParseError> ModuleLoader::parse_definitions(json j, Registry& registry) {
    auto ns_name = get_json_value<std::string>(j, "namespace");
    if (!ns_name) {
        return std::make_optional(ns_name.error());
    }
    Namespace* ns = registry.get_namespace(ns_name.value());

    return std::nullopt;
}

std::optional<Module> ModuleLoader::load_module(std::string_view path) {
    Module module;

    fmt::println("{}", std::format("{}/{}", path, MODULE_DEFINITION_FILENAME));
    std::ifstream f(std::format("{}/{}", path, MODULE_DEFINITION_FILENAME));
    json config = json::parse(f);

    auto type = get_json_value<std::string>(config, "type");
    if (!type) return std::nullopt;

    if (*type == "native") {
        auto entry_symbol = get_json_value<std::string>(config, "entry_symbol");
        if (!entry_symbol) return std::nullopt;

        dylib::library module_lib(std::format("{}/{}", path, MODULE_BINARY_FILENAME), dylib::decorations::os_default());

        bool success = module_lib.get_function<bool()>(*entry_symbol)();
        fmt::println("{}", success);
    }

    auto& registry = Registry::get_mut();

    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == "json") {
            parse_definitions(json::parse(entry.path().c_str()), registry);
        }
    }

    return module;
}

bool ModuleLoader::load_modules(std::span<std::string_view> module_paths) {
    for (std::string_view path : module_paths) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_directory()) {
                auto dir_path = entry.path();
                if (fs::exists(entry.path() / MODULE_DEFINITION_FILENAME)) {
                    load_module(dir_path.c_str());
                }
            }
        }
    }

    return true;
}
