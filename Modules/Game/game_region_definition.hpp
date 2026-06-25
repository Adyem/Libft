#ifndef FT_REGION_DEFINITION_HPP
# define FT_REGION_DEFINITION_HPP

#include "../CPP_class/class_string.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class game_region_definition
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t         _region_id;
        ft_string   _name;
        ft_string   _description;
        int32_t         _recommended_level;
        pt_recursive_mutex   *_mutex;
        uint8_t     _initialised_state;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept;


        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        int32_t unlock_internal(ft_bool lock_acquired) const noexcept;

    public:
        game_region_definition() noexcept;
        virtual ~game_region_definition() noexcept;
        game_region_definition(const game_region_definition &other) noexcept = delete;
        game_region_definition &operator=(const game_region_definition &other) noexcept = delete;
        game_region_definition(game_region_definition &&other) noexcept = delete;
        game_region_definition &operator=(game_region_definition &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const game_region_definition &other) noexcept;
        int32_t initialize(game_region_definition &&other) noexcept;
        int32_t move(game_region_definition &other) noexcept;
        int32_t initialize(int32_t region_id, const ft_string &name,
            const ft_string &description, int32_t recommended_level) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t lock(ft_bool *lock_acquired) const noexcept;
        void unlock(ft_bool lock_acquired) const noexcept;

        int32_t get_region_id() const noexcept;
        void set_region_id(int32_t region_id) noexcept;

        const ft_string &get_name() const noexcept;
        void set_name(const ft_string &name) noexcept;

        const ft_string &get_description() const noexcept;
        void set_description(const ft_string &description) noexcept;

        int32_t get_recommended_level() const noexcept;
        void set_recommended_level(int32_t recommended_level) noexcept;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
