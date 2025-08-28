#ifndef FT_BITSET_HPP
#define FT_BITSET_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cstddef>
#include <climits>

/*
 * ft_bitset
 * Fixed-size sequence of bits with basic operations and thread-safe
 * error reporting. Size is determined at construction and cannot change.
 */
class ft_bitset
{
    private:
        size_t      _size;
        size_t      _blockCount;
        size_t*     _data;
        mutable int _errorCode;
        mutable pt_mutex _mutex;

        static const size_t BITS_PER_BLOCK = sizeof(size_t) * CHAR_BIT;

        void    setError(int error) const;
        size_t  block_index(size_t pos) const;
        size_t  bit_mask(size_t pos) const;

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
};

inline size_t ft_bitset::block_index(size_t pos) const
{
    return (pos / BITS_PER_BLOCK);
}

inline size_t ft_bitset::bit_mask(size_t pos) const
{
    return (static_cast<size_t>(1) << (pos % BITS_PER_BLOCK));
}

inline void ft_bitset::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
    return ;
}

inline ft_bitset::ft_bitset(size_t bits)
    : _size(bits),
      _blockCount((bits + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK),
      _data(ft_nullptr),
      _errorCode(ER_SUCCESS)
{
    if (this->_blockCount > 0)
    {
        this->_data = static_cast<size_t*>(cma_malloc(sizeof(size_t) * this->_blockCount));
        if (this->_data == ft_nullptr)
        {
            this->setError(BITSET_ALLOC_FAIL);
            return ;
        }
        size_t i = 0;
        while (i < this->_blockCount)
            this->_data[i++] = 0;
    }
    return ;
}

inline ft_bitset::~ft_bitset()
{
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    return ;
}

inline ft_bitset::ft_bitset(ft_bitset&& other) noexcept
    : _size(other._size),
      _blockCount(other._blockCount),
      _data(other._data),
      _errorCode(other._errorCode)
{
    other._size = 0;
    other._blockCount = 0;
    other._data = ft_nullptr;
    other._errorCode = ER_SUCCESS;
    return ;
}

inline ft_bitset& ft_bitset::operator=(ft_bitset&& other) noexcept
{
    if (this != &other)
    {
        if (this->_mutex.lock(THREAD_ID) != SUCCES)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != SUCCES)
        {
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
        if (this->_data != ft_nullptr)
            cma_free(this->_data);
        this->_size = other._size;
        this->_blockCount = other._blockCount;
        this->_data = other._data;
        this->_errorCode = other._errorCode;
        other._size = 0;
        other._blockCount = 0;
        other._data = ft_nullptr;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

inline void ft_bitset::set(size_t pos)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (pos >= this->_size)
    {
        this->setError(BITSET_OUT_OF_RANGE);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    this->_data[block_index(pos)] |= bit_mask(pos);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

inline void ft_bitset::reset(size_t pos)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (pos >= this->_size)
    {
        this->setError(BITSET_OUT_OF_RANGE);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    this->_data[block_index(pos)] &= ~bit_mask(pos);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

inline void ft_bitset::flip(size_t pos)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (pos >= this->_size)
    {
        this->setError(BITSET_OUT_OF_RANGE);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    this->_data[block_index(pos)] ^= bit_mask(pos);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

inline bool ft_bitset::test(size_t pos) const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        const_cast<ft_bitset*>(this)->setError(PT_ERR_MUTEX_OWNER);
        return (false);
    }
    if (pos >= this->_size)
    {
        const_cast<ft_bitset*>(this)->setError(BITSET_OUT_OF_RANGE);
        this->_mutex.unlock(THREAD_ID);
        return (false);
    }
    bool res = (this->_data[block_index(pos)] & bit_mask(pos)) != 0;
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

inline size_t ft_bitset::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

inline void ft_bitset::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    size_t i = 0;
    while (i < this->_blockCount)
        this->_data[i++] = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

inline int ft_bitset::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

inline const char* ft_bitset::get_error_str() const
{
    return (ft_strerror(this->get_error()));
}

#endif
