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
    : std::integral_constant<ft_size_t, 0>
{
};

template <typename TypeToFind, typename CurrentType, typename... Types>
struct variant_index<TypeToFind, CurrentType, Types...>
    : std::integral_constant<ft_size_t, 1 + variant_index<TypeToFind, Types...>::value>
{
};

template <ft_size_t IndexValue, typename... Types>
struct variant_destroyer;

template <ft_size_t IndexValue>
struct variant_destroyer<IndexValue>
{
    static void destroy(ft_size_t, void*)
    {
        return ;
    }
};

template <ft_size_t IndexValue, typename CurrentType, typename... Types>
struct variant_destroyer<IndexValue, CurrentType, Types...>
{
    static void destroy(ft_size_t stored_index, void* storage_pointer)
    {
        if (stored_index == IndexValue)
            ::destroy_at(reinterpret_cast<CurrentType*>(storage_pointer));
        else
            variant_destroyer<IndexValue + 1, Types...>::destroy(stored_index,
                storage_pointer);
        return ;
    }
};

template <ft_size_t IndexValue, typename... Types>
struct variant_visitor;

template <ft_size_t IndexValue>
struct variant_visitor<IndexValue>
{
    template <typename VisitorType>
    static void apply(ft_size_t, void*, VisitorType&&)
    {
        return ;
    }
};

template <ft_size_t IndexValue, typename CurrentType, typename... Types>
struct variant_visitor<IndexValue, CurrentType, Types...>
{
    template <typename VisitorType>
    static void apply(ft_size_t stored_index, void* storage_pointer,
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
        ft_size_t                     _index;
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
            pf_printf_fd(2, "ft_variant lifecycle error: %s: %s\n", method_name,
                reason);
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
        static const ft_size_t npos = static_cast<ft_size_t>(-1);

        ft_variant()
            : _data(ft_nullptr), _index(npos), _mutex(ft_nullptr),
              _initialised_state(_state_uninitialised)
        {
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename TypeToStore>
        explicit ft_variant(const TypeToStore& value)
            : _data(ft_nullptr), _index(npos), _mutex(ft_nullptr),
              _initialised_state(_state_uninitialised)
        {
            (void)this->initialize(value);
            return ;
        }

        template <typename TypeToStore>
        explicit ft_variant(TypeToStore&& value)
            : _data(ft_nullptr), _index(npos), _mutex(ft_nullptr),
              _initialised_state(_state_uninitialised)
        {
            (void)this->initialize(ft_move(value));
            return ;
        }

        ~ft_variant()
        {
            if (this->_initialised_state == _state_initialised)
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

        int32_t initialize()
        {
            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("ft_variant::initialize",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_data = static_cast<storage_type*>(cma_malloc(sizeof(storage_type)));
            if (this->_data == ft_nullptr)
            {
                this->_initialised_state = _state_destroyed;
                return (set_error(FT_ERR_NO_MEMORY));
            }
            this->_index = npos;
            this->_initialised_state = _state_initialised;
            return (set_error(FT_ERR_SUCCESS));
        }

        template <typename TypeToStore>
        int32_t initialize(TypeToStore&& value)
        {
            int32_t initialize_error;

            initialize_error = this->initialize();
            if (initialize_error != FT_ERR_SUCCESS)
                return (set_error(initialize_error));
            this->emplace<TypeToStore>(std::forward<TypeToStore>(value));
            if (get_error() != FT_ERR_SUCCESS)
            {
                (void)this->destroy();
                this->_initialised_state = _state_destroyed;
                return (set_error(get_error()));
            }
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
            this->destroy_unlocked();
            if (this->_data != ft_nullptr)
            {
                cma_free(this->_data);
                this->_data = ft_nullptr;
            }
            (void)this->unlock_internal(lock_acquired);
            this->_index = npos;
            this->_initialised_state = _state_destroyed;
            return (set_error(FT_ERR_SUCCESS));
        }

        template <typename TypeToStore>
        void emplace(TypeToStore&& value)
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_variant::emplace");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            if (this->_data == ft_nullptr)
            {
                this->_data = static_cast<storage_type*>(cma_malloc(sizeof(storage_type)));
                if (this->_data == ft_nullptr)
                {
                    (void)this->unlock_internal(lock_acquired);
                    set_error(FT_ERR_NO_MEMORY);
                    return ;
                }
            }
            this->destroy_unlocked();
            construct_at(reinterpret_cast<std::decay_t<TypeToStore>*>(this->_data),
                std::forward<TypeToStore>(value));
            this->_index = variant_index<std::decay_t<TypeToStore>, Types...>::value;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename TypeToCheck>
        bool holds_alternative() const
        {
            ft_bool lock_acquired;
            int32_t lock_error;
            bool has_type;

            this->abort_if_not_initialised("ft_variant::holds_alternative");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (false);
            }
            has_type = (this->_index == variant_index<TypeToCheck, Types...>::value);
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
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
            ft_bool lock_acquired;
            int32_t lock_error;
            TypeToGet *value_pointer;

            this->abort_if_not_initialised("ft_variant::get");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (value_proxy<TypeToGet>(ft_nullptr,
                    set_error(lock_error)));
            if (this->_index != variant_index<TypeToGet, Types...>::value)
            {
                (void)this->unlock_internal(lock_acquired);
                return (value_proxy<TypeToGet>(ft_nullptr,
                    set_error(FT_ERR_INVALID_OPERATION)));
            }
            value_pointer = reinterpret_cast<TypeToGet*>(this->_data);
            (void)this->unlock_internal(lock_acquired);
            return (value_proxy<TypeToGet>(value_pointer,
                set_error(FT_ERR_SUCCESS)));
        }

        template <typename TypeToGet>
        value_proxy<const TypeToGet> get() const
        {
            ft_bool lock_acquired;
            int32_t lock_error;
            const TypeToGet *value_pointer;

            this->abort_if_not_initialised("ft_variant::get const");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (value_proxy<const TypeToGet>(ft_nullptr,
                    set_error(lock_error)));
            if (this->_index != variant_index<TypeToGet, Types...>::value)
            {
                (void)this->unlock_internal(lock_acquired);
                return (value_proxy<const TypeToGet>(ft_nullptr,
                    set_error(FT_ERR_INVALID_OPERATION)));
            }
            value_pointer = reinterpret_cast<const TypeToGet*>(this->_data);
            (void)this->unlock_internal(lock_acquired);
            return (value_proxy<const TypeToGet>(value_pointer,
                set_error(FT_ERR_SUCCESS)));
        }

        template <typename VisitorType>
        void visit(VisitorType&& visitor)
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_variant::visit");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            if (this->_index == npos)
            {
                (void)this->unlock_internal(lock_acquired);
                set_error(FT_ERR_INVALID_OPERATION);
                return ;
            }
            variant_visitor<0, Types...>::apply(this->_index, this->_data,
                std::forward<VisitorType>(visitor));
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        void reset()
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_variant::reset");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            this->destroy_unlocked();
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        int32_t enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int32_t initialize_result;

            this->abort_if_not_initialised("ft_variant::enable_thread_safety");
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

            this->abort_if_not_initialised("ft_variant::disable_thread_safety");
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
            this->abort_if_not_initialised("ft_variant::is_thread_safe");
            set_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int32_t lock(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            this->abort_if_not_initialised("ft_variant::lock");
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

template <typename... Types>
thread_local int32_t ft_variant<Types...>::_last_error = FT_ERR_SUCCESS;

#endif
