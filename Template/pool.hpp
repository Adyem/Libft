#ifndef POOL_HPP
#define POOL_HPP

#include <type_traits>
#include <utility>
#include "vector.hpp"
#include "../PThread/mutex.hpp"
#include "../Errno/errno.hpp"

template<typename T>
class Pool
{
    private:
        using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
        ft_vector<Storage> _buffer;
        ft_vector<size_t> _freeIndices;
        mutable pt_mutex _mutex;
        mutable int _error_code;

        void release(size_t idx) noexcept;
        T* ptrAt(size_t idx) noexcept;
        void set_error(int error_code) const noexcept;

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
        int get_error() const noexcept;
        const char* get_error_str() const noexcept;
};

template<typename T>
class Pool<T>::Object
{
    private:
        Pool<T>* _pool;
        size_t _idx;
        T* _ptr;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

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
        int get_error() const noexcept;
        const char* get_error_str() const noexcept;
};

template<typename T>
void Pool<T>::release(size_t idx) noexcept
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    this->_freeIndices.push_back(idx);
    int vector_error = this->_freeIndices.get_error();
    this->_mutex.unlock(THREAD_ID);
    if (vector_error != ER_SUCCESS)
        this->set_error(vector_error);
    else
        this->set_error(ER_SUCCESS);
    return ;
}

template<typename T>
T* Pool<T>::ptrAt(size_t idx) noexcept
{
    size_t buffer_size = this->_buffer.size();
    if (buffer_size == 0 || idx >= buffer_size)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (reinterpret_cast<T*>(&this->_buffer[idx]));
}

template<typename T>
Pool<T>::Pool()
    : _buffer()
    , _freeIndices()
    , _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template<typename T>
Pool<T>::Pool(Pool&& other)
    : _buffer(std::move(other._buffer))
    , _freeIndices(std::move(other._freeIndices))
    , _error_code(other._error_code)
{
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    return ;
}

template<typename T>
Pool<T>& Pool<T>::operator=(Pool&& other)
{
    if (this != &other)
    {
        this->_buffer = std::move(other._buffer);
        this->_freeIndices = std::move(other._freeIndices);
        this->_error_code = other._error_code;
        other._error_code = ER_SUCCESS;
    }
    this->set_error(this->_error_code);
    return (*this);
}

template<typename T>
Pool<T>::~Pool()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template<typename T>
void Pool<T>::resize(size_t new_size)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    this->_buffer.resize(new_size);
    int buffer_error = this->_buffer.get_error();
    if (buffer_error != ER_SUCCESS)
    {
        this->_mutex.unlock(THREAD_ID);
        this->set_error(buffer_error);
        return ;
    }
    this->_freeIndices.clear();
    int free_error = this->_freeIndices.get_error();
    if (free_error != ER_SUCCESS)
    {
        this->_mutex.unlock(THREAD_ID);
        this->set_error(free_error);
        return ;
    }
    this->_freeIndices.reserve(new_size);
    free_error = this->_freeIndices.get_error();
    if (free_error != ER_SUCCESS)
    {
        this->_mutex.unlock(THREAD_ID);
        this->set_error(free_error);
        return ;
    }
    size_t index = 0;
    while (index < new_size)
    {
        this->_freeIndices.push_back(index);
        int push_error = this->_freeIndices.get_error();
        if (push_error != ER_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            this->set_error(push_error);
            return ;
        }
        index++;
    }
    this->_mutex.unlock(THREAD_ID);
    this->set_error(ER_SUCCESS);
    return ;
}

template<typename T>
template<typename... Args>
typename Pool<T>::Object Pool<T>::acquire(Args&&... args)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        Object error_object;
        error_object.set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (error_object);
    }
    size_t free_count = this->_freeIndices.size();
    if (free_count == 0)
    {
        this->_mutex.unlock(THREAD_ID);
        this->set_error(FT_ERR_EMPTY);
        Object error_object;
        error_object.set_error(FT_ERR_EMPTY);
        return (error_object);
    }
    size_t last = free_count - 1;
    size_t idx = this->_freeIndices[last];
    this->_freeIndices.pop_back();
    int vector_error = this->_freeIndices.get_error();
    if (vector_error != ER_SUCCESS)
    {
        this->_mutex.unlock(THREAD_ID);
        this->set_error(vector_error);
        Object error_object;
        error_object.set_error(vector_error);
        return (error_object);
    }
    T* storage_ptr = this->ptrAt(idx);
    if (storage_ptr == ft_nullptr)
    {
        int pointer_error = this->_error_code;
        this->_mutex.unlock(THREAD_ID);
        Object error_object;
        error_object.set_error(pointer_error);
        return (error_object);
    }
    T* ptr = new (storage_ptr) T(std::forward<Args>(args)...);
    this->_mutex.unlock(THREAD_ID);
    Object result(this, idx, ptr);
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

template<typename T>
int Pool<T>::get_error() const noexcept
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

template<typename T>
const char* Pool<T>::get_error_str() const noexcept
{
    this->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

template<typename T>
void Pool<T>::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template<typename T>
Pool<T>::Object::Object() noexcept
    : _pool(ft_nullptr)
    , _idx(0)
    , _ptr(ft_nullptr)
    , _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template<typename T>
Pool<T>::Object::Object(Pool<T>* pool, size_t idx, T* ptr) noexcept
    : _pool(pool)
    , _idx(idx)
    , _ptr(ptr)
    , _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template<typename T>
Pool<T>::Object::~Object() noexcept
{
    if (this->_ptr != ft_nullptr)
    {
        this->_ptr->~T();
        if (this->_pool != ft_nullptr)
            this->_pool->release(this->_idx);
        else
            this->set_error(FT_ERR_INVALID_HANDLE);
    }
    this->_pool = ft_nullptr;
    this->_ptr = ft_nullptr;
    if (this->_error_code == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    return ;
}

template<typename T>
T* Pool<T>::Object::operator->() const noexcept
{
    if (this->_ptr == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (this->_ptr);
}

template<typename T>
Pool<T>::Object::operator bool() const noexcept
{
    if (this->_ptr != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (true);
    }
    this->set_error(FT_ERR_INVALID_HANDLE);
    return (false);
}

template<typename T>
Pool<T>::Object::Object(Object&& o) noexcept
    : _pool(o._pool)
    , _idx(o._idx)
    , _ptr(o._ptr)
    , _error_code(o._error_code)
{
    o._pool = ft_nullptr;
    o._ptr = ft_nullptr;
    o._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    return ;
}

template<typename T>
typename Pool<T>::Object& Pool<T>::Object::operator=(Object&& o) noexcept
{
    if (this != &o)
    {
        this->_pool = o._pool;
        this->_idx = o._idx;
        this->_ptr = o._ptr;
        this->_error_code = o._error_code;
        o._pool = ft_nullptr;
        o._ptr = ft_nullptr;
        o._error_code = ER_SUCCESS;
    }
    this->set_error(this->_error_code);
    return (*this);
}

template<typename T>
int Pool<T>::Object::get_error() const noexcept
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

template<typename T>
const char* Pool<T>::Object::get_error_str() const noexcept
{
    this->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

template<typename T>
void Pool<T>::Object::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

#endif
