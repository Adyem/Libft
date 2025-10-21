#ifndef PAIR_HPP
# define PAIR_HPP

#include <utility>
#include "../Errno/errno.hpp"
#include "move.hpp"

template <typename KeyType, typename ValueType>
class Pair
{
    private:
        mutable int                 _error_code;

        void set_error(int error) const;

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
        : _error_code(ER_SUCCESS), key(), value()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const KeyType &input_key, const ValueType &input_value)
        : _error_code(ER_SUCCESS), key(input_key), value(input_value)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const KeyType &input_key, ValueType &&input_value)
        : _error_code(ER_SUCCESS), key(input_key), value(ft_move(input_value))
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const Pair &other)
        : _error_code(ER_SUCCESS), key(), value()
{
    this->key = other.key;
    this->value = other.value;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(Pair &&other)
        : _error_code(ER_SUCCESS), key(), value()
{
    this->key = ft_move(other.key);
    this->value = ft_move(other.value);
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::~Pair()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> &Pair<KeyType, ValueType>::operator=(const Pair &other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->key = other.key;
    this->value = other.value;
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> &Pair<KeyType, ValueType>::operator=(Pair &&other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->key = ft_move(other.key);
    this->value = ft_move(other.value);
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename KeyType, typename ValueType>
KeyType Pair<KeyType, ValueType>::get_key() const
{
    KeyType key_copy(this->key);
    this->set_error(ER_SUCCESS);
    return (key_copy);
}

template <typename KeyType, typename ValueType>
ValueType Pair<KeyType, ValueType>::get_value() const
{
    ValueType value_copy(this->value);
    this->set_error(ER_SUCCESS);
    return (value_copy);
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_key(const KeyType &input_key)
{
    this->key = input_key;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_key(KeyType &&input_key)
{
    this->key = ft_move(input_key);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_value(const ValueType &input_value)
{
    this->value = input_value;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_value(ValueType &&input_value)
{
    this->value = ft_move(input_value);
    this->set_error(ER_SUCCESS);
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
