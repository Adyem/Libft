#include "game_experience_table.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <new>

ft_experience_table::ft_experience_table() noexcept
    : _levels(ft_nullptr), _count(0), _error(FT_ERR_SUCCESS), _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_experience_table::~ft_experience_table()
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_levels = ft_nullptr;
        this->_count = 0;
        this->set_error(lock_error);
        (void)this->disable_thread_safety();
        return ;
    }
    if (this->_levels)
        cma_free(this->_levels);
    this->_levels = ft_nullptr;
    this->_count = 0;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    (void)this->disable_thread_safety();
    return ;
}

int ft_experience_table::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_experience_table::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    if (this->_mutex == ft_nullptr)
        return ;
    (void)this->_mutex->unlock();
    return ;
}


void ft_experience_table::set_error(int err) const noexcept
{
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

int ft_experience_table::resize_locked(int new_count,
        bool validate_existing) noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    if (new_count <= 0)
    {
        if (this->_levels)
            cma_free(this->_levels);
        this->_levels = ft_nullptr;
        this->_count = 0;
        return (FT_ERR_SUCCESS);
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
    if (validate_existing)
    {
        int check_count;

        check_count = old_count;
        if (old_count > new_count)
            check_count = new_count;
        if (!this->is_valid(check_count, this->_levels))
        {
            this->set_error(FT_ERR_CONFIGURATION);
            return (this->_error);
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_experience_table::get_count() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int count_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(lock_error);
        return (0);
    }
    count_value = this->_count;
    const_cast<ft_experience_table *>(this)->set_error(this->_error);
    this->unlock_internal(lock_acquired);
    return (count_value);
}

int ft_experience_table::resize(int new_count) noexcept
{
    bool lock_acquired;
    int lock_error;
    int result;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    result = this->resize_locked(new_count);
    this->unlock_internal(lock_acquired);
    return (result);
}

int ft_experience_table::get_level(int experience) const noexcept
{
    bool lock_acquired;
    int lock_error;
    int level;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(lock_error);
        return (0);
    }
    if (!this->_levels || this->_count == 0)
    {
        const_cast<ft_experience_table *>(this)->set_error(FT_ERR_SUCCESS);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    level = 0;
    while (level < this->_count && experience >= this->_levels[level])
        ++level;
    const_cast<ft_experience_table *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (level);
}

int ft_experience_table::get_value(int index) const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(lock_error);
        return (0);
    }
    if (index < 0 || index >= this->_count || !this->_levels)
    {
        const_cast<ft_experience_table *>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    value = this->_levels[index];
    const_cast<ft_experience_table *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (value);
}

void ft_experience_table::set_value(int index, int value) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (index < 0 || index >= this->_count || !this->_levels)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_levels[index] = value;
    if (!this->is_valid(this->_count, this->_levels))
    {
        this->set_error(FT_ERR_CONFIGURATION);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_experience_table::set_levels(const int *levels, int count) noexcept
{
    bool lock_acquired;
    int lock_error;
    int resize_result;
    int level_index;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (count <= 0 || !levels)
    {
        resize_result = this->resize_locked(0);
        this->unlock_internal(lock_acquired);
        return (resize_result);
    }
    resize_result = this->resize_locked(count, false);
    if (resize_result != FT_ERR_SUCCESS)
    {
        this->unlock_internal(lock_acquired);
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
        this->unlock_internal(lock_acquired);
        return (this->_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_experience_table::generate_levels_total(int count, int base,
        double multiplier) noexcept
{
    bool lock_acquired;
    int lock_error;
    int resize_result;
    double value;
    int level_index;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (count <= 0)
    {
        resize_result = this->resize_locked(0);
        if (resize_result == FT_ERR_SUCCESS)
            this->set_error(FT_ERR_SUCCESS);
        this->unlock_internal(lock_acquired);
        return (resize_result);
    }
    resize_result = this->resize_locked(count, false);
    if (resize_result != FT_ERR_SUCCESS)
    {
        this->unlock_internal(lock_acquired);
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
        this->unlock_internal(lock_acquired);
        return (this->_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_experience_table::generate_levels_scaled(int count, int base,
        double multiplier) noexcept
{
    bool lock_acquired;
    int lock_error;
    int resize_result;
    double increment;
    double total;
    int index;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (count <= 0)
    {
        resize_result = this->resize_locked(0);
        this->unlock_internal(lock_acquired);
        return (resize_result);
    }
    resize_result = this->resize_locked(count, false);
    if (resize_result != FT_ERR_SUCCESS)
    {
        this->unlock_internal(lock_acquired);
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
        this->unlock_internal(lock_acquired);
        return (this->_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_experience_table::check_for_error() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int index;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(lock_error);
        return (lock_error);
    }
    if (!this->_levels)
    {
        const_cast<ft_experience_table *>(this)->set_error(FT_ERR_SUCCESS);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    index = 1;
    while (index < this->_count)
    {
        if (this->_levels[index] <= this->_levels[index - 1])
        {
            const_cast<ft_experience_table *>(this)->set_error(FT_ERR_CONFIGURATION);
            this->unlock_internal(lock_acquired);
            return (this->_levels[index]);
        }
        ++index;
    }
    const_cast<ft_experience_table *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (0);
}

int ft_experience_table::get_error() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(lock_error);
        return (lock_error);
    }
    error_code = this->_error;
    const_cast<ft_experience_table *>(this)->set_error(error_code);
    this->unlock_internal(lock_acquired);
    return (error_code);
}

const char *ft_experience_table::get_error_str() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_experience_table *>(this)->set_error(lock_error);
        return (ft_strerror(lock_error));
    }
    error_code = this->_error;
    const_cast<ft_experience_table *>(this)->set_error(error_code);
    this->unlock_internal(lock_acquired);
    return (ft_strerror(error_code));
}

int ft_experience_table::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_experience_table::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_experience_table::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}
