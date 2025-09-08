#include "game_experience_table.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"
#include "../Errno/errno.hpp"

ft_experience_table::ft_experience_table(int count) noexcept
    : _levels(ft_nullptr), _count(0), _error(ER_SUCCESS)
{
    if (count > 0)
    {
        this->_levels = static_cast<int*>(cma_calloc(count, sizeof(int)));
        if (!this->_levels)
        {
            this->set_error(CMA_BAD_ALLOC);
            return ;
        }
        this->_count = count;
    }
    return ;
}

ft_experience_table::~ft_experience_table()
{
    if (this->_levels)
        cma_free(this->_levels);
    this->_levels = ft_nullptr;
    this->_count = 0;
    this->_error = ER_SUCCESS;
    return ;
}

void ft_experience_table::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

bool ft_experience_table::is_valid(int count, const int *array) const noexcept
{
    if (!array || count <= 1)
        return (true);
    int index = 1;
    while (index < count)
    {
        if (array[index] <= array[index - 1])
            return (false);
        ++index;
    }
    return (true);
}

int ft_experience_table::get_count() const noexcept
{
    return (this->_count);
}

int ft_experience_table::resize(int new_count) noexcept
{
    this->_error = ER_SUCCESS;
    if (new_count <= 0)
    {
        if (this->_levels)
            cma_free(this->_levels);
        this->_levels = ft_nullptr;
        this->_count = 0;
        return (ER_SUCCESS);
    }
    int old_count = this->_count;
    int *new_levels = static_cast<int*>(cma_realloc(this->_levels,
                    sizeof(int) * new_count));
    if (!new_levels)
    {
        this->set_error(CMA_BAD_ALLOC);
        return (this->_error);
    }
    if (new_count > old_count)
    {
        int index = old_count;
        while (index < new_count)
        {
            new_levels[index] = 0;
            ++index;
        }
    }
    this->_levels = new_levels;
    this->_count = new_count;
    int check_count;
    if (old_count < new_count)
        check_count = old_count;
    else
        check_count = new_count;
    if (!this->is_valid(check_count, this->_levels))
        this->set_error(CHARACTER_LEVEL_TABLE_INVALID);
    return (this->_error);
}

int ft_experience_table::get_level(int experience) const noexcept
{
    if (!this->_levels || this->_count == 0)
        return (0);
    int level = 0;
    while (level < this->_count && experience >= this->_levels[level])
        ++level;
    return (level);
}

int ft_experience_table::get_value(int index) const noexcept
{
    if (index < 0 || index >= this->_count || !this->_levels)
    {
        const_cast<ft_experience_table*>(this)->set_error(VECTOR_OUT_OF_BOUNDS);
        return (0);
    }
    return (this->_levels[index]);
}

void ft_experience_table::set_value(int index, int value) noexcept
{
    if (index < 0 || index >= this->_count || !this->_levels)
    {
        this->set_error(VECTOR_OUT_OF_BOUNDS);
        return ;
    }
    this->_levels[index] = value;
    if (!this->is_valid(this->_count, this->_levels))
        this->set_error(CHARACTER_LEVEL_TABLE_INVALID);
    return ;
}

int ft_experience_table::set_levels(const int *levels, int count) noexcept
{
    this->_error = ER_SUCCESS;
    if (count <= 0 || !levels)
        return (this->resize(0));
    if (this->resize(count) != ER_SUCCESS)
        return (this->_error);
    int level_index = 0;
    while (level_index < count)
    {
        this->_levels[level_index] = levels[level_index];
        ++level_index;
    }
    if (!this->is_valid(this->_count, this->_levels))
        this->set_error(CHARACTER_LEVEL_TABLE_INVALID);
    return (this->_error);
}

int ft_experience_table::generate_levels_total(int count, int base,
                                               double multiplier) noexcept
{
    this->_error = ER_SUCCESS;
    if (count <= 0)
        return (this->resize(0));
    if (this->resize(count) != ER_SUCCESS)
        return (this->_error);
    double value = static_cast<double>(base);
    int level_index = 0;
    while (level_index < count)
    {
        this->_levels[level_index] = static_cast<int>(value);
        value *= multiplier;
        ++level_index;
    }
    if (!this->is_valid(this->_count, this->_levels))
        this->set_error(CHARACTER_LEVEL_TABLE_INVALID);
    return (this->_error);
}

int ft_experience_table::generate_levels_scaled(int count, int base,
                                                double multiplier) noexcept
{
    this->_error = ER_SUCCESS;
    if (count <= 0)
        return (this->resize(0));
    if (this->resize(count) != ER_SUCCESS)
        return (this->_error);
    double increment = static_cast<double>(base);
    double total = static_cast<double>(base);
    this->_levels[0] = static_cast<int>(total);
    int index = 1;
    while (index < count)
    {
        increment *= multiplier;
        total += increment;
        this->_levels[index] = static_cast<int>(total);
        ++index;
    }
    if (!this->is_valid(this->_count, this->_levels))
        this->set_error(CHARACTER_LEVEL_TABLE_INVALID);
    return (this->_error);
}

int ft_experience_table::check_for_error() const noexcept
{
    if (!this->_levels)
        return (0);
    int index = 1;
    while (index < this->_count)
    {
        if (this->_levels[index] <= this->_levels[index - 1])
        {
            const_cast<ft_experience_table*>(this)->set_error
                (CHARACTER_LEVEL_TABLE_INVALID);
            return (this->_levels[index]);
        }
        ++index;
    }
    return (0);
}

int ft_experience_table::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_experience_table::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

