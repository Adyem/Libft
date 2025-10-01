#ifndef FT_UNORD_MAP_HPP
#define FT_UNORD_MAP_HPP

#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "constructor.hpp"
#include <cstddef>
#include <functional>
#include "../PThread/mutex.hpp"
#include "../Libft/libft.hpp"
#include "move.hpp"

template <typename Key, typename MappedType>
struct ft_pair
{
    Key         first;
    MappedType  second;
    ft_pair();
    ft_pair(const Key& k, const MappedType& m);
};

template <typename Key, typename MappedType>
class ft_unord_map
{
    private:
        ft_pair<Key, MappedType>*  _data;
        bool*                      _occupied;
        size_t                     _capacity;
        size_t                     _size;
        mutable int                _error;
        mutable pt_mutex           _mutex;

        void    resize(size_t newCapacity);
        size_t  findIndex(const Key& key) const;
        void    set_error(int error) const;
        size_t  hashKey(const Key& key) const;
        void    insert_internal(const Key& key, const MappedType& value);
        bool    has_storage() const;
        void    flag_storage_error() const;

    public:
        class iterator
        {
            private:
                ft_pair<Key, MappedType>* _data;
                bool*                     _occupied;
                size_t                    _index;
                size_t                    _capacity;
                void                      skipUnoccupied();

            public:
                iterator(ft_pair<Key, MappedType>* data, bool* occ, size_t idx, size_t cap);
                ft_pair<Key, MappedType>& operator*() const;
                ft_pair<Key, MappedType>* operator->() const;
                iterator& operator++();
                bool operator==(const iterator& other) const;
                bool operator!=(const iterator& other) const;
        };

        class const_iterator
        {
            private:
                const ft_pair<Key, MappedType>* _data;
                const bool*                     _occupied;
                size_t                          _index;
                size_t                          _capacity;
                void                            skipUnoccupied();

            public:
                const_iterator(const ft_pair<Key, MappedType>* data, const bool* occ, size_t idx, size_t cap);
                const ft_pair<Key, MappedType>& operator*() const;
                const ft_pair<Key, MappedType>* operator->() const;
                const_iterator& operator++();
                bool operator==(const const_iterator& other) const;
                bool operator!=(const const_iterator& other) const;
        };

        ft_unord_map(size_t initialCapacity = 10);
        ft_unord_map(const ft_unord_map& other);
        ft_unord_map& operator=(const ft_unord_map& other);
        ft_unord_map(ft_unord_map&& other) noexcept;
        ft_unord_map& operator=(ft_unord_map&& other) noexcept;
        ~ft_unord_map();

        void           insert(const Key& key, const MappedType& value);
        iterator       find(const Key& key);
        const_iterator find(const Key& key) const;
        void           remove(const Key& key);
        bool           empty() const;
        void           clear();
        size_t         getSize() const;
        size_t         getCapacity() const;
        int            get_error() const;
        const char*    get_error_str() const;
        iterator       begin();
        iterator       end();
        const_iterator begin() const;
        const_iterator end() const;
        MappedType&    at(const Key& key);
        const MappedType& at(const Key& key) const;
        MappedType&    operator[](const Key& key);
};

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>::ft_pair() : first(), second()
{
    return ;
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>::ft_pair(const Key& k, const MappedType& m) : first(k), second(m)
{
    return ;
}

template <typename Key, typename MappedType>
ft_unord_map<Key, MappedType>::iterator::iterator(ft_pair<Key, MappedType>* data, bool* occ, size_t idx, size_t cap)
    : _data(data), _occupied(occ), _index(idx), _capacity(cap)
{
    skipUnoccupied();
    return ;
}

template <typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::iterator::skipUnoccupied()
{
    while (_index < _capacity && !_occupied[_index])
        ++_index;
    return ;
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>& ft_unord_map<Key, MappedType>::iterator::operator*() const
{
    return (_data[_index]);
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>* ft_unord_map<Key, MappedType>::iterator::operator->() const
{
    return (&_data[_index]);
}

template <typename Key, typename MappedType>
typename ft_unord_map<Key, MappedType>::iterator& ft_unord_map<Key, MappedType>::iterator::operator++()
{
    ++_index;
    skipUnoccupied();
    return (*this);
}

template <typename Key, typename MappedType>
bool ft_unord_map<Key, MappedType>::iterator::operator==(const iterator& other) const
{
    return (_data == other._data && _index == other._index);
}

template <typename Key, typename MappedType>
bool ft_unord_map<Key, MappedType>::iterator::operator!=(const iterator& other) const
{
    return (!(*this == other));
}

template <typename Key, typename MappedType>
ft_unord_map<Key, MappedType>::const_iterator::const_iterator(const ft_pair<Key, MappedType>* data, const bool* occ, size_t idx, size_t cap)
    : _data(data), _occupied(occ), _index(idx), _capacity(cap)
{
    skipUnoccupied();
    return ;
}

template <typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::const_iterator::skipUnoccupied()
{
    while (_index < _capacity && !_occupied[_index])
        ++_index;
    return ;
}

template <typename Key, typename MappedType>
const ft_pair<Key, MappedType>& ft_unord_map<Key, MappedType>::const_iterator::operator*() const
{
    return (_data[_index]);
}

template <typename Key, typename MappedType>
const ft_pair<Key, MappedType>* ft_unord_map<Key, MappedType>::const_iterator::operator->() const
{
    return (&_data[_index]);
}

template <typename Key, typename MappedType>
typename ft_unord_map<Key, MappedType>::const_iterator& ft_unord_map<Key, MappedType>::const_iterator::operator++()
{
    ++_index;
    skipUnoccupied();
    return (*this);
}

template <typename Key, typename MappedType>
bool ft_unord_map<Key, MappedType>::const_iterator::operator==(const const_iterator& other) const
{
    return (_data == other._data && _index == other._index);
}

template <typename Key, typename MappedType>
bool ft_unord_map<Key, MappedType>::const_iterator::operator!=(const const_iterator& other) const
{
    return (!(*this == other));
}

template <typename Key, typename MappedType>
ft_unord_map<Key, MappedType>::ft_unord_map(size_t initialCapacity)
    : _capacity(initialCapacity), _size(0), _error(ER_SUCCESS)
{
    void* rawData = cma_malloc(sizeof(ft_pair<Key, MappedType>) * _capacity);
    if (!rawData)
    {
        set_error(UNORD_MAP_MEMORY);
        _data = ft_nullptr;
        _occupied = ft_nullptr;
        return ;
    }
    _data = static_cast<ft_pair<Key, MappedType>*>(rawData);
    void* rawOccupied = cma_malloc(sizeof(bool) * _capacity);
    if (!rawOccupied)
    {
        set_error(UNORD_MAP_MEMORY);
        cma_free(_data);
        _data = ft_nullptr;
        _occupied = ft_nullptr;
        return ;
    }
    _occupied = static_cast<bool*>(rawOccupied);
    size_t i = 0;
    while (i < _capacity)
    {
        _occupied[i] = false;
        i++;
    }
}

template <typename Key, typename MappedType>
ft_unord_map<Key, MappedType>::ft_unord_map(const ft_unord_map<Key, MappedType>& other)
    : _capacity(other._capacity), _size(other._size), _error(other._error)
{
    if (other._data != ft_nullptr && _size > 0)
    {
        void* rawData = cma_malloc(sizeof(ft_pair<Key, MappedType>) * _capacity);
        if (!rawData)
        {
            set_error(UNORD_MAP_MEMORY);
            _data = ft_nullptr;
            _occupied = ft_nullptr;
            _size = 0;
            _capacity = 0;
            return ;
        }
        _data = static_cast<ft_pair<Key, MappedType>*>(rawData);
        void* rawOccupied = cma_malloc(sizeof(bool) * _capacity);
        if (!rawOccupied)
        {
            set_error(UNORD_MAP_MEMORY);
            cma_free(_data);
            _data = ft_nullptr;
            _occupied = ft_nullptr;
            _size = 0;
            _capacity = 0;
            return ;
        }
        _occupied = static_cast<bool*>(rawOccupied);
        size_t i = 0;
        while (i < _capacity)
        {
            _occupied[i] = false;
            i++;
        }
        size_t index = 0;
        size_t count = 0;
        while (count < other._size && index < other._capacity)
        {
            if (other._occupied[index])
            {
                construct_at(&_data[index], other._data[index]);
                _occupied[index] = true;
                count++;
            }
            index++;
        }
    }
    else
    {
        _data = ft_nullptr;
        _occupied = ft_nullptr;
    }
}

template <typename Key, typename MappedType>
ft_unord_map<Key, MappedType>& ft_unord_map<Key, MappedType>::operator=(const ft_unord_map<Key, MappedType>& other)
{
    if (this != &other)
    {
        if (_data != ft_nullptr)
        {
            size_t i = 0;
            size_t count = 0;
            while (count < _size && i < _capacity)
            {
                if (_occupied[i])
                {
                    ::destroy_at(&_data[i]);
                    count++;
                }
                i++;
            }
            cma_free(_data);
            cma_free(_occupied);
        }
        _capacity = other._capacity;
        _size = other._size;
        _error = other._error;
        if (other._data != ft_nullptr && other._size > 0)
        {
            void* rawData = cma_malloc(sizeof(ft_pair<Key, MappedType>) * other._capacity);
            if (!rawData)
            {
                set_error(UNORD_MAP_MEMORY);
                _data = ft_nullptr;
                _occupied = ft_nullptr;
                _size = 0;
                _capacity = 0;
                return (*this);
            }
            _data = static_cast<ft_pair<Key, MappedType>*>(rawData);
            void* rawOccupied = cma_malloc(sizeof(bool) * other._capacity);
            if (!rawOccupied)
            {
                set_error(UNORD_MAP_MEMORY);
                cma_free(_data);
                _data = ft_nullptr;
                _occupied = ft_nullptr;
                _size = 0;
                _capacity = 0;
                return (*this);
            }
            _occupied = static_cast<bool*>(rawOccupied);
            size_t i = 0;
            while (i < _capacity)
            {
                _occupied[i] = false;
                i++;
            }
            size_t index = 0;
            size_t count = 0;
            while (count < other._size && index < other._capacity)
            {
                if (other._occupied[index])
                {
                    construct_at(&_data[index], other._data[index]);
                    _occupied[index] = true;
                    count++;
                }
                index++;
            }
        }
        else
        {
            _data = ft_nullptr;
            _occupied = ft_nullptr;
        }
    }
    return (*this);
}

template <typename Key, typename MappedType>
ft_unord_map<Key, MappedType>::ft_unord_map(ft_unord_map<Key, MappedType>&& other) noexcept
    : _data(other._data), _occupied(other._occupied), _capacity(other._capacity), _size(other._size), _error(other._error)
{
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error = ER_SUCCESS;
}

template <typename Key, typename MappedType>
ft_unord_map<Key, MappedType>& ft_unord_map<Key, MappedType>::operator=(ft_unord_map<Key, MappedType>&& other) noexcept
{
    if (this != &other)
    {
        if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
        if (_data != ft_nullptr)
        {
            size_t i = 0;
            size_t count = 0;
            while (count < _size && i < _capacity)
            {
                if (_occupied[i])
                {
                    ::destroy_at(&_data[i]);
                    count++;
                }
                i++;
            }
            cma_free(_data);
            cma_free(_occupied);
        }
        _data = other._data;
        _occupied = other._occupied;
        _capacity = other._capacity;
        _size = other._size;
        _error = other._error;
        other._data = ft_nullptr;
        other._occupied = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename Key, typename MappedType>
ft_unord_map<Key, MappedType>::~ft_unord_map()
{
    if (_data != ft_nullptr)
    {
        size_t i = 0;
        size_t count = 0;
        while (count < _size && i < _capacity)
        {
            if (_occupied[i])
            {
                ::destroy_at(&_data[i]);
                count++;
            }
            i++;
        }
        cma_free(_data);
        cma_free(_occupied);
    }
}

template<typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::set_error(int error) const
{
    ft_errno = error;
    _error = error;
    return ;
}

template <typename Key, typename MappedType>
bool ft_unord_map<Key, MappedType>::has_storage() const
{
    if (_capacity == 0)
        return (false);
    if (_data == ft_nullptr)
        return (false);
    if (_occupied == ft_nullptr)
        return (false);
    return (true);
}

template <typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::flag_storage_error() const
{
    if (_error == ER_SUCCESS)
        set_error(UNORD_MAP_MEMORY);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_unord_map<Key, MappedType>::hashKey(const Key& key) const
{
    static std::hash<Key> hf;
    return (hf(key) % _capacity);
}

template <typename Key, typename MappedType>
size_t ft_unord_map<Key, MappedType>::findIndex(const Key& key) const
{
    if (_capacity == 0)
        return (_capacity);
    if (_data == ft_nullptr)
        return (_capacity);
    if (_occupied == ft_nullptr)
        return (_capacity);
    if (_size == 0)
        return (_capacity);
    size_t start = hashKey(key);
    size_t i = start;
    while (true)
    {
        if (!_occupied[i])
            return (_capacity);
        if (_data[i].first == key)
            return (i);
        i = (i + 1) % _capacity;
        if (i == start)
            return (_capacity);
    }
}

template <typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::resize(size_t newCapacity)
{
    if (newCapacity == 0)
    {
        set_error(UNORD_MAP_MEMORY);
        return ;
    }
    _error = ER_SUCCESS;
    void* rawData = cma_malloc(sizeof(ft_pair<Key, MappedType>) * newCapacity);
    if (!rawData)
    {
        set_error(UNORD_MAP_MEMORY);
        return ;
    }
    ft_pair<Key, MappedType>* newData = static_cast<ft_pair<Key, MappedType>*>(rawData);
    void* rawOccupied = cma_malloc(sizeof(bool) * newCapacity);
    if (!rawOccupied)
    {
        set_error(UNORD_MAP_MEMORY);
        cma_free(newData);
        return ;
    }
    bool* newOcc = static_cast<bool*>(rawOccupied);
    size_t i = 0;
    while (i < newCapacity)
    {
        newOcc[i] = false;
        i++;
    }
    size_t oldCap = _capacity;
    ft_pair<Key, MappedType>* oldData = _data;
    bool* oldOcc = _occupied;
    _data = newData;
    _occupied = newOcc;
    _capacity = newCapacity;
    size_t oldSize = _size;
    _size = 0;
    i = 0;
    size_t count = 0;
    while (count < oldSize && i < oldCap)
    {
        if (oldOcc[i])
        {
            insert_internal(oldData[i].first, oldData[i].second);
            ::destroy_at(&oldData[i]);
            count++;
        }
        i++;
    }
    cma_free(oldData);
    cma_free(oldOcc);
    if (_error == ER_SUCCESS)
        set_error(ER_SUCCESS);
    return ;
}

template <typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::insert_internal(const Key& key, const MappedType& value)
{
    if (_capacity == 0)
    {
        flag_storage_error();
        return ;
    }
    if (_data == ft_nullptr)
    {
        flag_storage_error();
        return ;
    }
    if (_occupied == ft_nullptr)
    {
        flag_storage_error();
        return ;
    }
    size_t idx = findIndex(key);
    if (idx != _capacity)
    {
        _data[idx].second = value;
        set_error(ER_SUCCESS);
        return ;
    }
    if ((_size * 2) >= _capacity)
    {
        resize(_capacity * 2);
        if (_error != ER_SUCCESS)
            return ;
    }
    size_t start = hashKey(key);
    size_t i = start;
    while (true)
    {
        if (!_occupied[i])
        {
            construct_at(&_data[i], ft_pair<Key, MappedType>(key, value));
            _occupied[i] = true;
            ++_size;
            set_error(ER_SUCCESS);
            return ;
        }
        i = (i + 1) % _capacity;
        if (i == start)
            return ;
    }
    return ;
}

template <typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::insert(const Key& key, const MappedType& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return ;
    }
    if (!has_storage())
    {
        flag_storage_error();
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    _error = ER_SUCCESS;
    insert_internal(key, value);
    if (_error == ER_SUCCESS)
        set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename Key, typename MappedType>
typename ft_unord_map<Key, MappedType>::iterator ft_unord_map<Key, MappedType>::find(const Key& key)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (iterator(_data, _occupied, _capacity, _capacity));
    }
    if (!has_storage())
    {
        flag_storage_error();
        iterator res(_data, _occupied, _capacity, _capacity);
        this->_mutex.unlock(THREAD_ID);
        return (res);
    }
    size_t idx = findIndex(key);
    if (idx == _capacity)
    {
        iterator res(_data, _occupied, _capacity, _capacity);
        set_error(ER_SUCCESS);
        this->_mutex.unlock(THREAD_ID);
        return (res);
    }
    iterator res(_data, _occupied, idx, _capacity);
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unord_map<Key, MappedType>::const_iterator ft_unord_map<Key, MappedType>::find(const Key& key) const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (const_iterator(_data, _occupied, _capacity, _capacity));
    }
    if (!has_storage())
    {
        flag_storage_error();
        const_iterator res(_data, _occupied, _capacity, _capacity);
        this->_mutex.unlock(THREAD_ID);
        return (res);
    }
    size_t idx = findIndex(key);
    if (idx == _capacity)
    {
        const_iterator res(_data, _occupied, _capacity, _capacity);
        set_error(ER_SUCCESS);
        this->_mutex.unlock(THREAD_ID);
        return (res);
    }
    const_iterator res(_data, _occupied, idx, _capacity);
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::remove(const Key& key)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (!has_storage())
    {
        flag_storage_error();
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t idx = findIndex(key);
    if (idx == _capacity)
    {
        set_error(ER_SUCCESS);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    ::destroy_at(&_data[idx]);
    _occupied[idx] = false;
    --_size;
    size_t next = (idx + 1) % _capacity;
    while (_occupied[next])
    {
        size_t h = hashKey(_data[next].first);
        if ((next > idx && (h <= idx || h > next)) || (next < idx && (h <= idx && h > next)))
        {
            construct_at(&_data[idx], ft_move(_data[next]));
            ::destroy_at(&_data[next]);
            _occupied[idx] = true;
            _occupied[next] = false;
            idx = next;
        }
        next = (next + 1) % _capacity;
    }
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename Key, typename MappedType>
bool ft_unord_map<Key, MappedType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (true);
    }
    bool res = (_size == 0);
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename Key, typename MappedType>
void ft_unord_map<Key, MappedType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return ;
    }
    if (!has_storage())
    {
        flag_storage_error();
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t i = 0;
    size_t count = 0;
    while (count < _size && i < _capacity)
    {
        if (_occupied[i])
        {
            ::destroy_at(&_data[i]);
            _occupied[i] = false;
            count++;
        }
        i++;
    }
    _size = 0;
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_unord_map<Key, MappedType>::getSize() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (0);
    }
    size_t s = _size;
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename Key, typename MappedType>
size_t ft_unord_map<Key, MappedType>::getCapacity() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (0);
    }
    size_t c = _capacity;
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (c);
}

template <typename Key, typename MappedType>
int ft_unord_map<Key, MappedType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (_error);
    int err = _error;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename Key, typename MappedType>
const char* ft_unord_map<Key, MappedType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (ft_strerror(_error));
    int err = _error;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename Key, typename MappedType>
typename ft_unord_map<Key, MappedType>::iterator ft_unord_map<Key, MappedType>::begin()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (iterator(_data, _occupied, 0, _capacity));
    }
    if (!has_storage())
    {
        flag_storage_error();
        iterator res(_data, _occupied, _capacity, _capacity);
        this->_mutex.unlock(THREAD_ID);
        return (res);
    }
    iterator res(_data, _occupied, 0, _capacity);
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unord_map<Key, MappedType>::iterator ft_unord_map<Key, MappedType>::end()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (iterator(_data, _occupied, _capacity, _capacity));
    }
    iterator res(_data, _occupied, _capacity, _capacity);
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unord_map<Key, MappedType>::const_iterator ft_unord_map<Key, MappedType>::begin() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (const_iterator(_data, _occupied, 0, _capacity));
    }
    if (!has_storage())
    {
        flag_storage_error();
        const_iterator res(_data, _occupied, _capacity, _capacity);
        this->_mutex.unlock(THREAD_ID);
        return (res);
    }
    const_iterator res(_data, _occupied, 0, _capacity);
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unord_map<Key, MappedType>::const_iterator ft_unord_map<Key, MappedType>::end() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (const_iterator(_data, _occupied, _capacity, _capacity));
    }
    const_iterator res(_data, _occupied, _capacity, _capacity);
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename Key, typename MappedType>
MappedType& ft_unord_map<Key, MappedType>::at(const Key& key)
{
    static MappedType errorMappedType = MappedType();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (errorMappedType);
    }
    if (!has_storage())
    {
        flag_storage_error();
        this->_mutex.unlock(THREAD_ID);
        return (errorMappedType);
    }
    size_t idx = findIndex(key);
    if (idx == _capacity)
    {
        set_error(UNORD_MAP_UNKNOWN);
        this->_mutex.unlock(THREAD_ID);
        return (errorMappedType);
    }
    MappedType& val = _data[idx].second;
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (val);
}

template <typename Key, typename MappedType>
const MappedType& ft_unord_map<Key, MappedType>::at(const Key& key) const
{
    static MappedType errorMappedType = MappedType();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(this->_mutex.get_error());
        return (errorMappedType);
    }
    if (!has_storage())
    {
        flag_storage_error();
        this->_mutex.unlock(THREAD_ID);
        return (errorMappedType);
    }
    size_t idx = findIndex(key);
    if (idx == _capacity)
    {
        set_error(UNORD_MAP_UNKNOWN);
        this->_mutex.unlock(THREAD_ID);
        return (errorMappedType);
    }
    const MappedType& val = _data[idx].second;
    set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (val);
}

template <typename Key, typename MappedType>
MappedType& ft_unord_map<Key, MappedType>::operator[](const Key& key)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        static MappedType errorVal = MappedType();
        set_error(this->_mutex.get_error());
        return (errorVal);
    }
    if (!has_storage())
    {
        static MappedType errorVal = MappedType();
        flag_storage_error();
        this->_mutex.unlock(THREAD_ID);
        return (errorVal);
    }
    _error = ER_SUCCESS;
    size_t idx = findIndex(key);
    if (idx != _capacity)
    {
        MappedType& res = _data[idx].second;
        set_error(ER_SUCCESS);
        this->_mutex.unlock(THREAD_ID);
        return (res);
    }
    if ((_size * 2) >= _capacity)
    {
        resize(_capacity * 2);
        if (_error != ER_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            static MappedType errorVal = MappedType();
            return (errorVal);
        }
    }
    size_t start = hashKey(key);
    size_t i = start;
    while (true)
    {
        if (!_occupied[i])
        {
            construct_at(&_data[i], ft_pair<Key, MappedType>(key, MappedType()));
            _occupied[i] = true;
            ++_size;
            MappedType& res = _data[i].second;
            set_error(ER_SUCCESS);
            this->_mutex.unlock(THREAD_ID);
            return (res);
        }
        i = (i + 1) % _capacity;
        if (i == start)
            break;
    }
    static MappedType errorVal = MappedType();
    set_error(UNORD_MAP_UNKNOWN);
    this->_mutex.unlock(THREAD_ID);
    return (errorVal);
}

#endif
