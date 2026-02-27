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

        static int32_t set_last_operation_error(int32_t error_code)
        {
            _last_error = error_code;
            return (error_code);
        }

        int lock_internal(bool *lock_acquired) const
        {
            if (lock_acquired != ft_nullptr)
                *lock_acquired = false;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
                return (set_last_operation_error(FT_ERR_SYS_MUTEX_LOCK_FAILED));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        int unlock_internal(bool lock_acquired) const
        {
            if (lock_acquired == false || this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (pt_recursive_mutex_unlock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
                return (set_last_operation_error(FT_ERR_SYS_MUTEX_UNLOCK_FAILED));
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
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename... Args>
        explicit ft_tuple(Args&&... args)
            : _data(ft_nullptr), _mutex(ft_nullptr)
        {
            this->_data = static_cast<tuple_t *>(cma_malloc(sizeof(tuple_t)));
            if (this->_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return ;
            }
            ::construct_at(this->_data, std::forward<Args>(args)...);
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_tuple()
        {
            bool lock_acquired;

            lock_acquired = false;
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

        template <std::size_t I>
        typename std::tuple_element<I, tuple_t>::type& get()
        {
            using elem_t = typename std::tuple_element<I, tuple_t>::type;
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (fallback_reference<elem_t>());
            if (this->_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_INVALID_OPERATION);
                (void)this->unlock_internal(lock_acquired);
                return (fallback_reference<elem_t>());
            }
            elem_t &reference = std::get<I>(*this->_data);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (reference);
        }

        template <std::size_t I>
        const typename std::tuple_element<I, tuple_t>::type& get() const
        {
            using elem_t = typename std::tuple_element<I, tuple_t>::type;
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (fallback_reference<elem_t>());
            if (this->_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_INVALID_OPERATION);
                (void)this->unlock_internal(lock_acquired);
                return (fallback_reference<elem_t>());
            }
            const elem_t &reference = std::get<I>(*this->_data);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (reference);
        }

        template <typename T>
        T& get()
        {
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (fallback_reference<T>());
            if (this->_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_INVALID_OPERATION);
                (void)this->unlock_internal(lock_acquired);
                return (fallback_reference<T>());
            }
            T &reference = std::get<T>(*this->_data);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (reference);
        }

        template <typename T>
        const T& get() const
        {
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (fallback_reference<T>());
            if (this->_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_INVALID_OPERATION);
                (void)this->unlock_internal(lock_acquired);
                return (fallback_reference<T>());
            }
            const T &reference = std::get<T>(*this->_data);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (reference);
        }

        void reset()
        {
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            this->destroy_locked();
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
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
            if (this->lock_internal(lock_acquired) != FT_ERR_SUCCESS)
                return (-1);
            return (0);
        }

        void unlock(bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t last_operation_error()
        {
            return (_last_error);
        }

        static const char *last_operation_error_str()
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

template <typename... Types>
thread_local int32_t ft_tuple<Types...>::_last_error = FT_ERR_SUCCESS;

#endif
