#ifndef GAME_WORLD_REGISTRY_HPP
# define GAME_WORLD_REGISTRY_HPP

#include "ft_region_definition.hpp"
#include "ft_world_region.hpp"
#include "../Template/map.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_world_registry
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        ft_map<int32_t, ft_region_definition> _regions;
        ft_map<int32_t, ft_world_region>      _world_regions;
        pt_recursive_mutex                         *_mutex;
        uint8_t                           _initialised_state;
        static thread_local int32_t           _last_error;

        static int32_t set_error(int32_t error_code) noexcept;


        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        int32_t unlock_internal(ft_bool lock_acquired) const noexcept;

    public:
        ft_world_registry() noexcept;
        virtual ~ft_world_registry() noexcept;
        ft_world_registry(const ft_world_registry &other) noexcept;
        ft_world_registry &operator=(const ft_world_registry &other) noexcept = delete;
        ft_world_registry(ft_world_registry &&other) noexcept;
        ft_world_registry &operator=(ft_world_registry &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_world_registry &other) noexcept;
        int32_t initialize(ft_world_registry &&other) noexcept;
        int32_t move(ft_world_registry &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t lock(ft_bool *lock_acquired) const noexcept;
        void unlock(ft_bool lock_acquired) const noexcept;

        int32_t register_region(const ft_region_definition &region) noexcept;
        int32_t register_world(const ft_world_region &world_region) noexcept;

        int32_t fetch_region(int32_t region_id, ft_region_definition &out_region) const noexcept;
        int32_t fetch_world(int32_t world_id, ft_world_region &out_world) const noexcept;

        ft_map<int32_t, ft_region_definition> &get_regions() noexcept;
        const ft_map<int32_t, ft_region_definition> &get_regions() const noexcept;
        void set_regions(const ft_map<int32_t, ft_region_definition> &regions) noexcept;

        ft_map<int32_t, ft_world_region> &get_world_regions() noexcept;
        const ft_map<int32_t, ft_world_region> &get_world_regions() const noexcept;
        void set_world_regions(const ft_map<int32_t, ft_world_region> &world_regions) noexcept;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
