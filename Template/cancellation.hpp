#ifndef FT_CANCELLATION_HPP
#define FT_CANCELLATION_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "function.hpp"
#include "vector.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <atomic>
#include <new>

class ft_cancellation_state
{
    private:
        std::atomic<bool>          _cancelled;
        ft_vector<ft_function<void()> > _callbacks;
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
            pf_printf_fd(2, "ft_cancellation_state lifecycle error: %s: %s\n",
                method_name, reason);
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

    public:
        ft_cancellation_state() noexcept
            : _cancelled(false), _callbacks(), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_cancellation_state() noexcept
        {
            if (this->_initialized_state == _state_uninitialized)
                this->abort_lifecycle_error("ft_cancellation_state::~ft_cancellation_state",
                    "destructor called while object is uninitialized");
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_cancellation_state(const ft_cancellation_state&) = delete;
        ft_cancellation_state &operator=(const ft_cancellation_state&) = delete;
        ft_cancellation_state(ft_cancellation_state&&) = delete;
        ft_cancellation_state &operator=(ft_cancellation_state&&) = delete;

        int initialize() noexcept
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_cancellation_state::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_cancelled.store(false, std::memory_order_release);
            this->_callbacks.clear();
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int destroy() noexcept
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            if (this->_initialized_state != _state_initialized)
            {
                this->abort_lifecycle_error("ft_cancellation_state::destroy",
                    "called while object is not initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            this->_callbacks.clear();
            this->_cancelled.store(false, std::memory_order_release);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            this->_initialized_state = _state_destroyed;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int register_callback(const ft_function<void()> &callback) noexcept
        {
            bool already_cancelled;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_cancellation_state::register_callback");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            already_cancelled = this->_cancelled.load(std::memory_order_acquire);
            if (!already_cancelled)
                this->_callbacks.push_back(callback);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            if (already_cancelled)
                callback();
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int request_cancel() noexcept
        {
            ft_vector<ft_function<void()> > callbacks_to_run;
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t index;

            this->abort_if_not_initialized("ft_cancellation_state::request_cancel");
            if (this->_cancelled.exchange(true, std::memory_order_acq_rel))
                return (set_last_operation_error(FT_ERR_SUCCESS));
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            index = 0;
            while (index < this->_callbacks.size())
            {
                callbacks_to_run.push_back(this->_callbacks[index]);
                ++index;
            }
            this->_callbacks.clear();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            index = 0;
            while (index < callbacks_to_run.size())
            {
                callbacks_to_run[index]();
                ++index;
            }
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        bool is_cancelled() const noexcept
        {
            bool cancelled;

            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (true);
            }
            cancelled = this->_cancelled.load(std::memory_order_acquire);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (cancelled);
        }

        int enable_thread_safety() noexcept
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            this->abort_if_not_initialized("ft_cancellation_state::enable_thread_safety");
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

        int disable_thread_safety() noexcept
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

        static int32_t last_operation_error() noexcept
        {
            return (_last_error);
        }
};

class ft_cancellation_source;

class ft_cancellation_token
{
    private:
        ft_cancellation_state *_state;

        friend class ft_cancellation_source;

        explicit ft_cancellation_token(ft_cancellation_state *state_pointer) noexcept
            : _state(state_pointer)
        {
            return ;
        }

    public:
        ft_cancellation_token() noexcept : _state(ft_nullptr) { return ; }
        ~ft_cancellation_token() noexcept { return ; }

        ft_cancellation_token(const ft_cancellation_token&) = delete;
        ft_cancellation_token &operator=(const ft_cancellation_token&) = delete;
        ft_cancellation_token(ft_cancellation_token&&) = delete;
        ft_cancellation_token &operator=(ft_cancellation_token&&) = delete;

        bool is_valid() const noexcept
        {
            return (this->_state != ft_nullptr);
        }

        bool is_cancellation_requested() const noexcept
        {
            if (this->_state == ft_nullptr)
                return (true);
            return (this->_state->is_cancelled());
        }

        int register_callback(const ft_function<void()> &callback) const noexcept
        {
            if (this->_state == ft_nullptr)
                return (FT_ERR_INVALID_STATE);
            return (this->_state->register_callback(callback));
        }
};

class ft_cancellation_source
{
    private:
        ft_cancellation_state *_state;
        uint8_t                _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const
        {
            if (method_name == ft_nullptr)
                method_name = "unknown";
            if (reason == ft_nullptr)
                reason = "unknown";
            pf_printf_fd(2, "ft_cancellation_source lifecycle error: %s: %s\n",
                method_name, reason);
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

    public:
        ft_cancellation_source() noexcept
            : _state(ft_nullptr), _initialized_state(_state_uninitialized)
        {
            return ;
        }

        ~ft_cancellation_source() noexcept
        {
            if (this->_initialized_state == _state_uninitialized)
                this->abort_lifecycle_error("ft_cancellation_source::~ft_cancellation_source",
                    "destructor called while object is uninitialized");
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            return ;
        }

        ft_cancellation_source(const ft_cancellation_source&) = delete;
        ft_cancellation_source &operator=(const ft_cancellation_source&) = delete;
        ft_cancellation_source(ft_cancellation_source&&) = delete;
        ft_cancellation_source &operator=(ft_cancellation_source&&) = delete;

        int initialize() noexcept
        {
            this->_state = new (std::nothrow) ft_cancellation_state();
            if (this->_state == ft_nullptr)
            {
                this->_initialized_state = _state_destroyed;
                return (FT_ERR_NO_MEMORY);
            }
            if (this->_state->initialize() != FT_ERR_SUCCESS)
            {
                delete this->_state;
                this->_state = ft_nullptr;
                this->_initialized_state = _state_destroyed;
                return (FT_ERR_INVALID_STATE);
            }
            this->_initialized_state = _state_initialized;
            return (FT_ERR_SUCCESS);
        }

        int destroy() noexcept
        {
            this->abort_if_not_initialized("ft_cancellation_source::destroy");
            if (this->_state != ft_nullptr)
            {
                delete this->_state;
                this->_state = ft_nullptr;
            }
            this->_initialized_state = _state_destroyed;
            return (FT_ERR_SUCCESS);
        }

        ft_cancellation_token get_token() const noexcept
        {
            this->abort_if_not_initialized("ft_cancellation_source::get_token");
            return (ft_cancellation_token(this->_state));
        }

        int request_cancel() noexcept
        {
            this->abort_if_not_initialized("ft_cancellation_source::request_cancel");
            if (this->_state == ft_nullptr)
                return (FT_ERR_INVALID_STATE);
            return (this->_state->request_cancel());
        }

        bool is_cancellation_requested() const noexcept
        {
            this->abort_if_not_initialized("ft_cancellation_source::is_cancellation_requested");
            if (this->_state == ft_nullptr)
                return (true);
            return (this->_state->is_cancelled());
        }
};

thread_local int32_t ft_cancellation_state::_last_error = FT_ERR_SUCCESS;

#endif
