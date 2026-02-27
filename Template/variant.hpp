#ifndef FT_VARIANT_HPP
#define FT_VARIANT_HPP

#include "constructor.hpp"
#include "move.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <type_traits>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <new>

template <typename TypeToFind, typename... Types>
struct variant_index;

template <typename TypeToFind, typename... Types>
struct variant_index<TypeToFind, TypeToFind, Types...>
    : std::integral_constant<size_t, 0>
{
};

template <typename TypeToFind, typename CurrentType, typename... Types>
struct variant_index<TypeToFind, CurrentType, Types...>
    : std::integral_constant<size_t, 1 + variant_index<TypeToFind, Types...>::value>
{
};

template <size_t IndexValue, typename... Types>
struct variant_destroyer;

template <size_t IndexValue>
struct variant_destroyer<IndexValue>
{
    static void destroy(size_t, void*)
    {
        return ;
    }
};

template <size_t IndexValue, typename CurrentType, typename... Types>
struct variant_destroyer<IndexValue, CurrentType, Types...>
{
    static void destroy(size_t stored_index, void* storage_pointer)
    {
        if (stored_index == IndexValue)
            ::destroy_at(reinterpret_cast<CurrentType*>(storage_pointer));
        else
            variant_destroyer<IndexValue + 1, Types...>::destroy(stored_index,
                storage_pointer);
        return ;
    }
};

template <size_t IndexValue, typename... Types>
struct variant_visitor;

template <size_t IndexValue>
struct variant_visitor<IndexValue>
{
    template <typename VisitorType>
    static void apply(size_t, void*, VisitorType&&)
    {
        return ;
    }
};

template <size_t IndexValue, typename CurrentType, typename... Types>
struct variant_visitor<IndexValue, CurrentType, Types...>
{
    template <typename VisitorType>
    static void apply(size_t stored_index, void* storage_pointer,
        VisitorType&& visitor)
    {
        if (stored_index == IndexValue)
            visitor(*reinterpret_cast<CurrentType*>(storage_pointer));
        else
            variant_visitor<IndexValue + 1, Types...>::apply(stored_index,
                storage_pointer, std::forward<VisitorType>(visitor));
        return ;
    }
};

template <typename... Types>
class ft_variant
{
    private:
        using storage_type = typename std::aligned_union<0, Types...>::type;

        storage_type              *_data;
        size_t                     _index;
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
            pf_printf_fd(2, "ft_variant lifecycle error: %s: %s\n", method_name,
                reason);
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
            lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
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
            unlock_result = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            if (unlock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_result));
            return (FT_ERR_SUCCESS);
        }

        void destroy_unlocked()
        {
            if (this->_data == ft_nullptr)
                return ;
            if (this->_index == npos)
                return ;
            variant_destroyer<0, Types...>::destroy(this->_index, this->_data);
            this->_index = npos;
            return ;
        }

    public:
        static const size_t npos = static_cast<size_t>(-1);

        ft_variant()
            : _data(ft_nullptr), _index(npos), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename TypeToStore>
        explicit ft_variant(const TypeToStore& value)
            : _data(ft_nullptr), _index(npos), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(value);
            return ;
        }

        template <typename TypeToStore>
        explicit ft_variant(TypeToStore&& value)
            : _data(ft_nullptr), _index(npos), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(ft_move(value));
            return ;
        }

        ~ft_variant()
        {
            if (this->_initialized_state == _state_initialized)
            {
                if (this->_mutex != ft_nullptr)
                    (void)this->disable_thread_safety();
                (void)this->destroy();
            }
            return ;
        }

        ft_variant(const ft_variant &other) = delete;
        ft_variant(ft_variant &&other) = delete;
        ft_variant &operator=(const ft_variant &other) = delete;
        ft_variant &operator=(ft_variant &&other) = delete;

        int initialize()
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_variant::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_data = static_cast<storage_type*>(cma_malloc(sizeof(storage_type)));
            if (this->_data == ft_nullptr)
            {
                this->_initialized_state = _state_destroyed;
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            }
            this->_index = npos;
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        template <typename TypeToStore>
        int initialize(TypeToStore&& value)
        {
            int initialize_error;

            initialize_error = this->initialize();
            if (initialize_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(initialize_error));
            this->emplace<TypeToStore>(std::forward<TypeToStore>(value));
            if (last_operation_error() != FT_ERR_SUCCESS)
            {
                (void)this->destroy();
                this->_initialized_state = _state_destroyed;
                return (set_last_operation_error(last_operation_error()));
            }
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int destroy()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            if (this->_initialized_state != _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            this->destroy_unlocked();
            if (this->_data != ft_nullptr)
            {
                cma_free(this->_data);
                this->_data = ft_nullptr;
            }
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            this->_index = npos;
            this->_initialized_state = _state_destroyed;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        template <typename TypeToStore>
        void emplace(TypeToStore&& value)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_variant::emplace");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (this->_data == ft_nullptr)
            {
                this->_data = static_cast<storage_type*>(cma_malloc(sizeof(storage_type)));
                if (this->_data == ft_nullptr)
                {
                    unlock_error = this->unlock_internal(lock_acquired);
                    if (unlock_error != FT_ERR_SUCCESS)
                        set_last_operation_error(unlock_error);
                    else
                        set_last_operation_error(FT_ERR_NO_MEMORY);
                    return ;
                }
            }
            this->destroy_unlocked();
            construct_at(reinterpret_cast<std::decay_t<TypeToStore>*>(this->_data),
                std::forward<TypeToStore>(value));
            this->_index = variant_index<std::decay_t<TypeToStore>, Types...>::value;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename TypeToCheck>
        bool holds_alternative() const
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            bool has_type;

            this->abort_if_not_initialized("ft_variant::holds_alternative");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (false);
            }
            has_type = (this->_index == variant_index<TypeToCheck, Types...>::value);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (false);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (has_type);
        }

        template <typename TypeToGet>
        class value_proxy
        {
            private:
                TypeToGet *_pointer;
                int32_t    _error;

            public:
                value_proxy(TypeToGet *pointer, int32_t error)
                    : _pointer(pointer), _error(error)
                {
                    return ;
                }

                operator TypeToGet&() const
                {
                    static TypeToGet fallback = TypeToGet();

                    if (this->_pointer == ft_nullptr)
                        return (fallback);
                    return (*this->_pointer);
                }

                TypeToGet *operator->() const
                {
                    return (this->_pointer);
                }

                int32_t get_error() const
                {
                    return (this->_error);
                }
        };

        template <typename TypeToGet>
        value_proxy<TypeToGet> get()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            TypeToGet *value_pointer;

            this->abort_if_not_initialized("ft_variant::get");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (value_proxy<TypeToGet>(ft_nullptr,
                    set_last_operation_error(lock_error)));
            if (this->_index != variant_index<TypeToGet, Types...>::value)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    return (value_proxy<TypeToGet>(ft_nullptr,
                        set_last_operation_error(unlock_error)));
                return (value_proxy<TypeToGet>(ft_nullptr,
                    set_last_operation_error(FT_ERR_INVALID_OPERATION)));
            }
            value_pointer = reinterpret_cast<TypeToGet*>(this->_data);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (value_proxy<TypeToGet>(ft_nullptr,
                    set_last_operation_error(unlock_error)));
            return (value_proxy<TypeToGet>(value_pointer,
                set_last_operation_error(FT_ERR_SUCCESS)));
        }

        template <typename TypeToGet>
        value_proxy<const TypeToGet> get() const
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            const TypeToGet *value_pointer;

            this->abort_if_not_initialized("ft_variant::get const");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (value_proxy<const TypeToGet>(ft_nullptr,
                    set_last_operation_error(lock_error)));
            if (this->_index != variant_index<TypeToGet, Types...>::value)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    return (value_proxy<const TypeToGet>(ft_nullptr,
                        set_last_operation_error(unlock_error)));
                return (value_proxy<const TypeToGet>(ft_nullptr,
                    set_last_operation_error(FT_ERR_INVALID_OPERATION)));
            }
            value_pointer = reinterpret_cast<const TypeToGet*>(this->_data);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (value_proxy<const TypeToGet>(ft_nullptr,
                    set_last_operation_error(unlock_error)));
            return (value_proxy<const TypeToGet>(value_pointer,
                set_last_operation_error(FT_ERR_SUCCESS)));
        }

        template <typename VisitorType>
        void visit(VisitorType&& visitor)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_variant::visit");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (this->_index == npos)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_INVALID_OPERATION);
                return ;
            }
            variant_visitor<0, Types...>::apply(this->_index, this->_data,
                std::forward<VisitorType>(visitor));
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        void reset()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_variant::reset");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            this->destroy_unlocked();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            this->abort_if_not_initialized("ft_variant::enable_thread_safety");
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

            this->abort_if_not_initialized("ft_variant::disable_thread_safety");
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
            this->abort_if_not_initialized("ft_variant::is_thread_safe");
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_result;

            this->abort_if_not_initialized("ft_variant::lock");
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

template <typename... Types>
thread_local int32_t ft_variant<Types...>::_last_error = FT_ERR_SUCCESS;

#endif
