#include "game_state.hpp"
#include "../Errno/errno.hpp"
#include <utility>

ft_game_state::ft_game_state() noexcept
    : _worlds(), _characters(), _error_code(ER_SUCCESS)
{
    ft_sharedptr<ft_world> world(new (std::nothrow) ft_world());
    if (!world)
        this->set_error(GAME_GENERAL_ERROR);
    else if (world.get_error() != ER_SUCCESS)
        this->set_error(world.get_error());
    else if (world->get_error() != ER_SUCCESS)
        this->set_error(world->get_error());
    else
    {
        this->_worlds.push_back(world);
        if (this->_worlds.get_error() != ER_SUCCESS)
            this->set_error(this->_worlds.get_error());
    }
    this->set_error(this->_error_code);
    return ;
}

ft_game_state::~ft_game_state() noexcept
{
    return ;
}

ft_game_state::ft_game_state(const ft_game_state &other) noexcept
    : _worlds(), _characters(), _error_code(other._error_code)
{
    size_t world_index = 0;
    size_t world_count = other._worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = other._worlds[world_index];
        if (temp_world.get_error() != ER_SUCCESS)
        {
            this->set_error(temp_world.get_error());
            return ;
        }
        if (temp_world && temp_world->get_error() != ER_SUCCESS)
        {
            this->set_error(temp_world->get_error());
            return ;
        }
        this->_worlds.push_back(temp_world);
        if (this->_worlds.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_worlds.get_error());
            return ;
        }
        world_index++;
    }
    size_t character_index = 0;
    size_t character_count = other._characters.size();
    while (character_index < character_count)
    {
        ft_sharedptr<ft_character> temp = other._characters[character_index];
        if (temp.get_error() != ER_SUCCESS)
        {
            this->set_error(temp.get_error());
            return ;
        }
        if (temp && temp->get_error() != ER_SUCCESS)
        {
            this->set_error(temp->get_error());
            return ;
        }
        this->_characters.push_back(temp);
        if (this->_characters.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_characters.get_error());
            return ;
        }
        character_index++;
    }
    this->set_error(this->_error_code);
    return ;
}

ft_game_state &ft_game_state::operator=(const ft_game_state &other) noexcept
{
    if (this != &other)
    {
        this->_worlds.clear();
        size_t world_index = 0;
        size_t world_count = other._worlds.size();
        while (world_index < world_count)
        {
            ft_sharedptr<ft_world> temp_world = other._worlds[world_index];
            if (temp_world.get_error() != ER_SUCCESS)
            {
                this->set_error(temp_world.get_error());
                return (*this);
            }
            if (temp_world && temp_world->get_error() != ER_SUCCESS)
            {
                this->set_error(temp_world->get_error());
                return (*this);
            }
            this->_worlds.push_back(temp_world);
            if (this->_worlds.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_worlds.get_error());
                return (*this);
            }
            world_index++;
        }
        this->_characters.clear();
        size_t character_index = 0;
        size_t character_count = other._characters.size();
        while (character_index < character_count)
        {
            ft_sharedptr<ft_character> temp = other._characters[character_index];
            if (temp.get_error() != ER_SUCCESS)
            {
                this->set_error(temp.get_error());
                return (*this);
            }
            if (temp && temp->get_error() != ER_SUCCESS)
            {
                this->set_error(temp->get_error());
                return (*this);
            }
            this->_characters.push_back(temp);
            if (this->_characters.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_characters.get_error());
                return (*this);
            }
            character_index++;
        }
        this->_error_code = other._error_code;
    }
    this->set_error(this->_error_code);
    return (*this);
}

ft_game_state::ft_game_state(ft_game_state &&other) noexcept
    : _worlds(std::move(other._worlds)),
    _characters(std::move(other._characters)),
    _error_code(other._error_code)
{
    size_t world_index = 0;
    size_t world_count = this->_worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = this->_worlds[world_index];
        if (temp_world.get_error() != ER_SUCCESS)
            this->set_error(temp_world.get_error());
        else if (temp_world && temp_world->get_error() != ER_SUCCESS)
            this->set_error(temp_world->get_error());
        world_index++;
    }
    if (this->_worlds.get_error() != ER_SUCCESS)
        this->set_error(this->_worlds.get_error());
    if (this->_characters.get_error() != ER_SUCCESS)
        this->set_error(this->_characters.get_error());
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    return ;
}

ft_game_state &ft_game_state::operator=(ft_game_state &&other) noexcept
{
    if (this != &other)
    {
        this->_worlds = std::move(other._worlds);
        this->_characters = std::move(other._characters);
        this->_error_code = other._error_code;
        size_t world_index = 0;
        size_t world_count = this->_worlds.size();
        while (world_index < world_count)
        {
            ft_sharedptr<ft_world> temp_world = this->_worlds[world_index];
            if (temp_world.get_error() != ER_SUCCESS)
                this->set_error(temp_world.get_error());
            else if (temp_world && temp_world->get_error() != ER_SUCCESS)
                this->set_error(temp_world->get_error());
            world_index++;
        }
        if (this->_worlds.get_error() != ER_SUCCESS)
            this->set_error(this->_worlds.get_error());
        if (this->_characters.get_error() != ER_SUCCESS)
            this->set_error(this->_characters.get_error());
        other._error_code = ER_SUCCESS;
    }
    this->set_error(this->_error_code);
    return (*this);
}

ft_vector<ft_sharedptr<ft_world> > &ft_game_state::get_worlds() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_worlds);
}

ft_vector<ft_sharedptr<ft_character> > &ft_game_state::get_characters() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_characters);
}

int ft_game_state::add_character(const ft_sharedptr<ft_character> &character) noexcept
{
    if (!character)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error_code);
    }
    if (character.get_error() != ER_SUCCESS)
    {
        this->set_error(character.get_error());
        return (this->_error_code);
    }
    if (character->get_error() != ER_SUCCESS)
    {
        this->set_error(character->get_error());
        return (this->_error_code);
    }
    this->_characters.push_back(character);
    if (this->_characters.get_error() != ER_SUCCESS)
        this->set_error(this->_characters.get_error());
    else
        this->set_error(ER_SUCCESS);
    return (this->_error_code);
}

void ft_game_state::remove_character(size_t index) noexcept
{
    if (index >= this->_characters.size())
    {
        this->set_error(GAME_GENERAL_ERROR);
        return ;
    }
    this->_characters.erase(this->_characters.begin() + index);
    if (this->_characters.get_error() != ER_SUCCESS)
        this->set_error(this->_characters.get_error());
    else
        this->set_error(ER_SUCCESS);
    return ;
}

int ft_game_state::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_game_state::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void ft_game_state::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}
