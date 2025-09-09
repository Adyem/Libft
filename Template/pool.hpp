#ifndef POOL_HPP
#define POOL_HPP

#include <type_traits>
#include <utility>
#include "vector.hpp"
#include "../PThread/mutex.hpp"

template<typename T>
class Pool
{
    private:
        using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
        ft_vector<Storage> _buffer;
        ft_vector<size_t> _freeIndices;
        mutable pt_mutex _mutex;

        void release(size_t idx) noexcept;
        T* ptrAt(size_t idx) noexcept;

    public:
        Pool();
        ~Pool();
        Pool(Pool&& other);
        Pool& operator=(Pool&& other);

        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        void resize(size_t new_size);

        class Object;
        template<typename... Args>
        Object acquire(Args&&... args);
};

template<typename T>
class Pool<T>::Object
{
    private:
        Pool<T>* _pool;
        size_t _idx;
        T* _ptr;

    public:
        Object() noexcept;
        Object(Pool<T>* pool, size_t idx, T* ptr) noexcept;
        ~Object() noexcept;

        T* operator->() const noexcept;
        explicit operator bool() const noexcept;

        Object(Object&& o) noexcept;
        Object& operator=(Object&& o) noexcept;

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
};

template<typename T>
void Pool<T>::release(size_t idx) noexcept
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    this->_freeIndices.push_back(idx);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template<typename T>
T* Pool<T>::ptrAt(size_t idx) noexcept
{
    return (reinterpret_cast<T*>(&_buffer[idx]));
}

template<typename T>
Pool<T>::Pool()
    : _buffer()
    , _freeIndices()
{
    return ;
}

template<typename T>
Pool<T>::Pool(Pool&& other)
    : _buffer(std::move(other._buffer))
    , _freeIndices(std::move(other._freeIndices))
{
    return ;
}

template<typename T>
Pool<T>& Pool<T>::operator=(Pool&& other)
{
    if (this != &other)
    {
        _buffer = std::move(other._buffer);
        _freeIndices = std::move(other._freeIndices);
    }
    return (*this);
}

template<typename T>
Pool<T>::~Pool()
{
    return ;
}

template<typename T>
void Pool<T>::resize(size_t new_size)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    _buffer.resize(new_size);
    _freeIndices.clear();
    _freeIndices.reserve(new_size);
    size_t index = 0;
    while (index < new_size)
    {
        _freeIndices.push_back(index);
        index++;
    }
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template<typename T>
template<typename... Args>
typename Pool<T>::Object Pool<T>::acquire(Args&&... args)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (Object());
    if (_freeIndices.size() == 0)
    {
        this->_mutex.unlock(THREAD_ID);
        return (Object());
    }
    size_t last = _freeIndices.size() - 1;
    size_t idx = _freeIndices[last];
    _freeIndices.pop_back();
    T* ptr = new (ptrAt(idx)) T(std::forward<Args>(args)...);
    this->_mutex.unlock(THREAD_ID);
    return (Object(this, idx, ptr));
}

template<typename T>
Pool<T>::Object::Object() noexcept
    : _pool(ft_nullptr)
    , _idx(0)
    , _ptr(ft_nullptr)
{
    return ;
}

template<typename T>
Pool<T>::Object::Object(Pool<T>* pool, size_t idx, T* ptr) noexcept
    : _pool(pool)
    , _idx(idx)
    , _ptr(ptr)
{
    return ;
}

template<typename T>
Pool<T>::Object::~Object() noexcept
{
    if (_ptr)
    {
        _ptr->~T();
        _pool->release(_idx);
    }
    return ;
}

template<typename T>
T* Pool<T>::Object::operator->() const noexcept
{
    return (this->_ptr);
}

template<typename T>
Pool<T>::Object::operator bool() const noexcept
{
    return (this->_ptr != ft_nullptr);
}

template<typename T>
Pool<T>::Object::Object(Object&& o) noexcept
    : _pool(o._pool)
    , _idx(o._idx)
    , _ptr(o._ptr)
{
    o._pool = ft_nullptr;
    o._ptr = ft_nullptr;
    return ;
}

template<typename T>
typename Pool<T>::Object& Pool<T>::Object::operator=(Object&& o) noexcept
{
    if (this != &o)
    {
        _pool = o._pool;
        _idx = o._idx;
        _ptr = o._ptr;
        o._pool = ft_nullptr;
        o._ptr = ft_nullptr;
    }
    return (*this);
}

#endif
