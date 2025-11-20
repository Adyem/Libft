#ifndef GAME_WORLD_REGISTRY_HPP
# define GAME_WORLD_REGISTRY_HPP

#include "ft_region_definition.hpp"
#include "ft_world_region.hpp"
#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_world_registry
{
    private:
        ft_map<int, ft_region_definition> _regions;
        ft_map<int, ft_world_region> _world_regions;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;

    public:
        ft_world_registry() noexcept;
        virtual ~ft_world_registry() noexcept;
        ft_world_registry(const ft_world_registry &other) noexcept;
        ft_world_registry &operator=(const ft_world_registry &other) noexcept;
        ft_world_registry(ft_world_registry &&other) noexcept;
        ft_world_registry &operator=(ft_world_registry &&other) noexcept;

        int register_region(const ft_region_definition &region) noexcept;
        int register_world(const ft_world_region &world_region) noexcept;

        int fetch_region(int region_id, ft_region_definition &out_region) const noexcept;
        int fetch_world(int world_id, ft_world_region &out_world) const noexcept;

        ft_map<int, ft_region_definition> &get_regions() noexcept;
        const ft_map<int, ft_region_definition> &get_regions() const noexcept;
        void set_regions(const ft_map<int, ft_region_definition> &regions) noexcept;

        ft_map<int, ft_world_region> &get_world_regions() noexcept;
        const ft_map<int, ft_world_region> &get_world_regions() const noexcept;
        void set_world_regions(const ft_map<int, ft_world_region> &world_regions) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
