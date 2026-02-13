#ifndef FT_BITSET_HPP
#define FT_BITSET_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <climits>

class ft_bitset
{
    private:
        static const size_t BITS_PER_BLOCK = sizeof(size_t) * CHAR_BIT;

        size_t              _size;
        size_t              _block_count;
        size_t*             _data;
        mutable pt_recursive_mutex* _mutex;

        size_t  block_index(size_t position) const;
        size_t  bit_mask(size_t position) const;
        void    destroy_data();
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

    public:
        explicit ft_bitset(size_t bits = 0);
        ~ft_bitset();

        ft_bitset(const ft_bitset&) = delete;
        ft_bitset& operator=(const ft_bitset&) = delete;

        ft_bitset(ft_bitset&& other) noexcept;
        ft_bitset& operator=(ft_bitset&& other) noexcept;

        void    set(size_t position);
        void    reset(size_t position);
        void    flip(size_t position);
        bool    test(size_t position) const;
        size_t  size() const;
        void    clear();

        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

inline size_t ft_bitset::block_index(size_t position) const
{
    return (position / BITS_PER_BLOCK);
}

inline size_t ft_bitset::bit_mask(size_t position) const
{
    return (static_cast<size_t>(1) << (position % BITS_PER_BLOCK));
}

inline void ft_bitset::destroy_data()
{
    if (this->_data != ft_nullptr)
    {
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->_size = 0;
    this->_block_count = 0;
}

inline ft_bitset::ft_bitset(size_t bits)
    : _size(bits),
      _block_count((bits + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK),
      _data(ft_nullptr),
      _mutex(ft_nullptr)
{
    if (this->_block_count > 0)
    {
        this->_data = static_cast<size_t*>(cma_malloc(sizeof(size_t) * this->_block_count));
        if (this->_data == ft_nullptr)
        {
            this->_size = 0;
            this->_block_count = 0;
            ft_global_error_stack_push(FT_ERR_BITSET_NO_MEMORY);
            return ;
        }
        size_t index = 0;

        while (index < this->_block_count)
        {
            this->_data[index] = 0;
            index = index + 1;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

inline ft_bitset::~ft_bitset()
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error == FT_ERR_SUCCESS)
    {
        this->destroy_data();
        this->unlock_internal(lock_acquired);
    }
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

inline ft_bitset::ft_bitset(ft_bitset&& other) noexcept
    : _size(0),
      _block_count(0),
      _data(ft_nullptr),
      _mutex(ft_nullptr)
{
    bool lock_acquired = false;
    int lock_error = other.lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_data = other._data;
    this->_size = other._size;
    this->_block_count = other._block_count;
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other._data = ft_nullptr;
    other._size = 0;
    other._block_count = 0;
    other.unlock_internal(lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

inline ft_bitset& ft_bitset::operator=(ft_bitset&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    bool this_lock_acquired = false;
    bool other_lock_acquired = false;
    int lock_error = this->lock_internal(&this_lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->destroy_data();
    bool previous_thread_safe = (this->_mutex != ft_nullptr);
    this->teardown_thread_safety();
    this->_data = other._data;
    this->_size = other._size;
    this->_block_count = other._block_count;
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other._data = ft_nullptr;
    other._size = 0;
    other._block_count = 0;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    this->unlock_internal(this_lock_acquired);
    if (previous_thread_safe)
        this->disable_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

inline void ft_bitset::set(size_t position)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    if (position >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    this->_data[this->block_index(position)] |= this->bit_mask(position);
    int unlock_result = this->unlock_internal(lock_acquired);

    if (unlock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_result);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

inline void ft_bitset::reset(size_t position)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    if (position >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    this->_data[this->block_index(position)] &= ~this->bit_mask(position);
    int unlock_result = this->unlock_internal(lock_acquired);

    if (unlock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_result);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

inline void ft_bitset::flip(size_t position)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    if (position >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    this->_data[this->block_index(position)] ^= this->bit_mask(position);
    int unlock_result = this->unlock_internal(lock_acquired);

    if (unlock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_result);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

inline bool ft_bitset::test(size_t position) const
{
    bool lock_acquired = false;
    int lock_result = const_cast<ft_bitset*>(this)->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (false);
    }
    if (position >= this->_size)
    {
        const_cast<ft_bitset*>(this)->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    bool result = (this->_data[this->block_index(position)] & this->bit_mask(position)) != 0;
    int unlock_result = const_cast<ft_bitset*>(this)->unlock_internal(lock_acquired);

    if (unlock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_result);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}

inline size_t ft_bitset::size() const
{
    bool lock_acquired = false;
    int lock_result = const_cast<ft_bitset*>(this)->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    size_t current_size = this->_size;
    int unlock_result = const_cast<ft_bitset*>(this)->unlock_internal(lock_acquired);

    if (unlock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_result);
        return (0);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (current_size);
}

inline void ft_bitset::clear()
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    size_t index = 0;

    while (index < this->_block_count)
    {
        this->_data[index] = 0;
        index = index + 1;
    }
    int unlock_result = this->unlock_internal(lock_acquired);

    if (unlock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_result);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

inline int ft_bitset::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

inline void ft_bitset::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline bool ft_bitset::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

inline int ft_bitset::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);

    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

inline void ft_bitset::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(result);
    return ;
}

inline int ft_bitset::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result == FT_ERR_SUCCESS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (result);
}

inline int ft_bitset::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

inline int ft_bitset::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);
    if (result != FT_ERR_SUCCESS && this->_mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&this->_mutex);
    return (result);
}

inline void ft_bitset::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

#ifdef LIBFT_TEST_BUILD
inline pt_recursive_mutex* ft_bitset::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
