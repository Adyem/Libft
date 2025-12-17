#ifndef POOL_HPP
#define POOL_HPP

#include <type_traits>
#include <utility>
#include "vector.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "move.hpp"

template<typename T>
class Pool
{
    private:
        using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
        ft_vector<Storage> _buffer;
        ft_vector<size_t> _freeIndices;
        mutable int _error_code;
        mutable pt_mutex* _state_mutex;
        bool _thread_safe_enabled;

        void release(size_t idx) noexcept;
        T* ptrAt(size_t idx) noexcept;
        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;
        void teardown_thread_safety() noexcept;

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
        int enable_thread_safety() noexcept;
        void disable_thread_safety() noexcept;
        bool is_thread_safe_enabled() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;
};

template<typename T>
class Pool<T>::Object
{
    private:
        friend class Pool<T>;
        Pool<T>* _pool;
        size_t _idx;
        T* _ptr;
        mutable int _error_code;
        mutable pt_mutex* _state_mutex;
        bool _thread_safe_enabled;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;
        void teardown_thread_safety() noexcept;

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
        int enable_thread_safety() noexcept;
        void disable_thread_safety() noexcept;
        bool is_thread_safe_enabled() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;
};

template<typename T>
void Pool<T>::release(size_t idx) noexcept
{
    bool lock_acquired;
    int vector_error;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_freeIndices.push_back(idx);
    vector_error = this->_freeIndices.get_error();
    if (vector_error != FT_ERR_SUCCESSS)
        this->set_error_unlocked(vector_error);
    else
        this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template<typename T>
T* Pool<T>::ptrAt(size_t idx) noexcept
{
    size_t buffer_size;

    buffer_size = this->_buffer.size();
    if (buffer_size == 0 || idx >= buffer_size)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (reinterpret_cast<T*>(&this->_buffer[idx]));
}

template<typename T>
Pool<T>::Pool()
    : _buffer()
    , _freeIndices()
    , _error_code(FT_ERR_SUCCESSS)
    , _state_mutex(ft_nullptr)
    , _thread_safe_enabled(false)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template<typename T>
Pool<T>::Pool(Pool&& other)
    : _buffer()
    , _freeIndices()
    , _error_code(FT_ERR_SUCCESSS)
    , _state_mutex(ft_nullptr)
    , _thread_safe_enabled(false)
{
    bool lock_acquired;
    pt_mutex *other_mutex;
    bool other_thread_safe;

    lock_acquired = false;
    other_mutex = ft_nullptr;
    other_thread_safe = false;
    if (other.lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_buffer = ft_move(other._buffer);
    this->_freeIndices = ft_move(other._freeIndices);
    this->_error_code = other._error_code;
    other_thread_safe = other._thread_safe_enabled;
    other_mutex = other._state_mutex;
    other._error_code = FT_ERR_SUCCESSS;
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other.unlock_internal(lock_acquired);
    if (other_thread_safe && other_mutex != ft_nullptr)
    {
        other_mutex->~pt_mutex();
        cma_free(other_mutex);
    }
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            this->_thread_safe_enabled = false;
            this->set_error(this->_error_code);
            return ;
        }
    }
    this->set_error(this->_error_code);
    return ;
}

template<typename T>
Pool<T>& Pool<T>::operator=(Pool&& other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    pt_mutex *previous_mutex;
    bool previous_thread_safe;
    pt_mutex *other_mutex;
    bool other_thread_safe;

    if (this != &other)
    {
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
        previous_mutex = this->_state_mutex;
        previous_thread_safe = this->_thread_safe_enabled;
        other_mutex = other._state_mutex;
        other_thread_safe = other._thread_safe_enabled;
        this->_buffer = ft_move(other._buffer);
        this->_freeIndices = ft_move(other._freeIndices);
        this->_error_code = other._error_code;
        this->_state_mutex = ft_nullptr;
        this->_thread_safe_enabled = other._thread_safe_enabled;
        other._error_code = FT_ERR_SUCCESSS;
        other._state_mutex = ft_nullptr;
        other._thread_safe_enabled = false;
        other.unlock_internal(other_lock_acquired);
        this->unlock_internal(this_lock_acquired);
        if (previous_thread_safe && previous_mutex != ft_nullptr && previous_mutex != this->_state_mutex)
        {
            previous_mutex->~pt_mutex();
            cma_free(previous_mutex);
        }
        if (other_thread_safe && other_mutex != ft_nullptr)
        {
            other_mutex->~pt_mutex();
            cma_free(other_mutex);
        }
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
            {
                this->_thread_safe_enabled = false;
                this->set_error(this->_error_code);
                return (*this);
            }
        }
    }
    this->set_error(this->_error_code);
    return (*this);
}

template<typename T>
Pool<T>::~Pool()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template<typename T>
void Pool<T>::resize(size_t new_size)
{
    bool lock_acquired;
    int buffer_error;
    int free_error;
    size_t index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_buffer.resize(new_size);
    buffer_error = this->_buffer.get_error();
    if (buffer_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(buffer_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_freeIndices.clear();
    free_error = this->_freeIndices.get_error();
    if (free_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(free_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_freeIndices.reserve(new_size);
    free_error = this->_freeIndices.get_error();
    if (free_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(free_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    index = 0;
    while (index < new_size)
    {
        this->_freeIndices.push_back(index);
        int push_error;

        push_error = this->_freeIndices.get_error();
        if (push_error != FT_ERR_SUCCESSS)
        {
            this->set_error_unlocked(push_error);
            this->unlock_internal(lock_acquired);
            return ;
        }
        index++;
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template<typename T>
template<typename... Args>
typename Pool<T>::Object Pool<T>::acquire(Args&&... args)
{
    bool lock_acquired;
    size_t free_count;
    size_t last;
    size_t idx;
    int vector_error;
    T* storage_ptr;
    Object error_object;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        error_object.set_error(ft_errno);
        return (error_object);
    }
    free_count = this->_freeIndices.size();
    if (free_count == 0)
    {
        this->set_error_unlocked(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        error_object.set_error(FT_ERR_EMPTY);
        return (error_object);
    }
    last = free_count - 1;
    idx = this->_freeIndices[last];
    this->_freeIndices.pop_back();
    vector_error = this->_freeIndices.get_error();
    if (vector_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(vector_error);
        this->unlock_internal(lock_acquired);
        error_object.set_error(vector_error);
        return (error_object);
    }
    storage_ptr = this->ptrAt(idx);
    if (storage_ptr == ft_nullptr)
    {
        int pointer_error;

        pointer_error = this->_error_code;
        this->unlock_internal(lock_acquired);
        error_object.set_error(pointer_error);
        return (error_object);
    }
    T* ptr;

    ptr = new (storage_ptr) T(std::forward<Args>(args)...);
    Object result(this, idx, ptr);
    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        if (result.enable_thread_safety() != 0)
        {
            this->_freeIndices.push_back(idx);
            vector_error = this->_freeIndices.get_error();
            if (vector_error != FT_ERR_SUCCESSS)
                this->set_error_unlocked(vector_error);
            else
                this->set_error_unlocked(result.get_error());
            this->unlock_internal(lock_acquired);
            return (result);
        }
    }
    result.set_error(FT_ERR_SUCCESSS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template<typename T>
int Pool<T>::get_error() const noexcept
{
    bool lock_acquired;
    int error_code;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ft_errno);
    error_code = this->_error_code;
    this->unlock_internal(lock_acquired);
    this->set_error(error_code);
    return (error_code);
}

template<typename T>
const char* Pool<T>::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

template<typename T>
void Pool<T>::set_error_unlocked(int error_code) const noexcept
{
    Pool<T>* mutable_pool;

    mutable_pool = const_cast<Pool<T> *>(this);
    mutable_pool->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template<typename T>
void Pool<T>::set_error(int error_code) const noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->set_error_unlocked(error_code);
    this->unlock_internal(lock_acquired);
    return ;
}

template<typename T>
Pool<T>::Object::Object() noexcept
    : _pool(ft_nullptr)
    , _idx(0)
    , _ptr(ft_nullptr)
    , _error_code(FT_ERR_SUCCESSS)
    , _state_mutex(ft_nullptr)
    , _thread_safe_enabled(false)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template<typename T>
Pool<T>::Object::Object(Pool<T>* pool, size_t idx, T* ptr) noexcept
    : _pool(pool)
    , _idx(idx)
    , _ptr(ptr)
    , _error_code(FT_ERR_SUCCESSS)
    , _state_mutex(ft_nullptr)
    , _thread_safe_enabled(false)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template<typename T>
Pool<T>::Object::~Object() noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->teardown_thread_safety();
        return ;
    }
    if (this->_ptr != ft_nullptr)
    {
        this->_ptr->~T();
        if (this->_pool != ft_nullptr)
            this->_pool->release(this->_idx);
        else
            this->set_error_unlocked(FT_ERR_INVALID_HANDLE);
    }
    this->_pool = ft_nullptr;
    this->_ptr = ft_nullptr;
    if (this->_error_code == FT_ERR_SUCCESSS)
        this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    this->teardown_thread_safety();
    return ;
}

template<typename T>
T* Pool<T>::Object::operator->() const noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ft_nullptr);
    if (this->_ptr == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_HANDLE);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (this->_ptr);
}

template<typename T>
Pool<T>::Object::operator bool() const noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (false);
    if (this->_ptr != ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return (true);
    }
    this->set_error_unlocked(FT_ERR_INVALID_HANDLE);
    this->unlock_internal(lock_acquired);
    return (false);
}

template<typename T>
Pool<T>::Object::Object(Object&& o) noexcept
    : _pool(o._pool)
    , _idx(o._idx)
    , _ptr(o._ptr)
    , _error_code(o._error_code)
    , _state_mutex(o._state_mutex)
    , _thread_safe_enabled(o._thread_safe_enabled)
{
    o._pool = ft_nullptr;
    o._ptr = ft_nullptr;
    o._error_code = FT_ERR_SUCCESSS;
    o._state_mutex = ft_nullptr;
    o._thread_safe_enabled = false;
    this->set_error(this->_error_code);
    return ;
}

template<typename T>
typename Pool<T>::Object& Pool<T>::Object::operator=(Object&& o) noexcept
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    pt_mutex *previous_mutex;
    bool previous_thread_safe;

    if (this != &o)
    {
        this_lock_acquired = false;
        if (this->lock_internal(&this_lock_acquired) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        other_lock_acquired = false;
        if (o.lock_internal(&other_lock_acquired) != 0)
        {
            this->unlock_internal(this_lock_acquired);
            this->set_error(ft_errno);
            return (*this);
        }
        previous_mutex = this->_state_mutex;
        previous_thread_safe = this->_thread_safe_enabled;
        this->_pool = o._pool;
        this->_idx = o._idx;
        this->_ptr = o._ptr;
        this->_error_code = o._error_code;
        this->_state_mutex = o._state_mutex;
        this->_thread_safe_enabled = o._thread_safe_enabled;
        o._pool = ft_nullptr;
        o._idx = 0;
        o._ptr = ft_nullptr;
        o._error_code = FT_ERR_SUCCESSS;
        o._state_mutex = ft_nullptr;
        o._thread_safe_enabled = false;
        o.unlock_internal(other_lock_acquired);
        this->unlock_internal(this_lock_acquired);
        if (previous_thread_safe && previous_mutex != ft_nullptr && previous_mutex != this->_state_mutex)
        {
            previous_mutex->~pt_mutex();
            cma_free(previous_mutex);
        }
    }
    this->set_error(this->_error_code);
    return (*this);
}

template<typename T>
int Pool<T>::Object::get_error() const noexcept
{
    bool lock_acquired;
    int error_code;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ft_errno);
    error_code = this->_error_code;
    this->unlock_internal(lock_acquired);
    this->set_error(error_code);
    return (error_code);
}

template<typename T>
const char* Pool<T>::Object::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

template<typename T>
void Pool<T>::Object::set_error_unlocked(int error_code) const noexcept
{
    typename Pool<T>::Object* mutable_object;

    mutable_object = const_cast<typename Pool<T>::Object *>(this);
    mutable_object->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template<typename T>
void Pool<T>::Object::set_error(int error_code) const noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->set_error_unlocked(error_code);
    this->unlock_internal(lock_acquired);
    return ;
}

template<typename T>
int Pool<T>::enable_thread_safety() noexcept
{
    void *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template<typename T>
void Pool<T>::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template<typename T>
bool Pool<T>::is_thread_safe_enabled() const noexcept
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    return (enabled);
}

template<typename T>
int Pool<T>::lock(bool *lock_acquired) const noexcept
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<Pool<T> *>(this)->set_error(ft_errno);
    else
        const_cast<Pool<T> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

template<typename T>
void Pool<T>::unlock(bool lock_acquired) const noexcept
{
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
        const_cast<Pool<T> *>(this)->set_error(this->_state_mutex->get_error());
    else
        const_cast<Pool<T> *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

template<typename T>
int Pool<T>::lock_internal(bool *lock_acquired) const noexcept
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

template<typename T>
void Pool<T>::unlock_internal(bool lock_acquired) const noexcept
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

template<typename T>
void Pool<T>::teardown_thread_safety() noexcept
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

template<typename T>
int Pool<T>::Object::enable_thread_safety() noexcept
{
    void *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template<typename T>
void Pool<T>::Object::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template<typename T>
bool Pool<T>::Object::is_thread_safe_enabled() const noexcept
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    return (enabled);
}

template<typename T>
int Pool<T>::Object::lock(bool *lock_acquired) const noexcept
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<typename Pool<T>::Object *>(this)->set_error(ft_errno);
    else
        const_cast<typename Pool<T>::Object *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

template<typename T>
void Pool<T>::Object::unlock(bool lock_acquired) const noexcept
{
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
        const_cast<typename Pool<T>::Object *>(this)->set_error(this->_state_mutex->get_error());
    else
    {
        ft_errno = FT_ERR_SUCCESSS;
        const_cast<typename Pool<T>::Object *>(this)->set_error(ft_errno);
    }
    return ;
}

template<typename T>
int Pool<T>::Object::lock_internal(bool *lock_acquired) const noexcept
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

template<typename T>
void Pool<T>::Object::unlock_internal(bool lock_acquired) const noexcept
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

template<typename T>
void Pool<T>::Object::teardown_thread_safety() noexcept
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
