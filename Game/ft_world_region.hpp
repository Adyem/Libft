#ifndef FT_WORLD_REGION_HPP
# define FT_WORLD_REGION_HPP

#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_world_region
{
    private:
        int             _world_id;
        ft_vector<int>  _region_ids;
        pt_recursive_mutex       *_mutex;
        uint8_t         _initialized_state;
        static thread_local int _last_error;

        void set_error(int error_code) const noexcept;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_world_region() noexcept;
        virtual ~ft_world_region() noexcept;
        ft_world_region(const ft_world_region &other) = delete;
        ft_world_region &operator=(const ft_world_region &other) = delete;
        ft_world_region(ft_world_region &&other) = delete;
        ft_world_region &operator=(ft_world_region &&other) = delete;

        int initialize() noexcept;
        int initialize(const ft_world_region &other) noexcept;
        int initialize(ft_world_region &&other) noexcept;
        int initialize(int world_id, const ft_vector<int> &region_ids) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_world_id() const noexcept;
        void set_world_id(int world_id) noexcept;

        const ft_vector<int> &get_region_ids() const noexcept;
        ft_vector<int> &get_region_ids() noexcept;
        void set_region_ids(const ft_vector<int> &region_ids) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
