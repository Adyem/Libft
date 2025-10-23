#include "game_experience_table.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_experience_table_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_experience_table_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_experience_table::lock_pair(const ft_experience_table &first,
        const ft_experience_table &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_experience_table *ordered_first;
    const ft_experience_table *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_experience_table *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_experience_table_sleep_backoff();
    }
}

ft_experience_table::ft_experience_table(int count) noexcept
    : _levels(ft_nullptr), _count(0), _error(ER_SUCCESS), _mutex()
{
    if (count > 0)
    {
        this->_levels = static_cast<int*>(cma_calloc(count, sizeof(int)));
        if (!this->_levels)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_count = count;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

ft_experience_table::~ft_experience_table()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->_levels = ft_nullptr;
        this->_count = 0;
        this->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return ;
    }
    if (this->_levels)
        cma_free(this->_levels);
    this->_levels = ft_nullptr;
    this->_count = 0;
    this->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return ;
}

ft_experience_table::ft_experience_table(const ft_experience_table &other) noexcept
    : _levels(ft_nullptr), _count(0), _error(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_experience_table_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_count = other._count;
    if (this->_count > 0)
    {
        if (!other._levels)
        {
            this->_count = 0;
            this->set_error(FT_ERR_CONFIGURATION);
            game_experience_table_restore_errno(other_guard, entry_errno);
            return ;
        }
        this->_levels = static_cast<int*>(cma_calloc(this->_count, sizeof(int)));
        if (!this->_levels)
        {
            this->_count = 0;
            this->set_error(FT_ERR_NO_MEMORY);
            game_experience_table_restore_errno(other_guard, entry_errno);
            return ;
        }
        int index;

        index = 0;
        while (index < this->_count)
        {
            this->_levels[index] = other._levels[index];
            ++index;
        }
    }
    this->set_error(other._error);
    game_experience_table_restore_errno(other_guard, entry_errno);
    return ;
}

ft_experience_table &ft_experience_table::operator=(const ft_experience_table &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_experience_table::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    int *new_levels;

    new_levels = ft_nullptr;
    if (other._count > 0)
    {
        if (!other._levels)
        {
            this->set_error(FT_ERR_CONFIGURATION);
            game_experience_table_restore_errno(this_guard, entry_errno);
            game_experience_table_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        new_levels = static_cast<int*>(cma_calloc(other._count, sizeof(int)));
        if (!new_levels)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            game_experience_table_restore_errno(this_guard, entry_errno);
            game_experience_table_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        int index;

        index = 0;
        while (index < other._count)
        {
            new_levels[index] = other._levels[index];
            ++index;
        }
    }
    if (this->_levels)
        cma_free(this->_levels);
    this->_levels = new_levels;
    this->_count = other._count;
    this->set_error(other._error);
    game_experience_table_restore_errno(this_guard, entry_errno);
    game_experience_table_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_experience_table::ft_experience_table(ft_experience_table &&other) noexcept
    : _levels(ft_nullptr), _count(0), _error(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_experience_table_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_levels = other._levels;
    this->_count = other._count;
    this->_error = other._error;
    other._levels = ft_nullptr;
    other._count = 0;
    other._error = ER_SUCCESS;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    game_experience_table_restore_errno(other_guard, entry_errno);
    return ;
}

ft_experience_table &ft_experience_table::operator=(ft_experience_table &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_experience_table::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    if (this->_levels)
        cma_free(this->_levels);
    this->_levels = other._levels;
    this->_count = other._count;
    this->_error = other._error;
    other._levels = ft_nullptr;
    other._count = 0;
    other._error = ER_SUCCESS;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    game_experience_table_restore_errno(this_guard, entry_errno);
    game_experience_table_restore_errno(other_guard, entry_errno);
    return (*this);
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
    int index;

    index = 1;
    while (index < count)
    {
        if (array[index] <= array[index - 1])
            return (false);
        ++index;
    }
    return (true);
}

int ft_experience_table::resize_locked(int new_count, ft_unique_lock<pt_mutex> &guard) noexcept
{
    (void)guard;
    this->set_error(ER_SUCCESS);
    if (new_count <= 0)
    {
        if (this->_levels)
            cma_free(this->_levels);
        this->_levels = ft_nullptr;
        this->_count = 0;
        return (ER_SUCCESS);
    }
    int old_count;
    int *new_levels;

    old_count = this->_count;
    new_levels = static_cast<int*>(cma_realloc(this->_levels,
                sizeof(int) * new_count));
    if (!new_levels)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (this->_error);
    }
    if (new_count > old_count)
    {
        int index;

        index = old_count;
        while (index < new_count)
        {
            new_levels[index] = 0;
            ++index;
        }
    }
    this->_levels = new_levels;
    this->_count = new_count;
    int check_count;

    check_count = old_count;
    if (old_count > new_count)
        check_count = new_count;
    if (!this->is_valid(check_count, this->_levels))
    {
        this->set_error(FT_ERR_CONFIGURATION);
        return (this->_error);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_experience_table::get_count() const noexcept
{
    int entry_errno;
    int count_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (0);
    }
    count_value = this->_count;
    const_cast<ft_experience_table *>(this)->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return (count_value);
}

int ft_experience_table::resize(int new_count) noexcept
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    result = this->resize_locked(new_count, guard);
    game_experience_table_restore_errno(guard, entry_errno);
    return (result);
}

int ft_experience_table::get_level(int experience) const noexcept
{
    int entry_errno;
    int level;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (0);
    }
    if (!this->_levels || this->_count == 0)
    {
        const_cast<ft_experience_table *>(this)->set_error(ER_SUCCESS);
        game_experience_table_restore_errno(guard, entry_errno);
        return (0);
    }
    level = 0;
    while (level < this->_count && experience >= this->_levels[level])
        ++level;
    const_cast<ft_experience_table *>(this)->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return (level);
}

int ft_experience_table::get_value(int index) const noexcept
{
    int entry_errno;
    int value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (0);
    }
    if (index < 0 || index >= this->_count || !this->_levels)
    {
        const_cast<ft_experience_table *>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        game_experience_table_restore_errno(guard, entry_errno);
        return (0);
    }
    value = this->_levels[index];
    const_cast<ft_experience_table *>(this)->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return (value);
}

void ft_experience_table::set_value(int index, int value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return ;
    }
    if (index < 0 || index >= this->_count || !this->_levels)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        game_experience_table_restore_errno(guard, entry_errno);
        return ;
    }
    this->_levels[index] = value;
    if (!this->is_valid(this->_count, this->_levels))
    {
        this->set_error(FT_ERR_CONFIGURATION);
        game_experience_table_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return ;
}

int ft_experience_table::set_levels(const int *levels, int count) noexcept
{
    int entry_errno;
    int resize_result;
    int level_index;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->set_error(ER_SUCCESS);
    if (count <= 0 || !levels)
    {
        resize_result = this->resize_locked(0, guard);
        game_experience_table_restore_errno(guard, entry_errno);
        return (resize_result);
    }
    resize_result = this->resize_locked(count, guard);
    if (resize_result != ER_SUCCESS)
    {
        game_experience_table_restore_errno(guard, entry_errno);
        return (this->_error);
    }
    level_index = 0;
    while (level_index < count)
    {
        this->_levels[level_index] = levels[level_index];
        ++level_index;
    }
    if (!this->is_valid(this->_count, this->_levels))
    {
        this->set_error(FT_ERR_CONFIGURATION);
        game_experience_table_restore_errno(guard, entry_errno);
        return (this->_error);
    }
    this->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_experience_table::generate_levels_total(int count, int base,
        double multiplier) noexcept
{
    int entry_errno;
    int resize_result;
    double value;
    int level_index;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->set_error(ER_SUCCESS);
    if (count <= 0)
    {
        resize_result = this->resize_locked(0, guard);
        game_experience_table_restore_errno(guard, entry_errno);
        return (resize_result);
    }
    resize_result = this->resize_locked(count, guard);
    if (resize_result != ER_SUCCESS)
    {
        game_experience_table_restore_errno(guard, entry_errno);
        return (this->_error);
    }
    value = static_cast<double>(base);
    level_index = 0;
    while (level_index < count)
    {
        this->_levels[level_index] = static_cast<int>(value);
        value *= multiplier;
        ++level_index;
    }
    if (!this->is_valid(this->_count, this->_levels))
    {
        this->set_error(FT_ERR_CONFIGURATION);
        game_experience_table_restore_errno(guard, entry_errno);
        return (this->_error);
    }
    this->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_experience_table::generate_levels_scaled(int count, int base,
        double multiplier) noexcept
{
    int entry_errno;
    int resize_result;
    double increment;
    double total;
    int index;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->set_error(ER_SUCCESS);
    if (count <= 0)
    {
        resize_result = this->resize_locked(0, guard);
        game_experience_table_restore_errno(guard, entry_errno);
        return (resize_result);
    }
    resize_result = this->resize_locked(count, guard);
    if (resize_result != ER_SUCCESS)
    {
        game_experience_table_restore_errno(guard, entry_errno);
        return (this->_error);
    }
    increment = static_cast<double>(base);
    total = static_cast<double>(base);
    this->_levels[0] = static_cast<int>(total);
    index = 1;
    while (index < count)
    {
        increment *= multiplier;
        total += increment;
        this->_levels[index] = static_cast<int>(total);
        ++index;
    }
    if (!this->is_valid(this->_count, this->_levels))
    {
        this->set_error(FT_ERR_CONFIGURATION);
        game_experience_table_restore_errno(guard, entry_errno);
        return (this->_error);
    }
    this->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_experience_table::check_for_error() const noexcept
{
    int entry_errno;
    int index;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (!this->_levels)
    {
        const_cast<ft_experience_table *>(this)->set_error(ER_SUCCESS);
        game_experience_table_restore_errno(guard, entry_errno);
        return (0);
    }
    index = 1;
    while (index < this->_count)
    {
        if (this->_levels[index] <= this->_levels[index - 1])
        {
            const_cast<ft_experience_table *>(this)->set_error(FT_ERR_CONFIGURATION);
            game_experience_table_restore_errno(guard, entry_errno);
            return (this->_levels[index]);
        }
        ++index;
    }
    const_cast<ft_experience_table *>(this)->set_error(ER_SUCCESS);
    game_experience_table_restore_errno(guard, entry_errno);
    return (0);
}

int ft_experience_table::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_experience_table *>(this)->set_error(error_code);
    game_experience_table_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_experience_table::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(guard.get_error());
        game_experience_table_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_experience_table *>(this)->set_error(error_code);
    game_experience_table_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}
