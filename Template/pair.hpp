#ifndef PAIR_HPP
# define PAIR_HPP

#include "move.hpp"

template <typename KeyType, typename ValueType>
class Pair
{
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
};

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair()
        : key(), value()
{
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const KeyType &input_key, const ValueType &input_value)
        : key(input_key), value(input_value)
{
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const KeyType &input_key, ValueType &&input_value)
        : key(input_key), value(ft_move(input_value))
{
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const Pair &other)
        : key(other.key), value(other.value)
{
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(Pair &&other)
        : key(ft_move(other.key)), value(ft_move(other.value))
{
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::~Pair()
{
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> &Pair<KeyType, ValueType>::operator=(const Pair &other)
{
    if (this != &other)
    {
        this->key = other.key;
        this->value = other.value;
    }
    return (*this);
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> &Pair<KeyType, ValueType>::operator=(Pair &&other)
{
    if (this != &other)
    {
        this->key = ft_move(other.key);
        this->value = ft_move(other.value);
    }
    return (*this);
}

template <typename KeyType, typename ValueType>
KeyType Pair<KeyType, ValueType>::get_key() const
{
    return (this->key);
}

template <typename KeyType, typename ValueType>
ValueType Pair<KeyType, ValueType>::get_value() const
{
    return (this->value);
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_key(const KeyType &input_key)
{
    this->key = input_key;
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_key(KeyType &&input_key)
{
    this->key = ft_move(input_key);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_value(const ValueType &input_value)
{
    this->value = input_value;
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_value(ValueType &&input_value)
{
    this->value = ft_move(input_value);
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
struct ft_remove_reference
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_reference<Type &>
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_reference<Type &&>
{
    typedef Type type;
};

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
struct ft_is_array<Type[]>
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
struct ft_remove_extent<Type[]>
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

template <typename Type>
constexpr Type &&ft_forward(typename ft_remove_reference<Type>::type &value) noexcept
{
    return (static_cast<Type &&>(value));
}

template <typename Type>
constexpr Type &&ft_forward(typename ft_remove_reference<Type>::type &&value) noexcept
{
    return (static_cast<Type &&>(value));
}

template <typename KeyType, typename ValueType>
Pair<typename ft_decay<KeyType>::type, typename ft_decay<ValueType>::type>
ft_make_pair(KeyType &&key, ValueType &&value)
{
    typedef typename ft_decay<KeyType>::type key_type;
    typedef typename ft_decay<ValueType>::type value_type;

    return (Pair<key_type, value_type>(
        ft_forward<KeyType>(key),
        ft_forward<ValueType>(value)));
}

#endif
