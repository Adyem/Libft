#include "experience_table.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"
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
    for (int i = 1; i < count; i++)
    {
        if (array[i] <= array[i - 1])
            return (false);
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
    int *new_levels = static_cast<int*>(cma_realloc(this->_levels,
                    sizeof(int) * new_count));
    if (!new_levels)
    {
        this->set_error(CMA_BAD_ALLOC);
        return (this->_error);
    }
    if (new_count > this->_count)
    {
        for (int i = this->_count; i < new_count; i++)
            new_levels[i] = 0;
    }
    this->_levels = new_levels;
    this->_count = new_count;
    if (!this->is_valid(this->_count, this->_levels))
        this->set_error(CHARACTER_LEVEL_TABLE_INVALID);
    return (this->_error);
}

int ft_experience_table::get_level(int experience) const noexcept
{
    if (!this->_levels || this->_count == 0)
        return (0);
    int lvl = 0;
    while (lvl < this->_count && experience >= this->_levels[lvl])
        ++lvl;
    return (lvl);
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
    for (int i = 0; i < count; ++i)
        this->_levels[i] = levels[i];
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
    for (int i = 0; i < count; ++i)
    {
        this->_levels[i] = static_cast<int>(value);
        value *= multiplier;
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
    for (int i = 1; i < count; ++i)
    {
        increment *= multiplier;
        total += increment;
        this->_levels[i] = static_cast<int>(total);
    }
    if (!this->is_valid(this->_count, this->_levels))
        this->set_error(CHARACTER_LEVEL_TABLE_INVALID);
    return (this->_error);
}

int ft_experience_table::check_for_error() const noexcept
{
    if (!this->_levels)
        return (0);
    for (int i = 1; i < this->_count; i++)
    {
        if (this->_levels[i] <= this->_levels[i - 1])
        {
            const_cast<ft_experience_table*>(this)->set_error
				(CHARACTER_LEVEL_TABLE_INVALID);
            return (this->_levels[i]);
        }
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

