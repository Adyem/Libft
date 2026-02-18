#include "game_world_replay.hpp"

#include "../Errno/errno.hpp"
#include "../Template/move.hpp"

static int world_replay_collect_callbacks(ft_world &world,
    ft_vector<ft_function<void(ft_world&, ft_event&)> > &callbacks) noexcept
{
    ft_sharedptr<ft_event_scheduler> &scheduler = world.get_event_scheduler();

    if (!scheduler)
        return (FT_ERR_GAME_GENERAL_ERROR);
    ft_vector<ft_sharedptr<ft_event> > scheduled_events;

    scheduler->dump_events(scheduled_events);
    if (scheduler->get_error() != FT_ERR_SUCCESS)
        return (scheduler->get_error());
    size_t event_index;
    size_t event_count;

    event_index = 0;
    event_count = scheduled_events.size();
    while (event_index < event_count)
    {
        const ft_function<void(ft_world&, ft_event&)> &event_callback =
            scheduled_events[event_index]->get_callback();

        if (scheduled_events[event_index]->get_error() != FT_ERR_SUCCESS)
            return (scheduled_events[event_index]->get_error());
        callbacks.push_back(event_callback);
        if (ft_vector<ft_function<void(ft_world&, ft_event&)> >::last_operation_error() != FT_ERR_SUCCESS)
            return (ft_vector<ft_function<void(ft_world&, ft_event&)> >::last_operation_error());
        event_index++;
    }
    return (FT_ERR_SUCCESS);
}

static int world_replay_restore_callbacks(ft_world &world,
    const ft_vector<ft_function<void(ft_world&, ft_event&)> > &callbacks) noexcept
{
    ft_sharedptr<ft_event_scheduler> &scheduler = world.get_event_scheduler();

    if (!scheduler)
        return (FT_ERR_GAME_GENERAL_ERROR);
    if (callbacks.size() == 0)
        return (FT_ERR_SUCCESS);
    ft_vector<ft_sharedptr<ft_event> > scheduled_events;

    scheduler->dump_events(scheduled_events);
    if (scheduler->get_error() != FT_ERR_SUCCESS)
        return (scheduler->get_error());
    size_t event_index;
    size_t event_count;
    size_t callback_count;

    callback_count = callbacks.size();
    event_index = 0;
    event_count = scheduled_events.size();
    if (callback_count != event_count)
        return (FT_ERR_GAME_GENERAL_ERROR);
    while (event_index < event_count)
    {
        ft_function<void(ft_world&, ft_event&)> callback_copy(callbacks[event_index]);

        scheduled_events[event_index]->set_callback(ft_move(callback_copy));
        if (scheduled_events[event_index]->get_error() != FT_ERR_SUCCESS)
            return (scheduled_events[event_index]->get_error());
        event_index++;
    }
    return (FT_ERR_SUCCESS);
}

ft_world_replay_session::ft_world_replay_session() noexcept
    : _snapshot_payload(), _event_callbacks()
{
    return ;
}

ft_world_replay_session::~ft_world_replay_session() noexcept
{
    return ;
}

int ft_world_replay_session::capture_snapshot(ft_world &world,
    const ft_character &character, const ft_inventory &inventory) noexcept
{
    ft_string snapshot_buffer;
    int result;
    ft_vector<ft_function<void(ft_world&, ft_event&)> > callback_snapshot;
    int callback_result;
    size_t callback_index;
    size_t callback_count;

    result = world.save_to_buffer(snapshot_buffer, character, inventory);
    if (result != FT_ERR_SUCCESS)
        return (result);
    callback_result = world_replay_collect_callbacks(world, callback_snapshot);
    if (callback_result != FT_ERR_SUCCESS)
        return (callback_result);
    this->_snapshot_payload = snapshot_buffer;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    this->_event_callbacks.clear();
    callback_index = 0;
    callback_count = callback_snapshot.size();
    while (callback_index < callback_count)
    {
        this->_event_callbacks.push_back(callback_snapshot[callback_index]);
        if (ft_vector<ft_function<void(ft_world&, ft_event&)> >::last_operation_error() != FT_ERR_SUCCESS)
            return (ft_vector<ft_function<void(ft_world&, ft_event&)> >::last_operation_error());
        callback_index++;
    }
    return (FT_ERR_SUCCESS);
}

int ft_world_replay_session::restore_snapshot(ft_sharedptr<ft_world> &world_ptr,
    ft_character &character, ft_inventory &inventory) noexcept
{
    int load_result;
    int callback_result;

    if (!world_ptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (this->_snapshot_payload.empty())
        return (FT_ERR_INVALID_STATE);
    load_result = world_ptr->load_from_buffer(this->_snapshot_payload.c_str(),
            character, inventory);
    if (load_result != FT_ERR_SUCCESS)
        return (load_result);
    if (this->_event_callbacks.size() > 0)
    {
        callback_result = world_replay_restore_callbacks(*world_ptr,
                this->_event_callbacks);
        if (callback_result != FT_ERR_SUCCESS)
            return (callback_result);
    }
    return (FT_ERR_SUCCESS);
}

int ft_world_replay_session::replay_ticks(ft_sharedptr<ft_world> &world_ptr,
    ft_character &character, ft_inventory &inventory, int ticks,
    const char *log_file_path, ft_string *log_buffer) noexcept
{
    int restore_result;

    restore_result = this->restore_snapshot(world_ptr, character, inventory);
    if (restore_result != FT_ERR_SUCCESS)
        return (restore_result);
    world_ptr->update_events(world_ptr, ticks, log_file_path, log_buffer);
    return (world_ptr->get_error());
}

int ft_world_replay_session::plan_route(ft_world &world, const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &path) noexcept
{
    return (world.plan_route(grid, start_x, start_y, start_z,
            goal_x, goal_y, goal_z, path));
}

int ft_world_replay_session::import_snapshot(const ft_string &snapshot_payload)
    noexcept
{
    this->_snapshot_payload = snapshot_payload;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    this->_event_callbacks.clear();
    return (FT_ERR_SUCCESS);
}

int ft_world_replay_session::export_snapshot(ft_string &out_snapshot) const
    noexcept
{
    out_snapshot = this->_snapshot_payload;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (FT_ERR_SUCCESS);
}

void ft_world_replay_session::clear_snapshot() noexcept
{
    this->_snapshot_payload.clear();
    this->_event_callbacks.clear();
    return ;
}
