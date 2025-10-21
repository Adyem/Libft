#ifndef FT_UNORDERED_MAP_HPP
#define FT_UNORDERED_MAP_HPP

#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "constructor.hpp"
#include <cstddef>
#include <functional>
#include "../Libft/libft.hpp"
#include <utility>
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
class ft_unordered_map
{
    private:
        ft_pair<Key, MappedType>*  _data;
        bool*                      _occupied;
        size_t                     _capacity;
        size_t                     _size;
        mutable int                _error;

        void    resize(size_t new_capacity);
        size_t  find_index(const Key& key) const;
        void    set_error(int error) const;
        size_t  hash_key(const Key& key) const;
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
                void                      skip_unoccupied();

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
                void                            skip_unoccupied();

            public:
                const_iterator(const ft_pair<Key, MappedType>* data, const bool* occ, size_t idx, size_t cap);
                const ft_pair<Key, MappedType>& operator*() const;
                const ft_pair<Key, MappedType>* operator->() const;
                const_iterator& operator++();
                bool operator==(const const_iterator& other) const;
                bool operator!=(const const_iterator& other) const;
        };

        ft_unordered_map(size_t initial_capacity = 10);
        ft_unordered_map(const ft_unordered_map& other);
        ft_unordered_map& operator=(const ft_unordered_map& other);
        ft_unordered_map(ft_unordered_map&& other) noexcept;
        ft_unordered_map& operator=(ft_unordered_map&& other) noexcept;
        ~ft_unordered_map();

        void           insert(const Key& key, const MappedType& value);
        iterator       find(const Key& key);
        const_iterator find(const Key& key) const;
        void           erase(const Key& key);
        bool           empty() const;
        void           clear();
        size_t         size() const;
        size_t         bucket_count() const;
        bool           has_valid_storage() const;
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
ft_unordered_map<Key, MappedType>::iterator::iterator(ft_pair<Key, MappedType>* data, bool* occ, size_t idx, size_t cap)
    : _data(data), _occupied(occ), _index(idx), _capacity(cap)
{
    skip_unoccupied();
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::skip_unoccupied()
{
    while (_index < _capacity && !_occupied[_index])
        ++_index;
    return ;
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>& ft_unordered_map<Key, MappedType>::iterator::operator*() const
{
    return (_data[_index]);
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>* ft_unordered_map<Key, MappedType>::iterator::operator->() const
{
    return (&_data[_index]);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator& ft_unordered_map<Key, MappedType>::iterator::operator++()
{
    ++_index;
    skip_unoccupied();
    return (*this);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::iterator::operator==(const iterator& other) const
{
    return (_data == other._data && _index == other._index);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::iterator::operator!=(const iterator& other) const
{
    return (!(*this == other));
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::const_iterator::const_iterator(const ft_pair<Key, MappedType>* data, const bool* occ, size_t idx, size_t cap)
    : _data(data), _occupied(occ), _index(idx), _capacity(cap)
{
    skip_unoccupied();
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::skip_unoccupied()
{
    while (_index < _capacity && !_occupied[_index])
        ++_index;
    return ;
}

template <typename Key, typename MappedType>
const ft_pair<Key, MappedType>& ft_unordered_map<Key, MappedType>::const_iterator::operator*() const
{
    return (_data[_index]);
}

template <typename Key, typename MappedType>
const ft_pair<Key, MappedType>* ft_unordered_map<Key, MappedType>::const_iterator::operator->() const
{
    return (&_data[_index]);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator& ft_unordered_map<Key, MappedType>::const_iterator::operator++()
{
    ++_index;
    skip_unoccupied();
    return (*this);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::const_iterator::operator==(const const_iterator& other) const
{
    return (_data == other._data && _index == other._index);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::const_iterator::operator!=(const const_iterator& other) const
{
    return (!(*this == other));
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::ft_unordered_map(size_t initial_capacity)
    : _capacity(initial_capacity), _size(0), _error(ER_SUCCESS)
{
    void* raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * _capacity);
    if (!raw_data)
    {
        set_error(FT_ERR_NO_MEMORY);
        _data = ft_nullptr;
        _occupied = ft_nullptr;
        return ;
    }
    _data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
    void* raw_occupied = cma_malloc(sizeof(bool) * _capacity);
    if (!raw_occupied)
    {
        set_error(FT_ERR_NO_MEMORY);
        cma_free(_data);
        _data = ft_nullptr;
        _occupied = ft_nullptr;
        return ;
    }
    _occupied = static_cast<bool*>(raw_occupied);
    size_t i = 0;
    while (i < _capacity)
    {
        _occupied[i] = false;
        i++;
    }
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::ft_unordered_map(const ft_unordered_map<Key, MappedType>& other)
    : _capacity(other._capacity), _size(other._size), _error(other._error)
{
    if (other._data != ft_nullptr && _size > 0)
    {
        void* raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * _capacity);
        if (!raw_data)
        {
            set_error(FT_ERR_NO_MEMORY);
            _data = ft_nullptr;
            _occupied = ft_nullptr;
            _size = 0;
            _capacity = 0;
            return ;
        }
        _data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
        void* raw_occupied = cma_malloc(sizeof(bool) * _capacity);
        if (!raw_occupied)
        {
            set_error(FT_ERR_NO_MEMORY);
            cma_free(_data);
            _data = ft_nullptr;
            _occupied = ft_nullptr;
            _size = 0;
            _capacity = 0;
            return ;
        }
        _occupied = static_cast<bool*>(raw_occupied);
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
ft_unordered_map<Key, MappedType>& ft_unordered_map<Key, MappedType>::operator=(const ft_unordered_map<Key, MappedType>& other)
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
            void* raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * other._capacity);
            if (!raw_data)
            {
                set_error(FT_ERR_NO_MEMORY);
                _data = ft_nullptr;
                _occupied = ft_nullptr;
                _size = 0;
                _capacity = 0;
                return (*this);
            }
            _data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
            void* raw_occupied = cma_malloc(sizeof(bool) * other._capacity);
            if (!raw_occupied)
            {
                set_error(FT_ERR_NO_MEMORY);
                cma_free(_data);
                _data = ft_nullptr;
                _occupied = ft_nullptr;
                _size = 0;
                _capacity = 0;
                return (*this);
            }
            _occupied = static_cast<bool*>(raw_occupied);
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
ft_unordered_map<Key, MappedType>::ft_unordered_map(ft_unordered_map<Key, MappedType>&& other) noexcept
    : _data(other._data), _occupied(other._occupied), _capacity(other._capacity), _size(other._size), _error(other._error)
{
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error = ER_SUCCESS;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>& ft_unordered_map<Key, MappedType>::operator=(ft_unordered_map<Key, MappedType>&& other) noexcept
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
    }
    return (*this);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::~ft_unordered_map()
{
    if (_data != ft_nullptr && _occupied != ft_nullptr)
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
    }
    if (_data != ft_nullptr)
        cma_free(_data);
    if (_occupied != ft_nullptr)
        cma_free(_occupied);
}

template<typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::set_error(int error) const
{
    ft_errno = error;
    _error = error;
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::has_storage() const
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
void ft_unordered_map<Key, MappedType>::flag_storage_error() const
{
    if (_error == ER_SUCCESS)
        set_error(FT_ERR_NO_MEMORY);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::hash_key(const Key& key) const
{
    static std::hash<Key> hf;
    return (hf(key) % _capacity);
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::find_index(const Key& key) const
{
    if (_capacity == 0)
        return (_capacity);
    if (_data == ft_nullptr)
        return (_capacity);
    if (_occupied == ft_nullptr)
        return (_capacity);
    if (_size == 0)
        return (_capacity);
    size_t start_index = hash_key(key);
    size_t i = start_index;
    while (true)
    {
        if (!_occupied[i])
            return (_capacity);
        if (_data[i].first == key)
            return (i);
        i = (i + 1) % _capacity;
        if (i == start_index)
            return (_capacity);
    }
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::resize(size_t new_capacity)
{
    if (new_capacity == 0)
    {
        set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    _error = ER_SUCCESS;
    void* raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * new_capacity);
    if (!raw_data)
    {
        set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    ft_pair<Key, MappedType>* new_data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
    void* raw_occupied = cma_malloc(sizeof(bool) * new_capacity);
    if (!raw_occupied)
    {
        set_error(FT_ERR_NO_MEMORY);
        cma_free(new_data);
        return ;
    }
    bool* new_occ = static_cast<bool*>(raw_occupied);
    size_t i = 0;
    while (i < new_capacity)
    {
        new_occ[i] = false;
        i++;
    }
    size_t old_capacity = _capacity;
    ft_pair<Key, MappedType>* old_data = _data;
    bool* old_occ = _occupied;
    _data = new_data;
    _occupied = new_occ;
    _capacity = new_capacity;
    size_t old_size = _size;
    _size = 0;
    i = 0;
    size_t count = 0;
    while (count < old_size && i < old_capacity)
    {
        if (old_occ[i])
        {
            insert_internal(old_data[i].first, old_data[i].second);
            ::destroy_at(&old_data[i]);
            count++;
        }
        i++;
    }
    cma_free(old_data);
    cma_free(old_occ);
    if (_error == ER_SUCCESS)
        set_error(ER_SUCCESS);
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::insert_internal(const Key& key, const MappedType& value)
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
    size_t idx = find_index(key);
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
    size_t start_index = hash_key(key);
    size_t i = start_index;
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
        if (i == start_index)
            return ;
    }
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::insert(const Key& key, const MappedType& value)
{
    if (!has_storage())
    {
        flag_storage_error();
        return ;
    }
    _error = ER_SUCCESS;
    insert_internal(key, value);
    if (_error == ER_SUCCESS)
        set_error(ER_SUCCESS);
    return ;
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator ft_unordered_map<Key, MappedType>::find(const Key& key)
{
    if (!has_storage())
    {
        flag_storage_error();
        return (iterator(_data, _occupied, _capacity, _capacity));
    }
    size_t idx = find_index(key);
    if (idx == _capacity)
    {
        set_error(ER_SUCCESS);
        return (iterator(_data, _occupied, _capacity, _capacity));
    }
    iterator res(_data, _occupied, idx, _capacity);
    set_error(ER_SUCCESS);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator ft_unordered_map<Key, MappedType>::find(const Key& key) const
{
    if (!has_storage())
    {
        flag_storage_error();
        return (const_iterator(_data, _occupied, _capacity, _capacity));
    }
    size_t idx = find_index(key);
    if (idx == _capacity)
    {
        set_error(ER_SUCCESS);
        return (const_iterator(_data, _occupied, _capacity, _capacity));
    }
    const_iterator res(_data, _occupied, idx, _capacity);
    set_error(ER_SUCCESS);
    return (res);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::erase(const Key& key)
{
    if (!has_storage())
    {
        flag_storage_error();
        return ;
    }
    size_t idx = find_index(key);
    if (idx == _capacity)
    {
        set_error(ER_SUCCESS);
        return ;
    }
    ::destroy_at(&_data[idx]);
    _occupied[idx] = false;
    --_size;
    size_t next = (idx + 1) % _capacity;
    while (_occupied[next])
    {
        size_t h = hash_key(_data[next].first);
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
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::empty() const
{
    bool res = (_size == 0);
    set_error(ER_SUCCESS);
    return (res);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::clear()
{
    if (!has_storage())
    {
        flag_storage_error();
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
    return ;
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::size() const
{
    size_t s = _size;
    set_error(ER_SUCCESS);
    return (s);
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::bucket_count() const
{
    size_t c = _capacity;
    set_error(ER_SUCCESS);
    return (c);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::has_valid_storage() const
{
    return (has_storage());
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::get_error() const
{
    int err = _error;
    return (err);
}

template <typename Key, typename MappedType>
const char* ft_unordered_map<Key, MappedType>::get_error_str() const
{
    int err = _error;
    return (ft_strerror(err));
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator ft_unordered_map<Key, MappedType>::begin()
{
    if (!has_storage())
    {
        flag_storage_error();
        return (iterator(_data, _occupied, _capacity, _capacity));
    }
    iterator res(_data, _occupied, 0, _capacity);
    set_error(ER_SUCCESS);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator ft_unordered_map<Key, MappedType>::end()
{
    iterator res(_data, _occupied, _capacity, _capacity);
    set_error(ER_SUCCESS);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator ft_unordered_map<Key, MappedType>::begin() const
{
    if (!has_storage())
    {
        flag_storage_error();
        return (const_iterator(_data, _occupied, _capacity, _capacity));
    }
    const_iterator res(_data, _occupied, 0, _capacity);
    set_error(ER_SUCCESS);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator ft_unordered_map<Key, MappedType>::end() const
{
    const_iterator res(_data, _occupied, _capacity, _capacity);
    set_error(ER_SUCCESS);
    return (res);
}

template <typename Key, typename MappedType>
MappedType& ft_unordered_map<Key, MappedType>::at(const Key& key)
{
    static MappedType error_mapped_value = MappedType();
    if (!has_storage())
    {
        flag_storage_error();
        return (error_mapped_value);
    }
    size_t idx = find_index(key);
    if (idx == _capacity)
    {
        set_error(FT_ERR_INTERNAL);
        return (error_mapped_value);
    }
    MappedType& val = _data[idx].second;
    set_error(ER_SUCCESS);
    return (val);
}

template <typename Key, typename MappedType>
const MappedType& ft_unordered_map<Key, MappedType>::at(const Key& key) const
{
    static MappedType error_mapped_value = MappedType();
    if (!has_storage())
    {
        flag_storage_error();
        return (error_mapped_value);
    }
    size_t idx = find_index(key);
    if (idx == _capacity)
    {
        set_error(FT_ERR_INTERNAL);
        return (error_mapped_value);
    }
    const MappedType& val = _data[idx].second;
    set_error(ER_SUCCESS);
    return (val);
}

template <typename Key, typename MappedType>
MappedType& ft_unordered_map<Key, MappedType>::operator[](const Key& key)
{
    if (!has_storage())
    {
        static MappedType error_value = MappedType();
        flag_storage_error();
        return (error_value);
    }
    _error = ER_SUCCESS;
    size_t idx = find_index(key);
    if (idx != _capacity)
    {
        MappedType& res = _data[idx].second;
        set_error(ER_SUCCESS);
        return (res);
    }
    if ((_size * 2) >= _capacity)
    {
        resize(_capacity * 2);
        if (_error != ER_SUCCESS)
        {
            static MappedType error_value = MappedType();
            return (error_value);
        }
    }
    size_t start_index = hash_key(key);
    size_t i = start_index;
    while (true)
    {
        if (!_occupied[i])
        {
            construct_at(&_data[i], ft_pair<Key, MappedType>(key, MappedType()));
            _occupied[i] = true;
            ++_size;
            MappedType& res = _data[i].second;
            set_error(ER_SUCCESS);
            return (res);
        }
        i = (i + 1) % _capacity;
        if (i == start_index)
            break;
    }
    static MappedType error_value = MappedType();
    set_error(FT_ERR_INTERNAL);
    return (error_value);
}

template <typename Key, typename MappedType>
using ft_unord_map = ft_unordered_map<Key, MappedType>;

#endif
