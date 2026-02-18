#ifndef POOL_HPP
#define POOL_HPP

#include <utility>
#include "vector.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

template<typename T>
class Pool
{
    private:
        ft_vector<T*>              _slots;
        ft_vector<size_t>          _free_indices;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                    _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        static thread_local int32_t _last_error;

        static int32_t set_last_operation_error(int32_t error_code) noexcept
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

        void abort_if_not_initialized(const char *method_name) const
        {
            if (this->_initialized_state == _state_initialized)
                return ;
            this->abort_lifecycle_error(method_name,
                "called while object is not initialized");
            return ;
        }

        int lock_internal(bool *lock_acquired) const
        {
            int lock_result;

            if (lock_acquired != ft_nullptr)
                *lock_acquired = false;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_result = this->_mutex->lock();
            if (lock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_result));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        int unlock_internal(bool lock_acquired) const
        {
            int unlock_result;

            if (lock_acquired == false)
                return (FT_ERR_SUCCESS);
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            unlock_result = this->_mutex->unlock();
            if (unlock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_result));
            return (FT_ERR_SUCCESS);
        }

        T* ptr_at(size_t index) noexcept
        {
            if (index >= this->_slots.size())
            {
                set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_slots[index]);
        }

        void release(size_t index) noexcept
        {
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (index < this->_slots.size())
                this->_slots[index] = ft_nullptr;
            this->_free_indices.push_back(index);
            (void)this->unlock_internal(lock_acquired);
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

    public:
        class Object
        {
            private:
                Pool<T>                   *_pool;
                size_t                     _index;
                T                         *_pointer;
                mutable pt_recursive_mutex *_mutex;
                uint8_t                    _initialized_state;

                static thread_local int32_t _last_error;

                static int32_t set_last_operation_error(int32_t error_code) noexcept
                {
                    _last_error = error_code;
                    return (error_code);
                }

                int lock_internal(bool *lock_acquired) const
                {
                    int lock_result;

                    if (lock_acquired != ft_nullptr)
                        *lock_acquired = false;
                    if (this->_mutex == ft_nullptr)
                        return (FT_ERR_SUCCESS);
                    lock_result = this->_mutex->lock();
                    if (lock_result != FT_ERR_SUCCESS)
                        return (set_last_operation_error(lock_result));
                    if (lock_acquired != ft_nullptr)
                        *lock_acquired = true;
                    return (FT_ERR_SUCCESS);
                }

                int unlock_internal(bool lock_acquired) const
                {
                    int unlock_result;

                    if (lock_acquired == false)
                        return (FT_ERR_SUCCESS);
                    if (this->_mutex == ft_nullptr)
                        return (FT_ERR_SUCCESS);
                    unlock_result = this->_mutex->unlock();
                    if (unlock_result != FT_ERR_SUCCESS)
                        return (set_last_operation_error(unlock_result));
                    return (FT_ERR_SUCCESS);
                }

            public:
                Object() noexcept
                    : _pool(ft_nullptr), _index(0), _pointer(ft_nullptr),
                      _mutex(ft_nullptr), _initialized_state(2)
                {
                    set_last_operation_error(FT_ERR_SUCCESS);
                    return ;
                }

                Object(Pool<T>* pool, size_t index, T* pointer) noexcept
                    : _pool(pool), _index(index), _pointer(pointer), _mutex(ft_nullptr),
                      _initialized_state(2)
                {
                    set_last_operation_error(FT_ERR_SUCCESS);
                    return ;
                }

                ~Object() noexcept
                {
                    if (this->_initialized_state != 2)
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
                    this->_initialized_state = 1;
                    return ;
                }

                Object(const Object&) = delete;
                Object& operator=(const Object&) = delete;

                Object(Object&& other) noexcept
                    : _pool(other._pool), _index(other._index), _pointer(other._pointer),
                      _mutex(other._mutex), _initialized_state(other._initialized_state)
                {
                    other._pool = ft_nullptr;
                    other._index = 0;
                    other._pointer = ft_nullptr;
                    other._mutex = ft_nullptr;
                    other._initialized_state = 1;
                    set_last_operation_error(FT_ERR_SUCCESS);
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
                    this->_initialized_state = other._initialized_state;
                    other._pool = ft_nullptr;
                    other._index = 0;
                    other._pointer = ft_nullptr;
                    other._mutex = ft_nullptr;
                    other._initialized_state = 1;
                    set_last_operation_error(FT_ERR_SUCCESS);
                    return (*this);
                }

                T* operator->() const noexcept
                {
                    if (this->_pointer == ft_nullptr)
                    {
                        set_last_operation_error(FT_ERR_INVALID_HANDLE);
                        return (ft_nullptr);
                    }
                    set_last_operation_error(FT_ERR_SUCCESS);
                    return (this->_pointer);
                }

                explicit operator bool() const noexcept
                {
                    if (this->_pointer == ft_nullptr)
                    {
                        set_last_operation_error(FT_ERR_INVALID_HANDLE);
                        return (false);
                    }
                    set_last_operation_error(FT_ERR_SUCCESS);
                    return (true);
                }

                int enable_thread_safety()
                {
                    pt_recursive_mutex *new_mutex;
                    int initialize_result;

                    if (this->_mutex != ft_nullptr)
                        return (set_last_operation_error(FT_ERR_SUCCESS));
                    new_mutex = new (std::nothrow) pt_recursive_mutex();
                    if (new_mutex == ft_nullptr)
                        return (set_last_operation_error(FT_ERR_NO_MEMORY));
                    initialize_result = new_mutex->initialize();
                    if (initialize_result != FT_ERR_SUCCESS)
                    {
                        delete new_mutex;
                        return (set_last_operation_error(initialize_result));
                    }
                    this->_mutex = new_mutex;
                    return (set_last_operation_error(FT_ERR_SUCCESS));
                }

                int disable_thread_safety()
                {
                    pt_recursive_mutex *mutex_pointer;
                    int destroy_result;

                    mutex_pointer = this->_mutex;
                    if (mutex_pointer == ft_nullptr)
                        return (set_last_operation_error(FT_ERR_SUCCESS));
                    this->_mutex = ft_nullptr;
                    destroy_result = mutex_pointer->destroy();
                    delete mutex_pointer;
                    if (destroy_result != FT_ERR_SUCCESS)
                        return (set_last_operation_error(destroy_result));
                    return (set_last_operation_error(FT_ERR_SUCCESS));
                }

                bool is_thread_safe() const
                {
                    set_last_operation_error(FT_ERR_SUCCESS);
                    return (this->_mutex != ft_nullptr);
                }

                int lock(bool *lock_acquired) const
                {
                    int lock_result;

                    lock_result = this->lock_internal(lock_acquired);
                    if (lock_result != FT_ERR_SUCCESS)
                        return (-1);
                    set_last_operation_error(FT_ERR_SUCCESS);
                    return (0);
                }

                void unlock(bool lock_acquired) const
                {
                    (void)this->unlock_internal(lock_acquired);
                    return ;
                }

                static int32_t last_operation_error() noexcept
                {
                    return (_last_error);
                }

                static const char *last_operation_error_str() noexcept
                {
                    return (ft_strerror(_last_error));
                }

#ifdef LIBFT_TEST_BUILD
                pt_recursive_mutex* get_mutex_for_validation() const noexcept
                {
                    return (this->_mutex);
                }
#endif
        };

        Pool()
            : _slots(), _free_indices(), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~Pool()
        {
            if (this->_initialized_state == _state_uninitialized)
                this->abort_lifecycle_error("Pool::~Pool",
                    "destructor called while object is uninitialized");
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        Pool(Pool&& other) = delete;
        Pool& operator=(Pool&& other) = delete;
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        int initialize()
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("Pool::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_slots.clear();
            this->_free_indices.clear();
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int destroy()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            if (this->_initialized_state != _state_initialized)
            {
                this->abort_lifecycle_error("Pool::destroy",
                    "called while object is not initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            size_t index;

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
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            this->_initialized_state = _state_destroyed;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        void resize(size_t new_size)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t index;

            this->abort_if_not_initialized("Pool::resize");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
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
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        template<typename... Args>
        Object acquire(Args&&... args)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t free_count;
            size_t last_index;
            size_t storage_index;
            T *constructed_pointer;
            Object result;

            this->abort_if_not_initialized("Pool::acquire");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (result);
            }
            free_count = this->_free_indices.size();
            if (free_count == 0)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_EMPTY);
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
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return (result);
            }
            construct_at(constructed_pointer, std::forward<Args>(args)...);
            this->_slots[storage_index] = constructed_pointer;
            result = Object(this, storage_index, constructed_pointer);
            if (this->_mutex != ft_nullptr)
                (void)result.enable_thread_safety();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                set_last_operation_error(unlock_error);
            else
                set_last_operation_error(FT_ERR_SUCCESS);
            return (result);
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            this->abort_if_not_initialized("Pool::enable_thread_safety");
            if (this->_mutex != ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            new_mutex = new (std::nothrow) pt_recursive_mutex();
            if (new_mutex == ft_nullptr)
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            initialize_result = new_mutex->initialize();
            if (initialize_result != FT_ERR_SUCCESS)
            {
                delete new_mutex;
                return (set_last_operation_error(initialize_result));
            }
            this->_mutex = new_mutex;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int destroy_result;

            if (this->_initialized_state != _state_initialized
                && this->_initialized_state != _state_destroyed)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            mutex_pointer = this->_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            this->_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(destroy_result));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            this->abort_if_not_initialized("Pool::is_thread_safe");
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_result;

            this->abort_if_not_initialized("Pool::lock");
            lock_result = this->lock_internal(lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (-1);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (0);
        }

        void unlock(bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t last_operation_error() noexcept
        {
            return (_last_error);
        }

        static const char *last_operation_error_str() noexcept
        {
            return (ft_strerror(_last_error));
        }

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept
        {
            return (this->_mutex);
        }
#endif
};

template<typename T>
thread_local int32_t Pool<T>::_last_error = FT_ERR_SUCCESS;

template<typename T>
thread_local int32_t Pool<T>::Object::_last_error = FT_ERR_SUCCESS;

#endif
