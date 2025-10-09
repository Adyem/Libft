#ifndef FT_MAP_HPP
#define FT_MAP_HPP

#include "pair.hpp"
#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstddef>
#include "../PThread/mutex.hpp"
#include "../Libft/libft.hpp"
#include <utility>

/*
** Complexity and iterator invalidation guarantees:
** - size, empty: O(1) without invalidation.
** - insert: O(n) due to linear search and potential reallocation; resizing invalidates all iterators.
** - remove: O(n); invalidates iterators pointing to removed element and any iterators after compaction.
** - find: O(n); does not invalidate iterators.
** - clear: O(n); invalidates all iterators and references.
** Thread safety: synchronize external access; internal mutex preserves error state only.
*/
template <typename Key, typename MappedType>
class ft_map
{
    private:
        Pair<Key, MappedType>*  _data;
        size_t                  _capacity;
        size_t                  _size;
        mutable int             _error;
        mutable pt_mutex        _mutex;

        void    resize(size_t new_capacity);
        size_t  find_index(const Key& key) const;
        void    set_error(int error) const;

    public:
        ft_map(size_t initial_capacity = 10);
        ft_map(const ft_map& other);
        ft_map& operator=(const ft_map& other);
        ft_map(ft_map&& other) noexcept;
        ft_map& operator=(ft_map&& other) noexcept;
        ~ft_map();

        void        insert(const Key& key, const MappedType& value);
        void        insert(const Key& key, MappedType&& value);
        Pair<Key, MappedType> *find(const Key& key);
        void        remove(const Key& key);
        bool        empty() const;
        void        clear();
        size_t      size() const;
        size_t      capacity() const;
        int         get_error() const;
        const char* get_error_str() const;

        Pair<Key, MappedType>* end();
        MappedType &at(const Key& key);

        const Pair<Key, MappedType> *end() const;
        const MappedType &at(const Key& key) const;
        const Pair<Key, MappedType> *find(const Key& key) const;
};

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(size_t initial_capacity)
    : _capacity(initial_capacity), _size(0), _error(ER_SUCCESS)
{
    if (this->_capacity == 0)
    {
        this->_data = ft_nullptr;
        this->set_error(ER_SUCCESS);
        return ;
    }
    void* raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * this->_capacity);
    if (!raw_memory)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        this->_data = ft_nullptr;
        this->_capacity = 0;
        return ;
    }
    this->_data = static_cast<Pair<Key, MappedType>*>(raw_memory);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(const ft_map<Key, MappedType>& other)
    : _capacity(other._capacity), _size(other._size), _error(other._error)
{
    if (other._data != ft_nullptr && this->_size > 0)
    {
        void* raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * this->_capacity);
        if (!raw_memory)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            this->_data = ft_nullptr;
            this->_size = 0;
            this->_capacity = 0;
            return ;
        }
        this->_data = static_cast<Pair<Key, MappedType>*>(raw_memory);
        size_t index = 0;
        while (index < this->_size)
        {
            construct_at(&this->_data[index], other._data[index]);
            index++;
        }
    }
    else
        this->_data = ft_nullptr;
    this->set_error(this->_error);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>& ft_map<Key, MappedType>::operator=(const ft_map<Key, MappedType>& other)
{
    if (this != &other)
    {
        if (this->_data != ft_nullptr)
        {
            size_t index = 0;
            while (index < this->_size)
            {
                ::destroy_at(&this->_data[index]);
                index++;
            }
            cma_free(this->_data);
        }
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_error = other._error;
        if (other._data != ft_nullptr && other._size > 0)
        {
            void* raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * other._capacity);
            if (!raw_memory)
            {
                this->set_error(FT_ERR_NO_MEMORY);
                this->_data = ft_nullptr;
                this->_size = 0;
                this->_capacity = 0;
                return (*this);
            }
            this->_data = static_cast<Pair<Key, MappedType>*>(raw_memory);
            size_t index = 0;
            while (index < other._size)
            {
                construct_at(&this->_data[index], other._data[index]);
                index++;
            }
        }
        else
            this->_data = ft_nullptr;
    }
    this->set_error(this->_error);
    return (*this);
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(ft_map<Key, MappedType>&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size),
      _error(other._error)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error = ER_SUCCESS;
    this->set_error(this->_error);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>& ft_map<Key, MappedType>::operator=(ft_map<Key, MappedType>&& other)
        noexcept
{
    if (this != &other)
    {
        if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(FT_ERR_MUTEX_NOT_OWNER);
            return (*this);
        }
        if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(FT_ERR_MUTEX_NOT_OWNER);
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
        if (this->_data != ft_nullptr)
        {
            size_t index = 0;
            while (index < this->_size)
            {
                ::destroy_at(&this->_data[index]);
                index++;
            }
            cma_free(this->_data);
        }
        this->_data = other._data;
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_error = other._error;
        other._data = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    this->set_error(this->_error);
    return (*this);
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::~ft_map()
{
    if (this->_data != ft_nullptr)
    {
        size_t index = 0;
        while (index < this->_size)
        {
            ::destroy_at(&this->_data[index]);
            index++;
        }
        cma_free(this->_data);
    }
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::insert(const Key& key, const MappedType& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    this->set_error(ER_SUCCESS);
    size_t index = find_index(key);
    if (index != this->_size)
    {
        this->_data[index].value = value;
        this->set_error(ER_SUCCESS);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        size_t next_capacity;
        if (this->_capacity == 0)
            next_capacity = 1;
        else
        {
            size_t doubled_capacity;

            doubled_capacity = this->_capacity * 2;
            if (doubled_capacity <= this->_capacity)
            {
                this->set_error(FT_ERR_NO_MEMORY);
                this->_mutex.unlock(THREAD_ID);
                return ;
            }
            next_capacity = doubled_capacity;
        }
        resize(next_capacity);
        if (this->_error != ER_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], Pair<Key, MappedType>(key, value));
    ++this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::insert(const Key& key, MappedType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    this->set_error(ER_SUCCESS);
    size_t index = find_index(key);
    if (index != this->_size)
    {
        this->_data[index].value = std::move(value);
        this->set_error(ER_SUCCESS);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        size_t next_capacity;
        if (this->_capacity == 0)
            next_capacity = 1;
        else
        {
            size_t doubled_capacity;

            doubled_capacity = this->_capacity * 2;
            if (doubled_capacity <= this->_capacity)
            {
                this->set_error(FT_ERR_NO_MEMORY);
                this->_mutex.unlock(THREAD_ID);
                return ;
            }
            next_capacity = doubled_capacity;
        }
        resize(next_capacity);
        if (this->_error != ER_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], Pair<Key, MappedType>(key, std::move(value)));
    ++this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename Key, typename MappedType>
Pair<Key, MappedType> *ft_map<Key, MappedType>::find(const Key& key)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    size_t index = 0;
    while (index < this->_size)
    {
        if (this->_data[index].key == key)
        {
            Pair<Key, MappedType>* result = &this->_data[index];
            this->set_error(ER_SUCCESS);
            this->_mutex.unlock(THREAD_ID);
            return (result);
        }
        index++;
    }
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ft_nullptr);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType> *ft_map<Key, MappedType>::find(const Key& key) const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    size_t index = 0;
    while (index < this->_size)
    {
        if (this->_data[index].key == key)
        {
            const Pair<Key, MappedType>* result = &this->_data[index];
            this->set_error(ER_SUCCESS);
            this->_mutex.unlock(THREAD_ID);
            return (result);
        }
        index++;
    }
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ft_nullptr);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::remove(const Key& key)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    size_t index = 0;
    while (index < this->_size)
    {
        if (this->_data[index].key == key)
        {
            ::destroy_at(&this->_data[index]);
            if (index != this->_size - 1)
            {
                construct_at(&this->_data[index], std::move(this->_data[this->_size - 1]));
                ::destroy_at(&this->_data[this->_size - 1]);
            }
            --this->_size;
            this->set_error(ER_SUCCESS);
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
        index++;
    }
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (true);
    }
    bool result = (this->_size == 0);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    size_t index = 0;
    while (index < this->_size)
    {
        ::destroy_at(&this->_data[index]);
        index++;
    }
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (0);
    }
    size_t current_size = this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (current_size);
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::capacity() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (0);
    }
    size_t current_capacity = this->_capacity;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (current_capacity);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (_error);
    }
    int error_value = this->_error;
    this->set_error(error_value);
    this->_mutex.unlock(THREAD_ID);
    return (error_value);
}

template <typename Key, typename MappedType>
const char* ft_map<Key, MappedType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_strerror(_error));
    }
    int error_value = this->_error;
    this->set_error(error_value);
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(error_value));
}

template<typename Key, typename MappedType>
void ft_map<Key, MappedType>::set_error(int error) const
{
    ft_errno = error;
    this->_error = error;
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::resize(size_t new_capacity)
{
    this->set_error(ER_SUCCESS);
    void* raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * new_capacity);
    if (!raw_memory)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    Pair<Key, MappedType>* new_data = static_cast<Pair<Key, MappedType>*>(raw_memory);
    size_t index = 0;
    while (index < this->_size)
    {
        construct_at(&new_data[index], std::move(this->_data[index]));
        ::destroy_at(&this->_data[index]);
        index++;
    }
    cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::find_index(const Key& key) const
{
    size_t index = 0;
    while  (index < this->_size)
    {
        if (this->_data[index].key == key)
            return (index);
        index++;
    }
    return (this->_size);
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::end()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (this->_data + this->_size);
    }
    Pair<Key, MappedType>* result = this->_data + this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::end() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (this->_data + this->_size);
    }
    const Pair<Key, MappedType>* result = this->_data + this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename Key, typename MappedType>
MappedType& ft_map<Key, MappedType>::at(const Key& key)
{
    static MappedType error_mapped_type = MappedType();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (error_mapped_type);
    }
    size_t index = find_index(key);
    if (this->_size == 0 || index == this->_size)
    {
        this->set_error(FT_ERR_INTERNAL);
        this->_mutex.unlock(THREAD_ID);
        return (error_mapped_type);
    }
    MappedType& value = this->_data[index].value;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename Key, typename MappedType>
const MappedType& ft_map<Key, MappedType>::at(const Key& key) const
{
    static MappedType error_mapped_type = MappedType();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (error_mapped_type);
    }
    size_t index = find_index(key);
    if (this->_size == 0 || index == this->_size)
    {
        this->set_error(FT_ERR_INTERNAL);
        this->_mutex.unlock(THREAD_ID);
        return (error_mapped_type);
    }
    const MappedType& value = this->_data[index].value;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

#endif
