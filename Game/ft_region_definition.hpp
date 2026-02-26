#ifndef FT_REGION_DEFINITION_HPP
# define FT_REGION_DEFINITION_HPP

#include "../CPP_class/class_string.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_region_definition
{
    private:
        int         _region_id;
        ft_string   _name;
        ft_string   _description;
        int         _recommended_level;
        pt_recursive_mutex   *_mutex;
        uint8_t     _initialized_state;
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
        ft_region_definition() noexcept;
        virtual ~ft_region_definition() noexcept;
        ft_region_definition(const ft_region_definition &other) noexcept = delete;
        ft_region_definition &operator=(const ft_region_definition &other) noexcept = delete;
        ft_region_definition(ft_region_definition &&other) noexcept = delete;
        ft_region_definition &operator=(ft_region_definition &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_region_definition &other) noexcept;
        int initialize(ft_region_definition &&other) noexcept;
        int initialize(int region_id, const ft_string &name,
            const ft_string &description, int recommended_level) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_region_id() const noexcept;
        void set_region_id(int region_id) noexcept;

        const ft_string &get_name() const noexcept;
        void set_name(const ft_string &name) noexcept;

        const ft_string &get_description() const noexcept;
        void set_description(const ft_string &description) noexcept;

        int get_recommended_level() const noexcept;
        void set_recommended_level(int recommended_level) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
