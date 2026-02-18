#ifndef FT_EVENT_EMITTER_HPP
#define FT_EVENT_EMITTER_HPP

#include "constructor.hpp"
#include "move.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename EventType, typename... Args>
class ft_event_emitter
{
    private:
        struct Listener
        {
            EventType _event;
            void (*_callback)(Args...);
        };

        Listener                   *_listeners;
        size_t                      _configured_initial_capacity;
        size_t                      _capacity;
        size_t                      _size;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                     _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        static thread_local int32_t _last_error;

        static int32_t set_last_operation_error(int32_t error_code) noexcept
        {
            ft_event_emitter<EventType, Args...>::_last_error = error_code;
            return (error_code);
        }

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const
        {
            if (method_name == ft_nullptr)
                method_name = "unknown";
            if (reason == ft_nullptr)
                reason = "unknown";
            pf_printf_fd(2, "ft_event_emitter lifecycle error: %s: %s\n",
                method_name, reason);
            su_abort();
            return ;
        }

        void abort_if_not_initialized(const char *method_name) const
        {
            if (this->_initialized_state == ft_event_emitter<EventType, Args...>::_state_initialized)
                return ;
            this->abort_lifecycle_error(method_name,
                "called while object is not initialized");
            return ;
        }

        int lock_internal(bool *lock_acquired) const
        {
            int lock_error;

            if (lock_acquired != ft_nullptr)
                *lock_acquired = false;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_error = this->_mutex->lock();
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        int unlock_internal(bool lock_acquired) const
        {
            int unlock_error;

            if (lock_acquired == false)
                return (FT_ERR_SUCCESS);
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            unlock_error = this->_mutex->unlock();
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            return (FT_ERR_SUCCESS);
        }

        bool ensure_capacity_unlocked(size_t desired)
        {
            size_t maximum_capacity;
            size_t new_capacity;
            Listener *new_data;
            size_t listener_index;

            if (desired <= this->_capacity)
                return (true);
            maximum_capacity = FT_SYSTEM_SIZE_MAX / sizeof(Listener);
            if (desired > maximum_capacity)
            {
                set_last_operation_error(FT_ERR_OUT_OF_RANGE);
                return (false);
            }
            if (this->_capacity == 0)
                new_capacity = 1;
            else
                new_capacity = this->_capacity;
            while (new_capacity < desired)
            {
                if (new_capacity >= maximum_capacity)
                {
                    new_capacity = maximum_capacity;
                    break ;
                }
                if (new_capacity > (maximum_capacity / 2))
                    new_capacity = maximum_capacity;
                else
                    new_capacity = new_capacity * 2;
            }
            new_data = static_cast<Listener *>(cma_malloc(sizeof(Listener) * new_capacity));
            if (new_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return (false);
            }
            listener_index = 0;
            while (listener_index < this->_size)
            {
                construct_at(&new_data[listener_index],
                    ft_move(this->_listeners[listener_index]));
                destroy_at(&this->_listeners[listener_index]);
                listener_index = listener_index + 1;
            }
            if (this->_listeners != ft_nullptr)
                cma_free(this->_listeners);
            this->_listeners = new_data;
            this->_capacity = new_capacity;
            return (true);
        }

        bool ensure_capacity(size_t desired)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            bool ensured;

            this->abort_if_not_initialized("ft_event_emitter::ensure_capacity");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (false);
            }
            ensured = this->ensure_capacity_unlocked(desired);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (false);
            }
            if (ensured == false)
                return (false);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (true);
        }

        void clear_unlocked()
        {
            size_t listener_index;

            listener_index = 0;
            while (listener_index < this->_size)
            {
                destroy_at(&this->_listeners[listener_index]);
                listener_index = listener_index + 1;
            }
            this->_size = 0;
            return ;
        }

        template <typename, typename...>
        friend struct ft_event_emitter_test_helper;

    public:
        ft_event_emitter(size_t initial_capacity = 0)
            : _listeners(ft_nullptr),
              _configured_initial_capacity(initial_capacity),
              _capacity(0),
              _size(0),
              _mutex(ft_nullptr),
              _initialized_state(ft_event_emitter<EventType, Args...>::_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_event_emitter()
        {
            if (this->_initialized_state == ft_event_emitter<EventType, Args...>::_state_uninitialized)
                this->abort_lifecycle_error("ft_event_emitter::~ft_event_emitter",
                    "destructor called while object is uninitialized");
            if (this->_initialized_state == ft_event_emitter<EventType, Args...>::_state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_event_emitter(const ft_event_emitter&) = delete;
        ft_event_emitter& operator=(const ft_event_emitter&) = delete;
        ft_event_emitter(ft_event_emitter&&) = delete;
        ft_event_emitter& operator=(ft_event_emitter&&) = delete;

        int initialize()
        {
            if (this->_initialized_state == ft_event_emitter<EventType, Args...>::_state_initialized)
            {
                this->abort_lifecycle_error("ft_event_emitter::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_listeners = ft_nullptr;
            this->_capacity = 0;
            this->_size = 0;
            if (this->_configured_initial_capacity > 0)
            {
                this->_listeners = static_cast<Listener *>(
                        cma_malloc(sizeof(Listener) * this->_configured_initial_capacity));
                if (this->_listeners == ft_nullptr)
                {
                    this->_initialized_state = ft_event_emitter<EventType, Args...>::_state_destroyed;
                    return (set_last_operation_error(FT_ERR_NO_MEMORY));
                }
                this->_capacity = this->_configured_initial_capacity;
            }
            this->_initialized_state = ft_event_emitter<EventType, Args...>::_state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int destroy()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            int disable_error;

            if (this->_initialized_state != ft_event_emitter<EventType, Args...>::_state_initialized)
            {
                this->abort_lifecycle_error("ft_event_emitter::destroy",
                    "called while object is not initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            this->clear_unlocked();
            if (this->_listeners != ft_nullptr)
            {
                cma_free(this->_listeners);
                this->_listeners = ft_nullptr;
            }
            this->_capacity = 0;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            disable_error = this->disable_thread_safety();
            this->_initialized_state = ft_event_emitter<EventType, Args...>::_state_destroyed;
            if (disable_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(disable_error));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int mutex_error;

            this->abort_if_not_initialized("ft_event_emitter::enable_thread_safety");
            if (this->_mutex != ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            mutex_pointer = new (std::nothrow) pt_recursive_mutex();
            if (mutex_pointer == ft_nullptr)
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            mutex_error = mutex_pointer->initialize();
            if (mutex_error != FT_ERR_SUCCESS)
            {
                delete mutex_pointer;
                return (set_last_operation_error(mutex_error));
            }
            this->_mutex = mutex_pointer;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int disable_thread_safety()
        {
            int destroy_error;

            if (this->_initialized_state == ft_event_emitter<EventType, Args...>::_state_initialized)
                this->abort_if_not_initialized("ft_event_emitter::disable_thread_safety");
            if (this->_mutex == ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            destroy_error = this->_mutex->destroy();
            delete this->_mutex;
            this->_mutex = ft_nullptr;
            return (set_last_operation_error(destroy_error));
        }

        bool is_thread_safe() const
        {
            this->abort_if_not_initialized("ft_event_emitter::is_thread_safe");
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_error;

            this->abort_if_not_initialized("ft_event_emitter::lock");
            lock_error = this->lock_internal(lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        void unlock(bool lock_acquired) const
        {
            int unlock_error;

            this->abort_if_not_initialized("ft_event_emitter::unlock");
            unlock_error = this->unlock_internal(lock_acquired);
            set_last_operation_error(unlock_error);
            return ;
        }

        void on(const EventType& event, void (*callback)(Args...))
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_event_emitter::on");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (this->ensure_capacity_unlocked(this->_size + 1) == false)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                return ;
            }
            construct_at(&this->_listeners[this->_size], Listener{event, callback});
            this->_size = this->_size + 1;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        void emit(const EventType& event, Args... args)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            bool found;
            size_t listener_index;

            this->abort_if_not_initialized("ft_event_emitter::emit");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            found = false;
            listener_index = 0;
            while (listener_index < this->_size)
            {
                if (this->_listeners[listener_index]._event == event)
                {
                    found = true;
                    this->_listeners[listener_index]._callback(args...);
                }
                listener_index = listener_index + 1;
            }
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            if (found == false)
            {
                set_last_operation_error(FT_ERR_NOT_FOUND);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        void remove_listener(const EventType& event, void (*callback)(Args...))
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t listener_index;
            size_t shift_index;

            this->abort_if_not_initialized("ft_event_emitter::remove_listener");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            listener_index = 0;
            while (listener_index < this->_size)
            {
                if (this->_listeners[listener_index]._event == event &&
                    this->_listeners[listener_index]._callback == callback)
                {
                    destroy_at(&this->_listeners[listener_index]);
                    shift_index = listener_index;
                    while ((shift_index + 1) < this->_size)
                    {
                        construct_at(&this->_listeners[shift_index],
                            ft_move(this->_listeners[shift_index + 1]));
                        destroy_at(&this->_listeners[shift_index + 1]);
                        shift_index = shift_index + 1;
                    }
                    this->_size = this->_size - 1;
                    unlock_error = this->unlock_internal(lock_acquired);
                    if (unlock_error != FT_ERR_SUCCESS)
                    {
                        set_last_operation_error(unlock_error);
                        return ;
                    }
                    set_last_operation_error(FT_ERR_SUCCESS);
                    return ;
                }
                listener_index = listener_index + 1;
            }
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_NOT_FOUND);
            return ;
        }

        size_t size() const
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t current_size;

            this->abort_if_not_initialized("ft_event_emitter::size");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (0);
            }
            current_size = this->_size;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (0);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (current_size);
        }

        bool empty() const
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            bool is_empty;

            this->abort_if_not_initialized("ft_event_emitter::empty");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (true);
            }
            is_empty = (this->_size == 0);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (true);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (is_empty);
        }

        void clear()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_event_emitter::clear");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            this->clear_unlocked();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        int get_error() const
        {
            return (static_cast<int>(ft_event_emitter<EventType, Args...>::_last_error));
        }

        const char *get_error_str() const
        {
            return (ft_strerror(this->get_error()));
        }

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept
        {
            return (this->_mutex);
        }
#endif
};

template <typename EventType, typename... Args>
thread_local int32_t ft_event_emitter<EventType, Args...>::_last_error = FT_ERR_SUCCESS;

template <typename EventType, typename... Args>
struct ft_event_emitter_test_helper
{
    static bool ensure_capacity(ft_event_emitter<EventType, Args...> &emitter,
        size_t desired)
    {
        return (emitter.ensure_capacity(desired));
    }
};

#endif
