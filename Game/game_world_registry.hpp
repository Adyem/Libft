#ifndef GAME_WORLD_REGISTRY_HPP
# define GAME_WORLD_REGISTRY_HPP

#include "ft_region_definition.hpp"
#include "ft_world_region.hpp"
#include "../Template/map.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_world_registry
{
    private:
        ft_map<int, ft_region_definition> _regions;
        ft_map<int, ft_world_region>      _world_regions;
        pt_mutex                         *_mutex;
        uint8_t                           _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_world_registry() noexcept;
        virtual ~ft_world_registry() noexcept;
        ft_world_registry(const ft_world_registry &other) noexcept = delete;
        ft_world_registry &operator=(const ft_world_registry &other) noexcept = delete;
        ft_world_registry(ft_world_registry &&other) noexcept = delete;
        ft_world_registry &operator=(ft_world_registry &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_world_registry &other) noexcept;
        int initialize(ft_world_registry &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

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

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
