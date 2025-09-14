#include "game_state.hpp"
#include "../Template/move.hpp"

ft_game_state::ft_game_state() noexcept
    : _world(new (std::nothrow) ft_world()), _characters(), _error_code(ER_SUCCESS)
{
    if (!this->_world)
        this->set_error(GAME_GENERAL_ERROR);
    else if (this->_world.get_error() != ER_SUCCESS)
        this->set_error(this->_world.get_error());
    else if (this->_world->get_error() != ER_SUCCESS)
        this->set_error(this->_world->get_error());
    return ;
}

ft_game_state::~ft_game_state() noexcept
{
    return ;
}

ft_game_state::ft_game_state(const ft_game_state &other) noexcept
    : _world(other._world), _characters(), _error_code(other._error_code)
{
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
    if (!this->_world)
        this->set_error(GAME_GENERAL_ERROR);
    else if (this->_world.get_error() != ER_SUCCESS)
        this->set_error(this->_world.get_error());
    else if (this->_world->get_error() != ER_SUCCESS)
        this->set_error(this->_world->get_error());
    return ;
}

ft_game_state &ft_game_state::operator=(const ft_game_state &other) noexcept
{
    if (this != &other)
    {
        this->_world = other._world;
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
        if (!this->_world)
            this->set_error(GAME_GENERAL_ERROR);
        else if (this->_world.get_error() != ER_SUCCESS)
            this->set_error(this->_world.get_error());
        else if (this->_world->get_error() != ER_SUCCESS)
            this->set_error(this->_world->get_error());
    }
    return (*this);
}

ft_game_state::ft_game_state(ft_game_state &&other) noexcept
    : _world(ft_move(other._world)),
    _characters(ft_move(other._characters)),
    _error_code(other._error_code)
{
    if (!this->_world)
        this->set_error(GAME_GENERAL_ERROR);
    else if (this->_world.get_error() != ER_SUCCESS)
        this->set_error(this->_world.get_error());
    else if (this->_world->get_error() != ER_SUCCESS)
        this->set_error(this->_world->get_error());
    if (this->_characters.get_error() != ER_SUCCESS)
        this->set_error(this->_characters.get_error());
    other._error_code = ER_SUCCESS;
    return ;
}

ft_game_state &ft_game_state::operator=(ft_game_state &&other) noexcept
{
    if (this != &other)
    {
        this->_world = ft_move(other._world);
        this->_characters = ft_move(other._characters);
        this->_error_code = other._error_code;
        if (!this->_world)
            this->set_error(GAME_GENERAL_ERROR);
        else if (this->_world.get_error() != ER_SUCCESS)
            this->set_error(this->_world.get_error());
        else if (this->_world->get_error() != ER_SUCCESS)
            this->set_error(this->_world->get_error());
        if (this->_characters.get_error() != ER_SUCCESS)
            this->set_error(this->_characters.get_error());
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

ft_sharedptr<ft_world> &ft_game_state::get_world() noexcept
{
    return (this->_world);
}

ft_vector<ft_sharedptr<ft_character> > &ft_game_state::get_characters() noexcept
{
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
