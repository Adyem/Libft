#include "game_world_replay.hpp"

#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"

ft_world_replay_session::ft_world_replay_session() noexcept
    : _snapshot_payload(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_world_replay_session::~ft_world_replay_session() noexcept
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_world_replay_session::ft_world_replay_session(const ft_world_replay_session &other) noexcept
    : _snapshot_payload(other._snapshot_payload), _error_code(other._error_code)
{
    if (this->_snapshot_payload.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        return ;
    }
    this->set_error(other._error_code);
    return ;
}

ft_world_replay_session &ft_world_replay_session::operator=(const ft_world_replay_session &other) noexcept
{
    if (this != &other)
    {
        this->_snapshot_payload = other._snapshot_payload;
        if (this->_snapshot_payload.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_snapshot_payload.get_error());
            return (*this);
        }
        this->set_error(other._error_code);
    }
    return (*this);
}

ft_world_replay_session::ft_world_replay_session(ft_world_replay_session &&other) noexcept
    : _snapshot_payload(ft_move(other._snapshot_payload)), _error_code(other._error_code)
{
    if (this->_snapshot_payload.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        other.set_error(ER_SUCCESS);
        return ;
    }
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    return ;
}

ft_world_replay_session &ft_world_replay_session::operator=(ft_world_replay_session &&other) noexcept
{
    if (this != &other)
    {
        this->_snapshot_payload = ft_move(other._snapshot_payload);
        if (this->_snapshot_payload.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_snapshot_payload.get_error());
            return (*this);
        }
        this->set_error(other._error_code);
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

void ft_world_replay_session::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int ft_world_replay_session::capture_snapshot(ft_world &world, const ft_character &character, const ft_inventory &inventory) noexcept
{
    ft_string snapshot_buffer;
    int result;

    result = world.save_to_buffer(snapshot_buffer, character, inventory);
    if (result != ER_SUCCESS)
    {
        this->set_error(world.get_error());
        return (this->_error_code);
    }
    this->_snapshot_payload = snapshot_buffer;
    if (this->_snapshot_payload.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
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
    if (load_result != ER_SUCCESS)
    {
        this->set_error(world_ptr->get_error());
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world_replay_session::replay_ticks(ft_sharedptr<ft_world> &world_ptr, ft_character &character, ft_inventory &inventory, int ticks,
    const char *log_file_path, ft_string *log_buffer) noexcept
{
    int restore_result;
    int world_error;

    restore_result = this->restore_snapshot(world_ptr, character, inventory);
    if (restore_result != ER_SUCCESS)
        return (this->_error_code);
    world_ptr->update_events(world_ptr, ticks, log_file_path, log_buffer);
    world_error = world_ptr->get_error();
    if (world_error != ER_SUCCESS)
    {
        this->set_error(world_error);
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world_replay_session::plan_route(ft_world &world, const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &path) noexcept
{
    int route_result;

    route_result = world.plan_route(grid, start_x, start_y, start_z, goal_x, goal_y, goal_z, path);
    if (route_result != ER_SUCCESS)
    {
        this->set_error(world.get_error());
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world_replay_session::import_snapshot(const ft_string &snapshot_payload) noexcept
{
    this->_snapshot_payload = snapshot_payload;
    if (this->_snapshot_payload.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world_replay_session::export_snapshot(ft_string &out_snapshot) const noexcept
{
    out_snapshot = this->_snapshot_payload;
    if (out_snapshot.get_error() != ER_SUCCESS)
    {
        this->set_error(out_snapshot.get_error());
        return (out_snapshot.get_error());
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

void ft_world_replay_session::clear_snapshot() noexcept
{
    this->_snapshot_payload.clear();
    if (this->_snapshot_payload.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_snapshot_payload.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
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
