#ifndef POOL_HPP
#define POOL_HPP

#include <utility>
#include "vector.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

template<typename T>
class Pool
{
    private:
        ft_vector<T*>              _slots;
        ft_vector<ft_size_t>          _free_indices;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                    _initialised_state;

        static const uint8_t _state_uninitialised = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialised = 2;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept
        {
            _last_error = error_code;
            return (error_code);
        }

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const
        {
            if (method_name == ft_nullptr)
                method_name = "unknown";
            if (reason == ft_nullptr)
                reason = "unknown";
            pf_printf_fd(2, "Pool lifecycle error: %s: %s\n", method_name, reason);
            su_abort();
            return ;
        }

        void abort_if_not_initialised(const char *method_name) const
        {
            if (this->_initialised_state == _state_initialised)
                return ;
            this->abort_lifecycle_error(method_name,
                "called while object is not initialised");
            return ;
        }

        int32_t lock_internal(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_FALSE;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (lock_result != FT_ERR_SUCCESS)
                return (set_error(lock_result));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_TRUE;
            return (FT_ERR_SUCCESS);
        }

        int32_t unlock_internal(ft_bool lock_acquired) const
        {
            if (lock_acquired == FT_FALSE)
                return (FT_ERR_SUCCESS);
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            return (FT_ERR_SUCCESS);
        }

        T* ptr_at(ft_size_t index) noexcept
        {
            if (index >= this->_slots.size())
            {
                set_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            set_error(FT_ERR_SUCCESS);
            return (this->_slots[index]);
        }

        void release(ft_size_t index) noexcept
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            if (index < this->_slots.size())
                this->_slots[index] = ft_nullptr;
            this->_free_indices.push_back(index);
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

    public:
        class Object
        {
            private:
                Pool<T>                   *_pool;
                ft_size_t                     _index;
                T                         *_pointer;
                mutable pt_recursive_mutex *_mutex;
                uint8_t                    _initialised_state;

                static thread_local int32_t _last_error;

                static int32_t set_error(int32_t error_code) noexcept
                {
                    _last_error = error_code;
                    return (error_code);
                }

                int32_t lock_internal(ft_bool *lock_acquired) const
                {
                    int32_t lock_result;

                    if (lock_acquired != ft_nullptr)
                        *lock_acquired = FT_FALSE;
                    if (this->_mutex == ft_nullptr)
                        return (FT_ERR_SUCCESS);
                    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
                    if (lock_result != FT_ERR_SUCCESS)
                        return (set_error(lock_result));
                    if (lock_acquired != ft_nullptr)
                        *lock_acquired = FT_TRUE;
                    return (FT_ERR_SUCCESS);
                }

                int32_t unlock_internal(ft_bool lock_acquired) const
                {
                    if (lock_acquired == FT_FALSE)
                        return (FT_ERR_SUCCESS);
                    if (this->_mutex == ft_nullptr)
                        return (FT_ERR_SUCCESS);
                    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
                    return (FT_ERR_SUCCESS);
                }

            public:
                Object() noexcept
                    : _pool(ft_nullptr), _index(0), _pointer(ft_nullptr),
                      _mutex(ft_nullptr), _initialised_state(2)
                {
                    set_error(FT_ERR_SUCCESS);
                    return ;
                }

                Object(Pool<T>* pool, ft_size_t index, T* pointer) noexcept
                    : _pool(pool), _index(index), _pointer(pointer), _mutex(ft_nullptr),
                      _initialised_state(2)
                {
                    set_error(FT_ERR_SUCCESS);
                    return ;
                }

                ~Object() noexcept
                {
                    if (this->_initialised_state != 2)
                        return ;
                    if (this->_pointer != ft_nullptr)
                    {
                        this->_pointer->~T();
                        if (this->_pool != ft_nullptr)
                            this->_pool->release(this->_index);
                    }
                    if (this->_mutex != ft_nullptr)
                        (void)this->disable_thread_safety();
                    this->_pool = ft_nullptr;
                    this->_pointer = ft_nullptr;
                    this->_index = 0;
                    this->_initialised_state = 1;
                    return ;
                }

                Object(const Object&) = delete;
                Object& operator=(const Object&) = delete;

                Object(Object&& other) noexcept
                    : _pool(other._pool), _index(other._index), _pointer(other._pointer),
                      _mutex(other._mutex), _initialised_state(other._initialised_state)
                {
                    other._pool = ft_nullptr;
                    other._index = 0;
                    other._pointer = ft_nullptr;
                    other._mutex = ft_nullptr;
                    other._initialised_state = 1;
                    set_error(FT_ERR_SUCCESS);
                    return ;
                }

                Object& operator=(Object&& other) noexcept
                {
                    if (this == &other)
                        return (*this);
                    if (this->_pointer != ft_nullptr)
                    {
                        this->_pointer->~T();
                        if (this->_pool != ft_nullptr)
                            this->_pool->release(this->_index);
                    }
                    if (this->_mutex != ft_nullptr)
                        (void)this->disable_thread_safety();
                    this->_pool = other._pool;
                    this->_index = other._index;
                    this->_pointer = other._pointer;
                    this->_mutex = other._mutex;
                    this->_initialised_state = other._initialised_state;
                    other._pool = ft_nullptr;
                    other._index = 0;
                    other._pointer = ft_nullptr;
                    other._mutex = ft_nullptr;
                    other._initialised_state = 1;
                    set_error(FT_ERR_SUCCESS);
                    return (*this);
                }

                T* operator->() const noexcept
                {
                    if (this->_pointer == ft_nullptr)
                    {
                        set_error(FT_ERR_INVALID_HANDLE);
                        return (ft_nullptr);
                    }
                    set_error(FT_ERR_SUCCESS);
                    return (this->_pointer);
                }

                explicit operator bool() const noexcept
                {
                    if (this->_pointer == ft_nullptr)
                    {
                        set_error(FT_ERR_INVALID_HANDLE);
                        return (false);
                    }
                    set_error(FT_ERR_SUCCESS);
                    return (true);
                }

                int32_t enable_thread_safety()
                {
                    pt_recursive_mutex *new_mutex;
                    int32_t initialize_result;

                    if (this->_mutex != ft_nullptr)
                        return (set_error(FT_ERR_SUCCESS));
                    new_mutex = new (std::nothrow) pt_recursive_mutex();
                    if (new_mutex == ft_nullptr)
                        return (set_error(FT_ERR_NO_MEMORY));
                    initialize_result = new_mutex->initialize();
                    if (initialize_result != FT_ERR_SUCCESS)
                    {
                        delete new_mutex;
                        return (set_error(initialize_result));
                    }
                    this->_mutex = new_mutex;
                    return (set_error(FT_ERR_SUCCESS));
                }

                int32_t disable_thread_safety()
                {
                    pt_recursive_mutex *mutex_pointer;
                    int32_t destroy_result;

                    mutex_pointer = this->_mutex;
                    if (mutex_pointer == ft_nullptr)
                        return (set_error(FT_ERR_SUCCESS));
                    this->_mutex = ft_nullptr;
                    destroy_result = mutex_pointer->destroy();
                    delete mutex_pointer;
                    if (destroy_result != FT_ERR_SUCCESS)
                        return (set_error(destroy_result));
                    return (set_error(FT_ERR_SUCCESS));
                }

                bool is_thread_safe() const
                {
                    set_error(FT_ERR_SUCCESS);
                    return (this->_mutex != ft_nullptr);
                }

                int32_t lock(ft_bool *lock_acquired) const
                {
                    int32_t lock_result;

                    lock_result = this->lock_internal(lock_acquired);
                    return (set_error(lock_result));
                }

                void unlock(ft_bool lock_acquired) const
                {
                    (void)this->unlock_internal(lock_acquired);
                    return ;
                }

                static int32_t get_error() noexcept
                {
                    return (_last_error);
                }

                static const char *get_error_str() noexcept
                {
                    return (ft_strerror(_last_error));
                }

        };

        Pool()
            : _slots(), _free_indices(), _mutex(ft_nullptr),
              _initialised_state(_state_uninitialised)
        {
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        ~Pool()
        {
            if (this->_initialised_state == _state_initialised)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        Pool(Pool&& other) = delete;
        Pool& operator=(Pool&& other) = delete;
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        int32_t initialize()
        {
            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("Pool::initialize",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_slots.clear();
            this->_free_indices.clear();
            this->_initialised_state = _state_initialised;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t destroy()
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            if (this->_initialised_state != _state_initialised)
                return (set_error(FT_ERR_SUCCESS));
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_error(lock_error));
            ft_size_t index;

            index = 0;
            while (index < this->_slots.size())
            {
                if (this->_slots[index] != ft_nullptr)
                {
                    this->_slots[index]->~T();
                    cma_free(this->_slots[index]);
                    this->_slots[index] = ft_nullptr;
                }
                ++index;
            }
            this->_slots.clear();
            this->_free_indices.clear();
            (void)this->unlock_internal(lock_acquired);
            this->_initialised_state = _state_destroyed;
            return (set_error(FT_ERR_SUCCESS));
        }

        void resize(ft_size_t new_size)
        {
            ft_bool lock_acquired;
            int32_t lock_error;
            ft_size_t index;

            this->abort_if_not_initialised("Pool::resize");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            this->_slots.resize(new_size, ft_nullptr);
            this->_free_indices.clear();
            this->_free_indices.reserve(new_size);
            index = 0;
            while (index < new_size)
            {
                this->_free_indices.push_back(index);
                ++index;
            }
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        template<typename... Args>
        Object acquire(Args&&... args)
        {
            ft_bool lock_acquired;
            int32_t lock_error;
            ft_size_t free_count;
            ft_size_t last_index;
            ft_size_t storage_index;
            T *constructed_pointer;
            Object result;

            this->abort_if_not_initialised("Pool::acquire");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (result);
            }
            free_count = this->_free_indices.size();
            if (free_count == 0)
            {
                (void)this->unlock_internal(lock_acquired);
                set_error(FT_ERR_EMPTY);
                return (result);
            }
            last_index = free_count - 1;
            storage_index = this->_free_indices[last_index];
            this->_free_indices.pop_back();
            constructed_pointer = static_cast<T*>(cma_malloc(sizeof(T)));
            if (constructed_pointer == ft_nullptr)
            {
                this->_free_indices.push_back(storage_index);
                (void)this->unlock_internal(lock_acquired);
                set_error(FT_ERR_NO_MEMORY);
                return (result);
            }
            construct_at(constructed_pointer, std::forward<Args>(args)...);
            this->_slots[storage_index] = constructed_pointer;
            result = Object(this, storage_index, constructed_pointer);
            if (this->_mutex != ft_nullptr)
                (void)result.enable_thread_safety();
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (result);
        }

        int32_t enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int32_t initialize_result;

            this->abort_if_not_initialised("Pool::enable_thread_safety");
            if (this->_mutex != ft_nullptr)
                return (set_error(FT_ERR_SUCCESS));
            new_mutex = new (std::nothrow) pt_recursive_mutex();
            if (new_mutex == ft_nullptr)
                return (set_error(FT_ERR_NO_MEMORY));
            initialize_result = new_mutex->initialize();
            if (initialize_result != FT_ERR_SUCCESS)
            {
                delete new_mutex;
                return (set_error(initialize_result));
            }
            this->_mutex = new_mutex;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int32_t destroy_result;

            if (this->_initialised_state != _state_initialised
                && this->_initialised_state != _state_destroyed)
                return (set_error(FT_ERR_INVALID_STATE));
            mutex_pointer = this->_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_error(FT_ERR_SUCCESS));
            this->_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_error(destroy_result));
            return (set_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            this->abort_if_not_initialised("Pool::is_thread_safe");
            set_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int32_t lock(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            this->abort_if_not_initialised("Pool::lock");
            lock_result = this->lock_internal(lock_acquired);
            return (set_error(lock_result));
        }

        void unlock(ft_bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t get_error() noexcept
        {
            return (_last_error);
        }

        static const char *get_error_str() noexcept
        {
            return (ft_strerror(_last_error));
        }

};

template<typename T>
thread_local int32_t Pool<T>::_last_error = FT_ERR_SUCCESS;

template<typename T>
thread_local int32_t Pool<T>::Object::_last_error = FT_ERR_SUCCESS;

#endif
