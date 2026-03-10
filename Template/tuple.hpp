#ifndef FT_TUPLE_HPP
#define FT_TUPLE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <tuple>
#include <utility>
#include <new>
#include <type_traits>
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"

template <typename... Types>
class ft_tuple
{
    private:
        using tuple_t = std::tuple<Types...>;

        tuple_t                     *_data;
        mutable pt_recursive_mutex  *_mutex;
        static thread_local int32_t  _last_error;

        static int32_t set_error(int32_t error_code)
        {
            _last_error = error_code;
            return (error_code);
        }

        int32_t lock_internal(ft_bool *lock_acquired) const
        {
            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_FALSE;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
                return (set_error(FT_ERR_SYS_MUTEX_LOCK_FAILED));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_TRUE;
            return (FT_ERR_SUCCESS);
        }

        int32_t unlock_internal(ft_bool lock_acquired) const
        {
            if (lock_acquired == FT_FALSE || this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            return (FT_ERR_SUCCESS);
        }

        void destroy_locked()
        {
            if (this->_data != ft_nullptr)
            {
                ::destroy_at(this->_data);
                cma_free(this->_data);
                this->_data = ft_nullptr;
            }
            return ;
        }

        template <typename ValueType>
        static ValueType &fallback_reference()
        {
            static ValueType default_value = ValueType();
            return (default_value);
        }

    public:
        ft_tuple()
            : _data(ft_nullptr), _mutex(ft_nullptr)
        {
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename... Args>
        explicit ft_tuple(Args&&... args)
            : _data(ft_nullptr), _mutex(ft_nullptr)
        {
            this->_data = static_cast<tuple_t *>(cma_malloc(sizeof(tuple_t)));
            if (this->_data == ft_nullptr)
            {
                set_error(FT_ERR_NO_MEMORY);
                return ;
            }
            ::construct_at(this->_data, std::forward<Args>(args)...);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_tuple()
        {
            ft_bool lock_acquired;

            lock_acquired = FT_FALSE;
            if (this->lock_internal(&lock_acquired) == FT_ERR_SUCCESS)
            {
                this->destroy_locked();
                (void)this->unlock_internal(lock_acquired);
            }
            (void)this->disable_thread_safety();
            return ;
        }

        ft_tuple(const ft_tuple&) = delete;
        ft_tuple& operator=(const ft_tuple&) = delete;
        ft_tuple(ft_tuple&& other) = delete;
        ft_tuple& operator=(ft_tuple&& other) = delete;

        template <ft_size_t I>
        typename std::tuple_element<I, tuple_t>::type& get()
        {
            using elem_t = typename std::tuple_element<I, tuple_t>::type;
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (fallback_reference<elem_t>());
            if (this->_data == ft_nullptr)
            {
                set_error(FT_ERR_INVALID_OPERATION);
                (void)this->unlock_internal(lock_acquired);
                return (fallback_reference<elem_t>());
            }
            elem_t &reference = std::get<I>(*this->_data);
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (reference);
        }

        template <ft_size_t I>
        const typename std::tuple_element<I, tuple_t>::type& get() const
        {
            using elem_t = typename std::tuple_element<I, tuple_t>::type;
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (fallback_reference<elem_t>());
            if (this->_data == ft_nullptr)
            {
                set_error(FT_ERR_INVALID_OPERATION);
                (void)this->unlock_internal(lock_acquired);
                return (fallback_reference<elem_t>());
            }
            const elem_t &reference = std::get<I>(*this->_data);
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (reference);
        }

        template <typename T>
        T& get()
        {
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (fallback_reference<T>());
            if (this->_data == ft_nullptr)
            {
                set_error(FT_ERR_INVALID_OPERATION);
                (void)this->unlock_internal(lock_acquired);
                return (fallback_reference<T>());
            }
            T &reference = std::get<T>(*this->_data);
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (reference);
        }

        template <typename T>
        const T& get() const
        {
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (fallback_reference<T>());
            if (this->_data == ft_nullptr)
            {
                set_error(FT_ERR_INVALID_OPERATION);
                (void)this->unlock_internal(lock_acquired);
                return (fallback_reference<T>());
            }
            const T &reference = std::get<T>(*this->_data);
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (reference);
        }

        void reset()
        {
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            this->destroy_locked();
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
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
            return (set_error(this->lock_internal(lock_acquired)));
        }

        void unlock(ft_bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t get_error()
        {
            return (_last_error);
        }

        static const char *get_error_str()
        {
            return (ft_strerror(_last_error));
        }

};

template <typename... Types>
thread_local int32_t ft_tuple<Types...>::_last_error = FT_ERR_SUCCESS;

#endif
