#include "registry.hpp"

#include <fstream>

//#define JSON_NOEXCEPTION
#include <nlohmann/json.hpp>
#include <ctre.hpp>
#include <fmt/core.h>

using json = nlohmann::json;

template <typename T>
bool is_json_type(const json& json);

template <>
bool is_json_type<std::string>(const json& j) {
    return j.is_string();
}

template <>
bool is_json_type<json::array_t>(const json& j) {
    return j.is_array();
}

template <typename T>
T get_json_value(const json& j, const std::string& key) {
    T data;

    if (j.contains(key)) {
        const json& value = j[key];
        if (is_json_type<T>(value)) {
            data = value;
        } else {
            fmt::println("failed bro");
            exit(-1);
        }
    } else {
        fmt::println("failed bro");
        exit(-1);
    }

    return data;
}

namespace registry {
    Registry* Registry::singleton;

    void Namespace::register_block(BlockDefinition block_definiton) {
        blocks.emplace(std::make_pair(block_definiton.name, std::move(block_definiton)));
    }

    const BlockDefinition* Namespace::get_block(std::string_view name) const {
        auto it = blocks.find(name);

        if (it != blocks.end()) return &it->second;
        return nullptr;
    }

    Registry::Registry() {}
    Registry::~Registry() {}

    Namespace* Registry::get_namespace(std::string_view name) {
        auto it = namespaces.find(name);

        if (it != namespaces.end()) {
            return &it->second;
        } else {
            return nullptr;
        }
    }

    Namespace* Registry::register_namespace(std::string name) {
        auto it = namespaces.emplace(std::make_pair(name, Namespace()));
        if (it.second) {
            return &it.first->second;
        } else {
            return nullptr;
        }
    }

    void Registry::create() {
        singleton = new Registry();
        singleton->locked = false;
    }

    void Registry::destroy() {
        assert(singleton->locked);

        delete singleton;
    }

    const Registry& Registry::get() {
        return *singleton;
    }

    Registry& Registry::get_mut() {
        assert(!singleton->locked);

        return *singleton;
    }

    void Registry::lock() {
        locked = true;

        std::hash<std::string> hasher;
        for (auto& [namespace_name, ns] : namespaces) {
            for (auto& [block_name, block_definition] : ns.blocks) {
                // TODO: FIX
                size_t hash = hasher(namespace_name + block_name);
                block_definition.id = hash;

                assert(!shorthand_id_to_block.contains(block_definition.id));

                shorthand_id_to_block.emplace(std::make_pair(block_definition.id, &block_definition));
            }
        }
    }

    void Registry::register_json_file(const std::filesystem::path& path) {
        std::ifstream f(path);

        json data = json::parse(f);

        std::string namespace_name = get_json_value<std::string>(data, "name");

        Namespace* ns = register_namespace(std::move(namespace_name));

        json::array_t blocks_array = get_json_value<json::array_t>(data, "blocks");

        for (const auto& block_object : blocks_array) {
            if (block_object.is_object()) {
                ns->register_block({
                    .name = get_json_value<std::string>(block_object, "name"),
                });
            } else {
                fmt::println("While loading namespace {}: non json object type found in blocks array", namespace_name);
                exit(-1);
            }
        }
    }

    const BlockDefinition* Registry::get_block(std::string_view namespace_name, std::string_view name) const {
        auto it = namespaces.find(namespace_name);

        if (it != namespaces.end()) {
            const Namespace& ns = it->second;
            return ns.get_block(name);
        }

        return nullptr;
    }

    const BlockDefinition* Registry::get_block(std::string_view path) const {
        assert(ctre::match<"^[a-z]+:[a-z]+$">(path));

        size_t pos = path.find(':');
        std::string_view namespace_name = path.substr(0, pos);
        std::string_view name = path.substr(pos + 1);

        return get_block(namespace_name, name);
    }
}
