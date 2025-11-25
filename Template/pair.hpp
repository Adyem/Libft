#ifndef PAIR_HPP
# define PAIR_HPP

#include <utility>
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "move.hpp"

template <typename KeyType, typename ValueType>
class Pair
{
    private:
        mutable int                 _error_code;
        mutable pt_mutex*           _mutex;
        bool                        _thread_safe_enabled;

        void set_error(int error) const;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

    public:
        KeyType key;
        ValueType value;
        Pair();
        Pair(const KeyType &input_key, const ValueType &input_value);
        Pair(const KeyType &input_key, ValueType &&input_value);
        Pair(const Pair &other);
        Pair(Pair &&other);
        ~Pair();

        Pair &operator=(const Pair &other);
        Pair &operator=(Pair &&other);

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

        KeyType get_key() const;
        ValueType get_value() const;
        void set_key(const KeyType &input_key);
        void set_key(KeyType &&input_key);
        void set_value(const ValueType &input_value);
        void set_value(ValueType &&input_value);
        int get_error() const;
        const char *get_error_str() const;
};

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair()
        : _error_code(ER_SUCCESS), _mutex(ft_nullptr),
        _thread_safe_enabled(false), key(), value()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const KeyType &input_key, const ValueType &input_value)
        : _error_code(ER_SUCCESS), _mutex(ft_nullptr),
        _thread_safe_enabled(false), key(input_key), value(input_value)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const KeyType &input_key, ValueType &&input_value)
        : _error_code(ER_SUCCESS), _mutex(ft_nullptr),
        _thread_safe_enabled(false), key(input_key), value(ft_move(input_value))
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const Pair &other)
        : _error_code(ER_SUCCESS), _mutex(ft_nullptr),
        _thread_safe_enabled(false), key(), value()
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->key = other.key;
    this->value = other.value;
    other.unlock_internal(other_lock_acquired);
    if (other._thread_safe_enabled && other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(Pair &&other)
        : _error_code(ER_SUCCESS), _mutex(ft_nullptr),
        _thread_safe_enabled(false), key(), value()
{
    bool other_lock_acquired;
    pt_mutex *other_mutex;
    bool other_thread_safe;

    other_lock_acquired = false;
    other_mutex = ft_nullptr;
    other_thread_safe = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->key = ft_move(other.key);
    this->value = ft_move(other.value);
    other_thread_safe = other._thread_safe_enabled;
    other_mutex = other._mutex;
    other.unlock_internal(other_lock_acquired);
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
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
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::~Pair()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> &Pair<KeyType, ValueType>::operator=(const Pair &other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;

    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
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
    this->key = other.key;
    this->value = other.value;
    this->unlock_internal(this_lock_acquired);
    other.unlock_internal(other_lock_acquired);
    if (other._thread_safe_enabled && other._mutex != ft_nullptr)
    {
        if (!this->_thread_safe_enabled)
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
    }
    else if (this->_thread_safe_enabled)
        this->disable_thread_safety();
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> &Pair<KeyType, ValueType>::operator=(Pair &&other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    pt_mutex *other_mutex;
    bool other_thread_safe;

    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
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
    other_mutex = ft_nullptr;
    other_thread_safe = false;
    this->key = ft_move(other.key);
    this->value = ft_move(other.value);
    this->unlock_internal(this_lock_acquired);
    other.unlock_internal(other_lock_acquired);
    this->teardown_thread_safety();
    other_thread_safe = other._thread_safe_enabled;
    other_mutex = other._mutex;
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
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
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename KeyType, typename ValueType>
int Pair<KeyType, ValueType>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
bool Pair<KeyType, ValueType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<Pair<KeyType, ValueType> *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

template <typename KeyType, typename ValueType>
int Pair<KeyType, ValueType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<Pair<KeyType, ValueType> *>(this)->set_error(ft_errno);
    else
        const_cast<Pair<KeyType, ValueType> *>(this)->set_error(ER_SUCCESS);
    return (result);
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != ER_SUCCESS)
        const_cast<Pair<KeyType, ValueType> *>(this)->set_error(this->_mutex->get_error());
    else
        const_cast<Pair<KeyType, ValueType> *>(this)->set_error(entry_errno);
    return ;
}

template <typename KeyType, typename ValueType>
KeyType Pair<KeyType, ValueType>::get_key() const
{
    KeyType key_copy;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<Pair<KeyType, ValueType> *>(this)->set_error(ft_errno);
        return (KeyType());
    }
    key_copy = this->key;
    this->unlock_internal(lock_acquired);
    const_cast<Pair<KeyType, ValueType> *>(this)->set_error(ER_SUCCESS);
    return (key_copy);
}

template <typename KeyType, typename ValueType>
ValueType Pair<KeyType, ValueType>::get_value() const
{
    ValueType value_copy;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<Pair<KeyType, ValueType> *>(this)->set_error(ft_errno);
        return (ValueType());
    }
    value_copy = this->value;
    this->unlock_internal(lock_acquired);
    const_cast<Pair<KeyType, ValueType> *>(this)->set_error(ER_SUCCESS);
    return (value_copy);
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_key(const KeyType &input_key)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->key = input_key;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_key(KeyType &&input_key)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->key = ft_move(input_key);
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_value(const ValueType &input_value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->value = input_value;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_value(ValueType &&input_value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->value = ft_move(input_value);
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename KeyType, typename ValueType>
int Pair<KeyType, ValueType>::get_error() const
{
    return (this->_error_code);
}

template <typename KeyType, typename ValueType>
const char *Pair<KeyType, ValueType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename KeyType, typename ValueType>
int Pair<KeyType, ValueType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

template <typename Type, Type Value>
struct ft_integral_constant
{
    static const Type value = Value;
    typedef Type value_type;
    typedef ft_integral_constant type;
};

typedef ft_integral_constant<bool, true> ft_true_type;
typedef ft_integral_constant<bool, false> ft_false_type;

template <typename Type>
struct ft_remove_const
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_const<const Type>
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_volatile
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_volatile<volatile Type>
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_cv
{
    typedef typename ft_remove_const<typename ft_remove_volatile<Type>::type>::type type;
};

template <typename Type>
struct ft_is_array
        : ft_false_type
{
};

template <typename Type>
struct ft_is_array<Type []>
        : ft_true_type
{
};

template <typename Type, unsigned long Size>
struct ft_is_array<Type[Size]>
        : ft_true_type
{
};

template <typename Type>
struct ft_remove_extent
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_extent<Type []>
{
    typedef Type type;
};

template <typename Type, unsigned long Size>
struct ft_remove_extent<Type[Size]>
{
    typedef Type type;
};

template <typename Type>
struct ft_add_pointer
{
    typedef typename ft_remove_reference<Type>::type *type;
};

template <typename Type>
struct ft_is_function
        : ft_false_type
{
};

template <typename ReturnType, typename... Arguments>
struct ft_is_function<ReturnType(Arguments...)>
        : ft_true_type
{
};

template <typename ReturnType, typename... Arguments>
struct ft_is_function<ReturnType(Arguments..., ...)>
        : ft_true_type
{
};

template <typename ReturnType, typename... Arguments>
struct ft_is_function<ReturnType(Arguments...) noexcept>
        : ft_true_type
{
};

template <typename ReturnType, typename... Arguments>
struct ft_is_function<ReturnType(Arguments..., ...) noexcept>
        : ft_true_type
{
};

template <typename Type, bool IsArray, bool IsFunction>
struct ft_decay_selector
{
    typedef typename ft_remove_cv<Type>::type type;
};

template <typename Type>
struct ft_decay_selector<Type, true, false>
{
    typedef typename ft_remove_extent<Type>::type *type;
};

template <typename Type>
struct ft_decay_selector<Type, false, true>
{
    typedef typename ft_add_pointer<Type>::type type;
};

template <typename Type>
struct ft_decay
{
    typedef typename ft_decay_selector<
        typename ft_remove_reference<Type>::type,
        ft_is_array<typename ft_remove_reference<Type>::type>::value,
        ft_is_function<typename ft_remove_reference<Type>::type>::value
    >::type type;
};

template <typename KeyType, typename ValueType>
Pair<typename ft_decay<KeyType>::type, typename ft_decay<ValueType>::type>
ft_make_pair(KeyType &&key, ValueType &&value)
{
    typedef typename ft_decay<KeyType>::type key_type;
    typedef typename ft_decay<ValueType>::type value_type;

    return (Pair<key_type, value_type>(
        std::forward<KeyType>(key),
        std::forward<ValueType>(value)));
}

#endif
