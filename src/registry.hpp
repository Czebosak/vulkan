#pragma once

#include <voxel/types.hpp>

#include <string>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

namespace registry {
    struct BlockDefinition {
        BlockID id;
        std::string name;
    };

    class Namespace {
    private:
        // Evil C++ Hackery
        template<typename ... Bases>
        struct overload : Bases ... {
            using is_transparent = void;
            using Bases::operator() ... ;
        };

        using transparent_string_hash = overload<
            std::hash<std::string>,
            std::hash<std::string_view>
        >;

        std::unordered_map<std::string, BlockDefinition, transparent_string_hash, std::equal_to<>> blocks;

        friend class Registry;
    public:
        void register_block(BlockDefinition block_definition);

        const BlockDefinition* get_block(std::string_view name) const;
    };

    class Registry {
    private:
        // Evil C++ Hackery
        template<typename ... Bases>
        struct overload : Bases ... {
            using is_transparent = void;
            using Bases::operator() ... ;
        };

        using transparent_string_hash = overload<
            std::hash<std::string>,
            std::hash<std::string_view>
        >;

        static Registry* singleton;

        std::unordered_map<std::string, Namespace, transparent_string_hash, std::equal_to<>> namespaces;
        std::unordered_map<BlockID, BlockDefinition*> shorthand_id_to_block;

        bool locked;

        Registry();
        ~Registry();

        Registry(const Registry&) = delete;
        Registry& operator=(const Registry&) = delete;

        // The reference is only guaranteed to be valid
        // as long as new namespaces aren't created
        Namespace& register_namespace(std::string name);
    public:
        static void create();
        static void destroy();

        static const Registry& get();
        static Registry& get_mut();

        void lock();

        void register_json_file(const std::filesystem::path& path);

        const BlockDefinition* get_block(std::string_view namespace_name, std::string_view name) const;

        // Name must be in format namespace:block
        const BlockDefinition* get_block(std::string_view name) const;
    };
}
