#ifndef FT_BITSET_HPP
#define FT_BITSET_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <climits>

class ft_bitset
{
    private:
        size_t          _size;
        size_t          _blockCount;
        size_t*         _data;
        mutable int     _error_code;
        mutable pt_mutex* _state_mutex;
        bool            _thread_safe_enabled;

        static const size_t BITS_PER_BLOCK = sizeof(size_t) * CHAR_BIT;

        void    set_error(int error) const;
        size_t  block_index(size_t pos) const;
        size_t  bit_mask(size_t pos) const;
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

    public:
        explicit ft_bitset(size_t bits = 0);
        ~ft_bitset();

        ft_bitset(const ft_bitset&) = delete;
        ft_bitset& operator=(const ft_bitset&) = delete;

        ft_bitset(ft_bitset&& other) noexcept;
        ft_bitset& operator=(ft_bitset&& other) noexcept;

        void    set(size_t pos);
        void    reset(size_t pos);
        void    flip(size_t pos);
        bool    test(size_t pos) const;
        size_t  size() const;
        void    clear();

        int         get_error() const;
        const char* get_error_str() const;
        int         enable_thread_safety();
        void        disable_thread_safety();
        bool        is_thread_safe_enabled() const;
        int         lock(bool *lock_acquired) const;
        void        unlock(bool lock_acquired) const;
        void       *get_mutex_address_debug() const;
};

inline size_t ft_bitset::block_index(size_t pos) const
{
    return (pos / BITS_PER_BLOCK);
}

inline size_t ft_bitset::bit_mask(size_t pos) const
{
    return (static_cast<size_t>(1) << (pos % BITS_PER_BLOCK));
}

inline void ft_bitset::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

inline ft_bitset::ft_bitset(size_t bits)
    : _size(bits),
      _blockCount((bits + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK),
      _data(ft_nullptr),
      _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (this->_blockCount > 0)
    {
        this->_data = static_cast<size_t*>(cma_malloc(sizeof(size_t) * this->_blockCount));
        if (this->_data == ft_nullptr)
        {
            this->set_error(FT_ERR_BITSET_NO_MEMORY);
            return ;
        }
        size_t i = 0;
        while (i < this->_blockCount)
            this->_data[i++] = 0;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_bitset::~ft_bitset()
{
    bool lock_acquired;
    size_t *data_pointer;

    lock_acquired = false;
    data_pointer = this->_data;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        this->_data = ft_nullptr;
        this->_size = 0;
        this->_blockCount = 0;
        this->unlock_internal(lock_acquired);
    }
    if (data_pointer != ft_nullptr)
        cma_free(data_pointer);
    this->teardown_thread_safety();
    return ;
}

inline ft_bitset::ft_bitset(ft_bitset&& other) noexcept
    : _size(0),
      _blockCount(0),
      _data(ft_nullptr),
      _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    size_t transferred_size;
    size_t transferred_block_count;
    size_t *transferred_data;
    int other_error_code;
    pt_mutex *transferred_mutex;
    bool transferred_thread_safe;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    transferred_size = other._size;
    transferred_block_count = other._blockCount;
    transferred_data = other._data;
    other_error_code = other._error_code;
    transferred_mutex = other._state_mutex;
    transferred_thread_safe = other._thread_safe_enabled;
    other._size = 0;
    other._blockCount = 0;
    other._data = ft_nullptr;
    other._error_code = ER_SUCCESS;
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other.unlock_internal(other_lock_acquired);
    this->_size = transferred_size;
    this->_blockCount = transferred_block_count;
    this->_data = transferred_data;
    this->_state_mutex = transferred_mutex;
    this->_thread_safe_enabled = transferred_thread_safe;
    this->_error_code = other_error_code;
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_bitset& ft_bitset::operator=(ft_bitset&& other) noexcept
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    size_t *previous_data;
    pt_mutex *previous_mutex;

    if (this == &other)
        return (*this);
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    previous_data = this->_data;
    previous_mutex = this->_state_mutex;
    this->_size = other._size;
    this->_blockCount = other._blockCount;
    this->_data = other._data;
    this->_error_code = other._error_code;
    this->_state_mutex = other._state_mutex;
    this->_thread_safe_enabled = other._thread_safe_enabled;
    other._size = 0;
    other._blockCount = 0;
    other._data = ft_nullptr;
    other._error_code = ER_SUCCESS;
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    if (previous_data != ft_nullptr && previous_data != this->_data)
        cma_free(previous_data);
    if (previous_mutex != ft_nullptr && previous_mutex != this->_state_mutex)
    {
        previous_mutex->~pt_mutex();
        cma_free(previous_mutex);
    }
    this->set_error(ER_SUCCESS);
    return (*this);
}

inline void ft_bitset::set(size_t pos)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (pos >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    this->_data[block_index(pos)] |= bit_mask(pos);
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return ;
}

inline void ft_bitset::reset(size_t pos)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (pos >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    this->_data[block_index(pos)] &= ~bit_mask(pos);
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return ;
}

inline void ft_bitset::flip(size_t pos)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (pos >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    this->_data[block_index(pos)] ^= bit_mask(pos);
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return ;
}

inline bool ft_bitset::test(size_t pos) const
{
    bool lock_acquired;
    bool result;

    lock_acquired = false;
    if (const_cast<ft_bitset*>(this)->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_bitset*>(this)->set_error(ft_errno);
        return (false);
    }
    if (pos >= this->_size)
    {
        const_cast<ft_bitset*>(this)->unlock_internal(lock_acquired);
        const_cast<ft_bitset*>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    result = (this->_data[block_index(pos)] & bit_mask(pos)) != 0;
    const_cast<ft_bitset*>(this)->unlock_internal(lock_acquired);
    const_cast<ft_bitset*>(this)->set_error(ER_SUCCESS);
    return (result);
}

inline size_t ft_bitset::size() const
{
    size_t current_size;
    bool lock_acquired;

    lock_acquired = false;
    if (const_cast<ft_bitset*>(this)->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_bitset*>(this)->set_error(ft_errno);
        return (0);
    }
    current_size = this->_size;
    const_cast<ft_bitset*>(this)->unlock_internal(lock_acquired);
    const_cast<ft_bitset*>(this)->set_error(ER_SUCCESS);
    return (current_size);
}

inline void ft_bitset::clear()
{
    bool lock_acquired;
    size_t index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    index = 0;
    while (index < this->_blockCount)
    {
        this->_data[index] = 0;
        index = index + 1;
    }
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return ;
}

inline int ft_bitset::get_error() const
{
    return (this->_error_code);
}

inline const char* ft_bitset::get_error_str() const
{
    return (ft_strerror(this->get_error()));
}

inline int ft_bitset::enable_thread_safety()
{
    void *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

inline void ft_bitset::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

inline bool ft_bitset::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<ft_bitset*>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

inline int ft_bitset::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_bitset*>(this)->set_error(ft_errno);
    else
        const_cast<ft_bitset*>(this)->set_error(ER_SUCCESS);
    return (result);
}

inline void ft_bitset::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != ER_SUCCESS)
        const_cast<ft_bitset*>(this)->set_error(this->_state_mutex->get_error());
    else
        const_cast<ft_bitset*>(this)->set_error(entry_errno);
    return ;
}

inline void *ft_bitset::get_mutex_address_debug() const
{
    return (this->_state_mutex);
}

inline int ft_bitset::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

inline void ft_bitset::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

inline void ft_bitset::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

#endif
