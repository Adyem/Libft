#ifndef FT_CANCELLATION_HPP
#define FT_CANCELLATION_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "function.hpp"
#include "shared_ptr.hpp"
#include "vector.hpp"
#include "move.hpp"
#include <atomic>
#include <pthread.h>
#include <errno.h>
#include <new>
#include <type_traits>

namespace ft_cancellation_detail
{
    class ft_cancellation_state
    {
        private:
            mutable int _error_code;
            std::atomic<bool> _cancelled;
            ft_vector<ft_function<void()> > _callbacks;
            pthread_mutex_t _mutex;
            bool _mutex_initialized;

            void set_error(int error_code) const;

        public:
            ft_cancellation_state() noexcept;
            ~ft_cancellation_state() noexcept;

            ft_cancellation_state(const ft_cancellation_state&) = delete;
            ft_cancellation_state &operator=(const ft_cancellation_state&) = delete;

            int register_callback(const ft_function<void()> &callback) noexcept;
            void request_cancel() noexcept;
            bool is_cancelled() const noexcept;
            int get_error() const noexcept;
            const char *get_error_str() const noexcept;
    };
}

class ft_cancellation_source;

class ft_cancellation_token
{
    private:
        ft_sharedptr<ft_cancellation_detail::ft_cancellation_state> _state;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

        friend class ft_cancellation_source;

    public:
        ft_cancellation_token() noexcept;
        ~ft_cancellation_token() noexcept;

        ft_cancellation_token(const ft_cancellation_token &other) noexcept;
        ft_cancellation_token &operator=(const ft_cancellation_token &other) noexcept;
        ft_cancellation_token(ft_cancellation_token &&other) noexcept;
        ft_cancellation_token &operator=(ft_cancellation_token &&other) noexcept;

        bool is_valid() const noexcept;
        bool is_cancellation_requested() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        int register_callback(const ft_function<void()> &callback) const noexcept;
};

class ft_cancellation_source
{
    private:
        ft_sharedptr<ft_cancellation_detail::ft_cancellation_state> _state;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    public:
        ft_cancellation_source() noexcept;
        ~ft_cancellation_source() noexcept;

        ft_cancellation_source(const ft_cancellation_source &other) noexcept;
        ft_cancellation_source &operator=(const ft_cancellation_source &other) noexcept;
        ft_cancellation_source(ft_cancellation_source &&other) noexcept;
        ft_cancellation_source &operator=(ft_cancellation_source &&other) noexcept;

        ft_cancellation_token get_token() const noexcept;
        void request_cancel() noexcept;
        bool is_cancellation_requested() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

namespace ft_cancellation_detail
{
    inline void ft_cancellation_state::set_error(int error_code) const
    {
        ft_errno = error_code;
        this->_error_code = error_code;
        return ;
    }

    inline ft_cancellation_state::ft_cancellation_state() noexcept
        : _error_code(FT_ER_SUCCESSS),
          _cancelled(false),
          _callbacks(),
          _mutex(),
          _mutex_initialized(false)
    {
        if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
        {
            this->set_error(ft_map_system_error(errno));
            return ;
        }
        this->_mutex_initialized = true;
        this->set_error(FT_ER_SUCCESSS);
        return ;
    }

    inline ft_cancellation_state::~ft_cancellation_state() noexcept
    {
        if (this->_mutex_initialized)
            pthread_mutex_destroy(&this->_mutex);
        this->set_error(FT_ER_SUCCESSS);
        return ;
    }

    inline int ft_cancellation_state::register_callback(const ft_function<void()> &callback) noexcept
    {
        if (this->_mutex_initialized == false)
        {
            this->set_error(FT_ERR_INVALID_STATE);
            return (FT_ERR_INVALID_STATE);
        }
        if (pthread_mutex_lock(&this->_mutex) != 0)
        {
            this->set_error(ft_map_system_error(errno));
            return (this->_error_code);
        }
        bool already_cancelled;

        already_cancelled = this->_cancelled.load(std::memory_order_acquire);
        if (already_cancelled == false)
        {
            this->_callbacks.push_back(callback);
            if (this->_callbacks.get_error() != FT_ER_SUCCESSS)
            {
                int push_error;

                push_error = this->_callbacks.get_error();
                pthread_mutex_unlock(&this->_mutex);
                this->set_error(push_error);
                return (push_error);
            }
        }
        if (pthread_mutex_unlock(&this->_mutex) != 0)
        {
            this->set_error(ft_map_system_error(errno));
            return (this->_error_code);
        }
        if (already_cancelled)
        {
            ft_function<void()> callback_copy(callback);

            callback_copy();
        }
        this->set_error(FT_ER_SUCCESSS);
        return (FT_ER_SUCCESSS);
    }

    inline void ft_cancellation_state::request_cancel() noexcept
    {
        if (this->_cancelled.exchange(true, std::memory_order_acq_rel))
        {
            this->set_error(FT_ER_SUCCESSS);
            return ;
        }
        ft_vector<ft_function<void()> > callbacks_to_run;

        if (this->_mutex_initialized)
        {
            if (pthread_mutex_lock(&this->_mutex) != 0)
            {
                this->set_error(ft_map_system_error(errno));
                return ;
            }
            size_t callback_index;

            callback_index = 0;
            while (callback_index < this->_callbacks.size())
            {
                callbacks_to_run.push_back(this->_callbacks[callback_index]);
                if (callbacks_to_run.get_error() != FT_ER_SUCCESSS)
                {
                    int copy_error;

                    copy_error = callbacks_to_run.get_error();
                    pthread_mutex_unlock(&this->_mutex);
                    this->set_error(copy_error);
                    return ;
                }
                ++callback_index;
            }
            this->_callbacks.clear();
            if (this->_callbacks.get_error() != FT_ER_SUCCESSS)
            {
                int clear_error;

                clear_error = this->_callbacks.get_error();
                pthread_mutex_unlock(&this->_mutex);
                this->set_error(clear_error);
                return ;
            }
            if (pthread_mutex_unlock(&this->_mutex) != 0)
            {
                this->set_error(ft_map_system_error(errno));
                return ;
            }
        }
        size_t invoke_index;

        invoke_index = 0;
        while (invoke_index < callbacks_to_run.size())
        {
            callbacks_to_run[invoke_index]();
            ++invoke_index;
        }
        this->set_error(FT_ER_SUCCESSS);
        return ;
    }

    inline bool ft_cancellation_state::is_cancelled() const noexcept
    {
        return (this->_cancelled.load(std::memory_order_acquire));
    }

    inline int ft_cancellation_state::get_error() const noexcept
    {
        ft_errno = this->_error_code;
        return (this->_error_code);
    }

    inline const char *ft_cancellation_state::get_error_str() const noexcept
    {
        return (ft_strerror(this->_error_code));
    }
}

inline void ft_cancellation_token::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

inline ft_cancellation_token::ft_cancellation_token() noexcept
    : _state(), _error_code(FT_ER_SUCCESSS)
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

inline ft_cancellation_token::~ft_cancellation_token() noexcept
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

inline ft_cancellation_token::ft_cancellation_token(const ft_cancellation_token &other) noexcept
    : _state(other._state), _error_code(FT_ER_SUCCESSS)
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

inline ft_cancellation_token &ft_cancellation_token::operator=(const ft_cancellation_token &other) noexcept
{
    if (this != &other)
        this->_state = other._state;
    this->set_error(FT_ER_SUCCESSS);
    return (*this);
}

inline ft_cancellation_token::ft_cancellation_token(ft_cancellation_token &&other) noexcept
    : _state(ft_move(other._state)), _error_code(FT_ER_SUCCESSS)
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

inline ft_cancellation_token &ft_cancellation_token::operator=(ft_cancellation_token &&other) noexcept
{
    if (this != &other)
        this->_state = ft_move(other._state);
    this->set_error(FT_ER_SUCCESSS);
    return (*this);
}

inline bool ft_cancellation_token::is_valid() const noexcept
{
    bool valid_state;

    valid_state = static_cast<bool>(this->_state);
    this->set_error(FT_ER_SUCCESSS);
    return (valid_state);
}

inline bool ft_cancellation_token::is_cancellation_requested() const noexcept
{
    if (!this->is_valid())
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (true);
    }
    const ft_cancellation_detail::ft_cancellation_state *state_pointer;

    state_pointer = this->_state.get();
    if (!state_pointer)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (true);
    }
    bool cancelled;

    cancelled = state_pointer->is_cancelled();
    this->set_error(FT_ER_SUCCESSS);
    return (cancelled);
}

inline int ft_cancellation_token::get_error() const noexcept
{
    ft_errno = this->_error_code;
    return (this->_error_code);
}

inline const char *ft_cancellation_token::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

inline int ft_cancellation_token::register_callback(const ft_function<void()> &callback) const noexcept
{
    if (!this->is_valid())
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    const ft_cancellation_detail::ft_cancellation_state *state_pointer_const;
    ft_cancellation_detail::ft_cancellation_state *state_pointer;

    state_pointer_const = this->_state.get();
    if (!state_pointer_const)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    state_pointer = const_cast<ft_cancellation_detail::ft_cancellation_state*>(state_pointer_const);
    int status;

    status = state_pointer->register_callback(callback);
    this->set_error(status);
    return (status);
}

inline void ft_cancellation_source::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

inline ft_cancellation_source::ft_cancellation_source() noexcept
    : _state(), _error_code(FT_ER_SUCCESSS)
{
    ft_cancellation_detail::ft_cancellation_state *state_pointer;

    state_pointer = new (std::nothrow) ft_cancellation_detail::ft_cancellation_state();
    if (!state_pointer)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    this->_state.reset(state_pointer, 1, false);
    if (this->_state.get_error() != FT_ER_SUCCESSS)
    {
        int allocation_error;

        allocation_error = this->_state.get_error();
        this->set_error(allocation_error);
        return ;
    }
    if (state_pointer->get_error() != FT_ER_SUCCESSS)
    {
        int state_error;

        state_error = state_pointer->get_error();
        this->_state.reset();
        this->set_error(state_error);
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

inline ft_cancellation_source::~ft_cancellation_source() noexcept
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

inline ft_cancellation_source::ft_cancellation_source(const ft_cancellation_source &other) noexcept
    : _state(other._state), _error_code(FT_ER_SUCCESSS)
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

inline ft_cancellation_source &ft_cancellation_source::operator=(const ft_cancellation_source &other) noexcept
{
    if (this != &other)
        this->_state = other._state;
    this->set_error(FT_ER_SUCCESSS);
    return (*this);
}

inline ft_cancellation_source::ft_cancellation_source(ft_cancellation_source &&other) noexcept
    : _state(ft_move(other._state)), _error_code(FT_ER_SUCCESSS)
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

inline ft_cancellation_source &ft_cancellation_source::operator=(ft_cancellation_source &&other) noexcept
{
    if (this != &other)
        this->_state = ft_move(other._state);
    this->set_error(FT_ER_SUCCESSS);
    return (*this);
}

inline ft_cancellation_token ft_cancellation_source::get_token() const noexcept
{
    ft_cancellation_token token;

    token = ft_cancellation_token();
    if (!static_cast<bool>(this->_state))
    {
        token.set_error(FT_ERR_INVALID_STATE);
        this->set_error(FT_ERR_INVALID_STATE);
        return (token);
    }
    token._state = this->_state;
    token.set_error(FT_ER_SUCCESSS);
    this->set_error(FT_ER_SUCCESSS);
    return (token);
}

inline void ft_cancellation_source::request_cancel() noexcept
{
    if (!static_cast<bool>(this->_state))
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    ft_cancellation_detail::ft_cancellation_state *state_pointer;

    state_pointer = this->_state.get();
    if (!state_pointer)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    state_pointer->request_cancel();
    this->set_error(state_pointer->get_error());
    return ;
}

inline bool ft_cancellation_source::is_cancellation_requested() const noexcept
{
    if (!static_cast<bool>(this->_state))
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (true);
    }
    const ft_cancellation_detail::ft_cancellation_state *state_pointer;

    state_pointer = this->_state.get();
    if (!state_pointer)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (true);
    }
    bool cancelled;

    cancelled = state_pointer->is_cancelled();
    this->set_error(FT_ER_SUCCESSS);
    return (cancelled);
}

inline int ft_cancellation_source::get_error() const noexcept
{
    ft_errno = this->_error_code;
    return (this->_error_code);
}

inline const char *ft_cancellation_source::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

#endif
