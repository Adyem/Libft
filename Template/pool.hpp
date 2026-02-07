#ifndef POOL_HPP
#define POOL_HPP

#include <type_traits>
#include <utility>
#include "vector.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "move.hpp"

template<typename T>
class Pool
{
    private:
        using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
        ft_vector<Storage> _buffer;
        ft_vector<size_t> _freeIndices;
        mutable pt_recursive_mutex* _mutex;

        void release(size_t idx) noexcept;
        T* ptrAt(size_t idx) noexcept;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

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
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template<typename T>
class Pool<T>::Object
{
    private:
        friend class Pool<T>;
        Pool<T>* _pool;
        size_t _idx;
        T* _ptr;
        mutable pt_recursive_mutex* _mutex;

        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

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
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template<typename T>
void Pool<T>::release(size_t idx) noexcept
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_freeIndices.push_back(idx);
    int vector_error = ft_global_error_stack_peek_last_error();

    if (vector_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(vector_error);
        return ;
    }
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template<typename T>
T* Pool<T>::ptrAt(size_t idx) noexcept
{
    size_t buffer_size = this->_buffer.size();

    if (buffer_size == 0 || idx >= buffer_size)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (reinterpret_cast<T*>(&this->_buffer[idx]));
}

template<typename T>
Pool<T>::Pool()
    : _buffer(),
      _freeIndices(),
      _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template<typename T>
Pool<T>::~Pool()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template<typename T>
Pool<T>::Pool(Pool&& other)
    : _buffer(),
      _freeIndices(),
      _mutex(ft_nullptr)
{
    bool lock_acquired = false;
    int lock_result = other.lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_buffer = ft_move(other._buffer);
    this->_freeIndices = ft_move(other._freeIndices);
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other.unlock_internal(lock_acquired);
    if (other_thread_safe)
    {
        int enable_result = this->enable_thread_safety();

        if (enable_result != FT_ERR_SUCCESSS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template<typename T>
Pool<T>& Pool<T>::operator=(Pool&& other)
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired = false;
    int lock_result = this->lock_internal(&this_lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    bool other_lock_acquired = false;
    lock_result = other.lock_internal(&other_lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    this->_buffer = ft_move(other._buffer);
    this->_freeIndices = ft_move(other._freeIndices);
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    this->teardown_thread_safety();
    if (other_thread_safe)
    {
        int enable_result = this->enable_thread_safety();

        if (enable_result != FT_ERR_SUCCESSS)
            return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template<typename T>
void Pool<T>::resize(size_t new_size)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_buffer.resize(new_size);
    int buffer_error = ft_global_error_stack_peek_last_error();
    ft_global_error_stack_push(buffer_error);

    if (buffer_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(buffer_error);
        return ;
    }
    this->_freeIndices.clear();
    int free_error = ft_global_error_stack_peek_last_error();
    ft_global_error_stack_push(free_error);

    if (free_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(free_error);
        return ;
    }
    this->_freeIndices.reserve(new_size);
    free_error = ft_global_error_stack_peek_last_error();
    ft_global_error_stack_push(free_error);

    if (free_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(free_error);
        return ;
    }
    size_t index = 0;

    while (index < new_size)
    {
        this->_freeIndices.push_back(index);
        int push_error = ft_global_error_stack_peek_last_error();
        ft_global_error_stack_push(push_error);

        if (push_error != FT_ERR_SUCCESSS)
        {
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(push_error);
            return ;
        }
        index = index + 1;
    }
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template<typename T>
template<typename... Args>
typename Pool<T>::Object Pool<T>::acquire(Args&&... args)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (Object());
    }
    size_t free_count = this->_freeIndices.size();

    if (free_count == 0)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_EMPTY);
        return (Object());
    }
    size_t last = free_count - 1;
    size_t idx = this->_freeIndices[last];
    this->_freeIndices.pop_back();
    int vector_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_push(vector_error);

    if (vector_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(vector_error);
        return (Object());
    }
    T* storage_ptr = this->ptrAt(idx);

    if (storage_ptr == ft_nullptr)
    {
        this->unlock_internal(lock_acquired);
        return (Object());
    }
    T* ptr = new (storage_ptr) T(std::forward<Args>(args)...);
    Object result(this, idx, ptr);

    if (this->_mutex != ft_nullptr)
    {
        int enable_result = result.enable_thread_safety();

        if (enable_result != FT_ERR_SUCCESSS)
        {
            this->_freeIndices.push_back(idx);
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(enable_result);
            return (result);
        }
    }
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template<typename T>
int Pool<T>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    int result = this->prepare_thread_safety();

    ft_global_error_stack_push(result);
    return (result);
}

template<typename T>
void Pool<T>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template<typename T>
bool Pool<T>::is_thread_safe_enabled() const
{
    bool enabled = (this->_mutex != ft_nullptr);

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template<typename T>
int Pool<T>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);

    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESSS)
        return (-1);
    return (0);
}

template<typename T>
void Pool<T>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);

    ft_global_error_stack_push(result);
}

template<typename T>
int Pool<T>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);

    if (result == FT_ERR_SUCCESSS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (result);
}

template<typename T>
int Pool<T>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template<typename T>
int Pool<T>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);

    if (result != FT_ERR_SUCCESSS && this->_mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&this->_mutex);
    return (result);
}

template<typename T>
void Pool<T>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

#ifdef LIBFT_TEST_BUILD

template<typename T>
pt_recursive_mutex* Pool<T>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

template<typename T>
Pool<T>::Object::Object() noexcept
    : _pool(ft_nullptr),
      _idx(0),
      _ptr(ft_nullptr),
      _mutex(ft_nullptr)
{
}

template<typename T>
Pool<T>::Object::Object(Pool<T>* pool, size_t idx, T* ptr) noexcept
    : _pool(pool),
      _idx(idx),
      _ptr(ptr),
      _mutex(ft_nullptr)
{
}

template<typename T>
Pool<T>::Object::~Object() noexcept
{
    if (this->_ptr != ft_nullptr)
    {
        this->_ptr->~T();
        if (this->_pool != ft_nullptr)
        {
            this->_pool->release(this->_idx);
        }
        else
        {
            ft_global_error_stack_push(FT_ERR_INVALID_HANDLE);
        }
    }
    this->teardown_thread_safety();
    this->_pool = ft_nullptr;
    this->_idx = 0;
    this->_ptr = ft_nullptr;
}

template<typename T>
T* Pool<T>::Object::operator->() const noexcept
{
    if (this->_ptr == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_HANDLE);
        return (ft_nullptr);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (this->_ptr);
}

template<typename T>
Pool<T>::Object::operator bool() const noexcept
{
    bool result = (this->_ptr != ft_nullptr);

    ft_global_error_stack_push(result ? FT_ERR_SUCCESSS : FT_ERR_INVALID_HANDLE);
    return (result);
}

template<typename T>
Pool<T>::Object::Object(Object&& o) noexcept
    : _pool(o._pool),
      _idx(o._idx),
      _ptr(o._ptr),
      _mutex(o._mutex)
{
    o._pool = ft_nullptr;
    o._idx = 0;
    o._ptr = ft_nullptr;
    o._mutex = ft_nullptr;
}

template<typename T>
typename Pool<T>::Object& Pool<T>::Object::operator=(Object&& o) noexcept
{
    if (this == &o)
        return (*this);
    this->teardown_thread_safety();
    this->_pool = o._pool;
    this->_idx = o._idx;
    this->_ptr = o._ptr;
    this->_mutex = o._mutex;
    o._pool = ft_nullptr;
    o._idx = 0;
    o._ptr = ft_nullptr;
    o._mutex = ft_nullptr;
    return (*this);
}

template<typename T>
int Pool<T>::Object::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    int result = this->prepare_thread_safety();

    ft_global_error_stack_push(result);
    return (result);
}

template<typename T>
void Pool<T>::Object::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template<typename T>
bool Pool<T>::Object::is_thread_safe_enabled() const
{
    bool enabled = (this->_mutex != ft_nullptr);

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template<typename T>
int Pool<T>::Object::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);

    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESSS)
        return (-1);
    return (0);
}

template<typename T>
void Pool<T>::Object::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);

    ft_global_error_stack_push(result);
}

template<typename T>
int Pool<T>::Object::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);

    if (result == FT_ERR_SUCCESSS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (result);
}

template<typename T>
int Pool<T>::Object::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template<typename T>
int Pool<T>::Object::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);

    if (result != FT_ERR_SUCCESSS && this->_mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&this->_mutex);
    return (result);
}

template<typename T>
void Pool<T>::Object::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

#ifdef LIBFT_TEST_BUILD

template<typename T>
pt_recursive_mutex* Pool<T>::Object::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
