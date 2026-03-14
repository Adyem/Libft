#ifndef GAME_WORLD_REPLAY_HPP
# define GAME_WORLD_REPLAY_HPP

#include "game_world.hpp"
#include "game_character.hpp"
#include "game_inventory.hpp"
#include "game_pathfinding.hpp"
#include "../Template/shared_ptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/vector.hpp"
#include "../Template/function.hpp"
#include <stdint.h>

class ft_world_replay_session
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        ft_string   _snapshot_payload;
        ft_vector<ft_function<void(ft_world&, ft_event&)> > _event_callbacks;
        uint8_t     _initialised_state;


    public:
        ft_world_replay_session() noexcept;
        ~ft_world_replay_session() noexcept;
        ft_world_replay_session(const ft_world_replay_session &other) noexcept;
        ft_world_replay_session &operator=(const ft_world_replay_session &other) noexcept = delete;
        ft_world_replay_session(ft_world_replay_session &&other) noexcept;
        ft_world_replay_session &operator=(ft_world_replay_session &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t move(ft_world_replay_session &other) noexcept;
        int32_t destroy() noexcept;
        int32_t capture_snapshot(ft_world &world, const ft_character &character, const ft_inventory &inventory) noexcept;
        int32_t restore_snapshot(ft_sharedptr<ft_world> &world_ptr, ft_character &character, ft_inventory &inventory) noexcept;
        int32_t replay_ticks(ft_sharedptr<ft_world> &world_ptr, ft_character &character, ft_inventory &inventory, int32_t ticks,
            const char *log_file_path = ft_nullptr, ft_string *log_buffer = ft_nullptr) noexcept;
        int32_t plan_route(ft_world &world, const ft_map3d &grid,
            ft_size_t start_x, ft_size_t start_y, ft_size_t start_z,
            ft_size_t goal_x, ft_size_t goal_y, ft_size_t goal_z,
            ft_vector<ft_path_step> &path) noexcept;
        int32_t import_snapshot(const ft_string &snapshot_payload) noexcept;
        int32_t export_snapshot(ft_string &out_snapshot) const noexcept;
        void clear_snapshot() noexcept;
};

#endif
