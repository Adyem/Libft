#include "game_world_replay.hpp"

#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Template/move.hpp"

static int  world_replay_collect_callbacks(ft_world &world,
        ft_vector<ft_function<void(ft_world&, ft_event&)> > &callbacks) noexcept
{
    ft_sharedptr<ft_event_scheduler> &scheduler = world.get_event_scheduler();

    if (!scheduler)
        return (FT_ERR_GAME_GENERAL_ERROR);
    ft_vector<ft_sharedptr<ft_event> > scheduled_events;

    scheduler->dump_events(scheduled_events);
    if (scheduler->get_error() != FT_ERR_SUCCESSS)
        return (scheduler->get_error());
    size_t  event_index;
    size_t  event_count;

    event_index = 0;
    event_count = scheduled_events.size();
    while (event_index < event_count)
    {
        const ft_function<void(ft_world&, ft_event&)> &event_callback =
            scheduled_events[event_index]->get_callback();

        if (scheduled_events[event_index]->get_error() != FT_ERR_SUCCESSS)
            return (scheduled_events[event_index]->get_error());
        callbacks.push_back(event_callback);
        if (callbacks.get_error() != FT_ERR_SUCCESSS)
            return (callbacks.get_error());
        event_index++;
    }
    return (FT_ERR_SUCCESSS);
}

static int  world_replay_restore_callbacks(ft_world &world,
        const ft_vector<ft_function<void(ft_world&, ft_event&)> > &callbacks) noexcept
{
    ft_sharedptr<ft_event_scheduler> &scheduler = world.get_event_scheduler();

    if (!scheduler)
        return (FT_ERR_GAME_GENERAL_ERROR);
    if (callbacks.size() == 0)
        return (FT_ERR_SUCCESSS);
    ft_vector<ft_sharedptr<ft_event> > scheduled_events;

    scheduler->dump_events(scheduled_events);
    if (scheduler->get_error() != FT_ERR_SUCCESSS)
        return (scheduler->get_error());
    size_t  event_index;
    size_t  event_count;
    size_t  callback_count;

    callback_count = callbacks.size();
    event_index = 0;
    event_count = scheduled_events.size();
    if (callback_count != event_count)
        return (FT_ERR_GAME_GENERAL_ERROR);
    while (event_index < event_count)
    {
        ft_function<void(ft_world&, ft_event&)> callback_copy(callbacks[event_index]);

        scheduled_events[event_index]->set_callback(ft_move(callback_copy));
        if (scheduled_events[event_index]->get_error() != FT_ERR_SUCCESSS)
            return (scheduled_events[event_index]->get_error());
        event_index++;
    }
    return (FT_ERR_SUCCESSS);
}

ft_world_replay_session::ft_world_replay_session() noexcept
    : _snapshot_payload(), _event_callbacks(), _error_code(FT_ERR_SUCCESSS)
{
    return ;
}

ft_world_replay_session::~ft_world_replay_session() noexcept
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_world_replay_session::ft_world_replay_session(const ft_world_replay_session &other) noexcept
    : _snapshot_payload(other._snapshot_payload),
      _event_callbacks(), _error_code(other._error_code)
{
    size_t  callback_index;
    size_t  callback_count;

    if (this->_snapshot_payload.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        return ;
    }
    if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_event_callbacks.get_error());
        return ;
    }
    callback_index = 0;
    callback_count = other._event_callbacks.size();
    while (callback_index < callback_count)
    {
        this->_event_callbacks.push_back(other._event_callbacks[callback_index]);
        if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->_event_callbacks.get_error());
            return ;
        }
        callback_index++;
    }
    this->set_error(other._error_code);
    return ;
}

ft_world_replay_session &ft_world_replay_session::operator=(const ft_world_replay_session &other) noexcept
{
    if (this != &other)
    {
        this->_snapshot_payload = other._snapshot_payload;
        if (this->_snapshot_payload.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->_snapshot_payload.get_error());
            return (*this);
        }
        this->_event_callbacks.clear();
        if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->_event_callbacks.get_error());
            return (*this);
        }
        size_t  callback_index;
        size_t  callback_count;

        callback_index = 0;
        callback_count = other._event_callbacks.size();
        while (callback_index < callback_count)
        {
            this->_event_callbacks.push_back(other._event_callbacks[callback_index]);
            if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
            {
                this->set_error(this->_event_callbacks.get_error());
                return (*this);
            }
            callback_index++;
        }
        this->set_error(other._error_code);
    }
    return (*this);
}

ft_world_replay_session::ft_world_replay_session(ft_world_replay_session &&other) noexcept
    : _snapshot_payload(ft_move(other._snapshot_payload)),
      _event_callbacks(ft_move(other._event_callbacks)), _error_code(other._error_code)
{
    if (this->_snapshot_payload.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        other.set_error(FT_ERR_SUCCESSS);
        return ;
    }
    if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_event_callbacks.get_error());
        other.set_error(FT_ERR_SUCCESSS);
        return ;
    }
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_world_replay_session &ft_world_replay_session::operator=(ft_world_replay_session &&other) noexcept
{
    if (this != &other)
    {
        this->_snapshot_payload = ft_move(other._snapshot_payload);
        if (this->_snapshot_payload.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->_snapshot_payload.get_error());
            return (*this);
        }
        this->_event_callbacks = ft_move(other._event_callbacks);
        if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->_event_callbacks.get_error());
            return (*this);
        }
        this->set_error(other._error_code);
        other.set_error(FT_ERR_SUCCESSS);
    }
    return (*this);
}

void ft_world_replay_session::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    return ;
}

int ft_world_replay_session::capture_snapshot(ft_world &world, const ft_character &character, const ft_inventory &inventory) noexcept
{
    ft_string snapshot_buffer;
    int result;
    ft_vector<ft_function<void(ft_world&, ft_event&)> > callback_snapshot;
    int callback_result;

    result = world.save_to_buffer(snapshot_buffer, character, inventory);
    if (result != FT_ERR_SUCCESSS)
    {
        this->set_error(world.get_error());
        return (this->_error_code);
    }
    callback_result = world_replay_collect_callbacks(world, callback_snapshot);
    if (callback_result != FT_ERR_SUCCESSS)
    {
        this->set_error(callback_result);
        return (this->_error_code);
    }
    this->_snapshot_payload = snapshot_buffer;
    if (this->_snapshot_payload.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        return (this->_error_code);
    }
    this->_event_callbacks = ft_move(callback_snapshot);
    if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_event_callbacks.get_error());
        return (this->_error_code);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int ft_world_replay_session::restore_snapshot(ft_sharedptr<ft_world> &world_ptr, ft_character &character, ft_inventory &inventory) noexcept
{
    int load_result;

    if (!world_ptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (this->_error_code);
    }
    if (this->_snapshot_payload.empty())
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (this->_error_code);
    }
    load_result = world_ptr->load_from_buffer(this->_snapshot_payload.c_str(), character, inventory);
    if (load_result != FT_ERR_SUCCESSS)
    {
        this->set_error(world_ptr->get_error());
        return (this->_error_code);
    }
    if (this->_event_callbacks.size() > 0)
    {
        int callback_result;

        callback_result = world_replay_restore_callbacks(*world_ptr, this->_event_callbacks);
        if (callback_result != FT_ERR_SUCCESSS)
        {
            this->set_error(callback_result);
            return (this->_error_code);
        }
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int ft_world_replay_session::replay_ticks(ft_sharedptr<ft_world> &world_ptr, ft_character &character, ft_inventory &inventory, int ticks,
    const char *log_file_path, ft_string *log_buffer) noexcept
{
    int restore_result;
    int world_error;

    restore_result = this->restore_snapshot(world_ptr, character, inventory);
    if (restore_result != FT_ERR_SUCCESSS)
        return (this->_error_code);
    world_ptr->update_events(world_ptr, ticks, log_file_path, log_buffer);
    world_error = world_ptr->get_error();
    if (world_error != FT_ERR_SUCCESSS)
    {
        this->set_error(world_error);
        return (this->_error_code);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int ft_world_replay_session::plan_route(ft_world &world, const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &path) noexcept
{
    int route_result;

    route_result = world.plan_route(grid, start_x, start_y, start_z, goal_x, goal_y, goal_z, path);
    if (route_result != FT_ERR_SUCCESSS)
    {
        this->set_error(world.get_error());
        return (this->_error_code);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int ft_world_replay_session::import_snapshot(const ft_string &snapshot_payload) noexcept
{
    this->_snapshot_payload = snapshot_payload;
    if (this->_snapshot_payload.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        return (this->_error_code);
    }
    this->_event_callbacks.clear();
    if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_event_callbacks.get_error());
        return (this->_error_code);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int ft_world_replay_session::export_snapshot(ft_string &out_snapshot) const noexcept
{
    out_snapshot = this->_snapshot_payload;
    if (out_snapshot.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(out_snapshot.get_error());
        return (out_snapshot.get_error());
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void ft_world_replay_session::clear_snapshot() noexcept
{
    this->_snapshot_payload.clear();
    if (this->_snapshot_payload.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        return ;
    }
    this->_event_callbacks.clear();
    if (this->_event_callbacks.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_event_callbacks.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_world_replay_session::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_world_replay_session::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
