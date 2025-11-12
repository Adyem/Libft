#ifndef GAME_HOOKS_HPP
# define GAME_HOOKS_HPP

#include "game_character.hpp"
#include "game_item.hpp"
#include "game_world.hpp"
#include "game_event.hpp"
#include "../Template/function.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_game_hooks
{
    private:
        ft_function<void(ft_character&, ft_item&)> _on_item_crafted;
        ft_function<void(ft_character&, int, uint8_t)> _on_character_damaged;
        ft_function<void(ft_world&, ft_event&)> _on_event_triggered;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error) const noexcept;
        static int lock_pair(const ft_game_hooks &first, const ft_game_hooks &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

        template <typename Callback, typename... Args>
        friend void game_hooks_invoke_safe(const ft_game_hooks *hooks,
                Callback &callback,
                ft_unique_lock<pt_mutex> &guard,
                int entry_errno,
                Args &...args) noexcept;

    public:
        ft_game_hooks() noexcept;
        ~ft_game_hooks() noexcept;
        ft_game_hooks(const ft_game_hooks &other) noexcept;
        ft_game_hooks &operator=(const ft_game_hooks &other) noexcept;
        ft_game_hooks(ft_game_hooks &&other) noexcept;
        ft_game_hooks &operator=(ft_game_hooks &&other) noexcept;

        void set_on_item_crafted(ft_function<void(ft_character&, ft_item&)> &&callback) noexcept;
        void set_on_character_damaged(ft_function<void(ft_character&, int, uint8_t)> &&callback) noexcept;
        void set_on_event_triggered(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept;

        ft_function<void(ft_character&, ft_item&)> get_on_item_crafted() const noexcept;
        ft_function<void(ft_character&, int, uint8_t)> get_on_character_damaged() const noexcept;
        ft_function<void(ft_world&, ft_event&)> get_on_event_triggered() const noexcept;

        void invoke_on_item_crafted(ft_character &character, ft_item &item) const noexcept;
        void invoke_on_character_damaged(ft_character &character, int damage, uint8_t type) const noexcept;
        void invoke_on_event_triggered(ft_world &world, ft_event &event) const noexcept;

        void reset() noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
