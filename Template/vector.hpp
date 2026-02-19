#ifndef FT_VECTOR_HPP
#define FT_VECTOR_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "constructor.hpp"
#include "move.hpp"
#include <cstddef>
#include <type_traits>
#include <cstdint>
#include <new>
#include "../PThread/recursive_mutex.hpp"
#include "../Basic/basic.hpp"

template <typename Type, typename = void>
struct ft_is_complete
    : std::false_type
{
};

template <typename Type>
struct ft_is_complete<Type, std::void_t<decltype(sizeof(Type))> >
    : std::true_type
{
};

template <typename Type, size_t InlineCapacity, bool Enabled>
struct ft_vector_inline_storage
{
};

template <typename Type, size_t InlineCapacity>
struct ft_vector_inline_storage<Type, InlineCapacity, true>
{
    alignas(Type) unsigned char buffer[sizeof(Type) * InlineCapacity];
};
template <typename ElementType>
class ft_vector
{
    private:
        static const size_t SMALL_CAPACITY = 8;
        static const bool SMALL_BUFFER_AVAILABLE = ft_is_complete<ElementType>::value;

        ft_vector_inline_storage<ElementType, SMALL_CAPACITY,
                SMALL_BUFFER_AVAILABLE> _inline_storage;
        ElementType    *_data;
        ft_size_t       _size;
        ft_size_t       _capacity;
        mutable pt_recursive_mutex *_mutex;
        static thread_local int32_t _last_error;

        void    destroy_elements_unlocked(ft_size_t from, ft_size_t to);
        int32_t reserve_internal_unlocked(ft_size_t new_capacity);
        ElementType    *small_data();
        const ElementType  *small_data() const;
        bool    using_small_buffer() const;
        void    reset_to_small_buffer();
	int32_t     lock_internal(bool *lock_acquired) const;
	int32_t     unlock_internal(bool lock_acquired) const;
        static int32_t set_last_operation_error(int32_t error_code) noexcept;

    protected:
        ElementType release_at(ft_size_t index);

    public:
        using iterator = ElementType*;
        using const_iterator = const ElementType*;

        ft_vector(ft_size_t initial_capacity = 0);
        ~ft_vector();

        ft_vector(const ft_vector&) = delete;
        ft_vector& operator=(const ft_vector&) = delete;

	    ft_vector(ft_vector&& other) = delete;
	    ft_vector& operator=(ft_vector&& other) = delete;

	    int32_t     initialize() noexcept;
	    int32_t     initialize(const ft_vector<ElementType> &other);
	    int32_t     initialize(ft_vector<ElementType> &&other);
	    int32_t     destroy() noexcept;

	    int32_t     enable_thread_safety();
	    int32_t     disable_thread_safety();
	    int32_t     copy_from(const ft_vector<ElementType> &other);
	    bool    is_thread_safe() const;
	    int32_t     lock(bool *lock_acquired) const;
	    void    unlock(bool lock_acquired) const;
        static int32_t last_operation_error() noexcept;
        static const char *last_operation_error_str() noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif

        ft_size_t size() const;
        ft_size_t capacity() const;
        bool empty() const;

        void push_back(const ElementType &value);
        void push_back(ElementType &&value);
        void pop_back();

        ElementType& operator[](ft_size_t index);
        const ElementType& operator[](ft_size_t index) const;

        void clear();
        void reserve(ft_size_t new_capacity);
        void resize(ft_size_t new_size, const ElementType& value = ElementType());

        iterator insert(iterator pos, const ElementType& value);
        iterator erase(iterator pos);
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
};

template <typename ElementType>
thread_local int32_t ft_vector<ElementType>::_last_error = FT_ERR_SUCCESS;

template <typename ElementType>
int32_t ft_vector<ElementType>::set_last_operation_error(int32_t error_code) noexcept
{
    ft_vector<ElementType>::_last_error = error_code;
    return (error_code);
}

template <typename ElementType>
int32_t ft_vector<ElementType>::last_operation_error() noexcept
{
    return (ft_vector<ElementType>::_last_error);
}

template <typename ElementType>
const char *ft_vector<ElementType>::last_operation_error_str() noexcept
{
    return (ft_strerror(ft_vector<ElementType>::last_operation_error()));
}

template <typename ElementType>
ft_vector<ElementType>::ft_vector(ft_size_t initial_capacity)
	: _data(ft_nullptr),
	  _size(0),
	  _capacity(0),
	  _mutex(ft_nullptr)
{
    this->reset_to_small_buffer();
    if (initial_capacity > SMALL_CAPACITY)
    {
        int32_t reserve_error;

        reserve_error = this->reserve_internal_unlocked(initial_capacity);
        if (reserve_error != FT_ERR_SUCCESS)
        {
            return ;
        }
    }
    return ;
}

template <typename ElementType>
ft_vector<ElementType>::~ft_vector()
{
    (void)this->disable_thread_safety();
    (void)this->destroy();
    return ;
}

template <typename ElementType>
int32_t ft_vector<ElementType>::initialize() noexcept
{
    this->reset_to_small_buffer();
    this->_size = 0;
    return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}

template <typename ElementType>
int32_t ft_vector<ElementType>::destroy() noexcept
{
    bool lock_acquired = false;
    int32_t lock_error = this->lock_internal(&lock_acquired);
    if (lock_error == FT_ERR_SUCCESS)
    {
        bool small_buffer_in_use = this->using_small_buffer();
        this->destroy_elements_unlocked(0, this->_size);
        if (this->_data != ft_nullptr && small_buffer_in_use == false)
            cma_free(this->_data);
        this->reset_to_small_buffer();
        this->_size = 0;
        this->unlock_internal(lock_acquired);
    }
    return (ft_vector<ElementType>::set_last_operation_error(lock_error));
}

template <typename ElementType>
int32_t ft_vector<ElementType>::initialize(const ft_vector<ElementType> &other)
{
    int32_t initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (ft_vector<ElementType>::set_last_operation_error(initialization_error));
    return (ft_vector<ElementType>::set_last_operation_error(this->copy_from(other)));
}

template <typename ElementType>
int32_t ft_vector<ElementType>::initialize(ft_vector<ElementType> &&other)
{
    if (this == &other)
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    int32_t initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (ft_vector<ElementType>::set_last_operation_error(initialization_error));
    bool other_lock_acquired = false;
    int32_t other_lock_error = other.lock_internal(&other_lock_acquired);
    if (other_lock_error != FT_ERR_SUCCESS)
        return (ft_vector<ElementType>::set_last_operation_error(other_lock_error));
    if (other.using_small_buffer())
    {
        ft_size_t index = 0;
        while (index < other._size)
        {
            construct_at(&this->_data[index], other._data[index]);
            ++index;
        }
        this->_size = other._size;
    }
    else
    {
        if (this->_data != ft_nullptr && this->using_small_buffer() == false)
            cma_free(this->_data);
        this->_data = other._data;
        this->_size = other._size;
        this->_capacity = other._capacity;
        other._data = ft_nullptr;
        other._size = 0;
        other.reset_to_small_buffer();
    }
    other.unlock_internal(other_lock_acquired);
    pt_recursive_mutex *moved_mutex = other._mutex;
    other._mutex = ft_nullptr;
    this->_mutex = moved_mutex;
    return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}

template <typename ElementType>
int32_t ft_vector<ElementType>::copy_from(const ft_vector<ElementType> &other)
{
    if (this == &other)
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    bool self_lock_acquired = false;
    int32_t self_lock_error = this->lock_internal(&self_lock_acquired);
    if (self_lock_error != FT_ERR_SUCCESS)
        return (ft_vector<ElementType>::set_last_operation_error(self_lock_error));
    bool other_lock_acquired = false;
    int32_t other_lock_error = other.lock_internal(&other_lock_acquired);
    if (other_lock_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(self_lock_acquired);
        return (ft_vector<ElementType>::set_last_operation_error(other_lock_error));
    }
    this->destroy_elements_unlocked(0, this->_size);
    if (this->_data != ft_nullptr && this->using_small_buffer() == false)
        cma_free(this->_data);
    this->reset_to_small_buffer();
    this->_size = 0;
    int32_t reserve_error = this->reserve_internal_unlocked(other._capacity);
    if (reserve_error != FT_ERR_SUCCESS)
    {
        other.unlock_internal(other_lock_acquired);
        this->unlock_internal(self_lock_acquired);
        return (ft_vector<ElementType>::set_last_operation_error(reserve_error));
    }
    size_t index = 0;
    while (index < other._size)
    {
        construct_at(&this->_data[index], other._data[index]);
        ++index;
    }
    this->_size = other._size;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(self_lock_acquired);
    return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}


template <typename ElementType>
int32_t ft_vector<ElementType>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    }
    pt_recursive_mutex *mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_NO_MEMORY));
    int32_t initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (ft_vector<ElementType>::set_last_operation_error(initialize_error));
    }
    this->_mutex = mutex_pointer;
    return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}

template <typename ElementType>
int32_t ft_vector<ElementType>::disable_thread_safety()
{
    if (this->_mutex == ft_nullptr)
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    int32_t destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (ft_vector<ElementType>::set_last_operation_error(destroy_error));
}

template <typename ElementType>
bool ft_vector<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_mutex != ft_nullptr);
    return (enabled);
}

template <typename ElementType>
int32_t ft_vector<ElementType>::lock(bool *lock_acquired) const
{
    int32_t result;
    int32_t lock_error;

    lock_error = this->lock_internal(lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_vector<ElementType>::set_last_operation_error(lock_error);
        return (-1);
    }
    result = 0;
    ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (result);
}

template <typename ElementType>
void ft_vector<ElementType>::unlock(bool lock_acquired) const
{
    int32_t unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_vector<ElementType>::set_last_operation_error(unlock_error);
    return ;
}

template <typename ElementType>
ft_size_t ft_vector<ElementType>::size() const
{
    bool   lock_acquired;
    ft_size_t current_size;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0);
    }
    current_size = this->_size;
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
ft_size_t ft_vector<ElementType>::capacity() const
{
    bool   lock_acquired;
    ft_size_t current_capacity;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0);
    }
    current_capacity = this->_capacity;
    this->unlock_internal(lock_acquired);
    return (current_capacity);
}

template <typename ElementType>
bool ft_vector<ElementType>::empty() const
{
    bool lock_acquired;
    bool is_empty;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (true);
    }
    is_empty = (this->_size == 0);
    this->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename ElementType>
void ft_vector<ElementType>::push_back(const ElementType &value)
{
    bool   lock_acquired;
    ft_size_t new_capacity;
    int32_t lock_error;
    int32_t reserve_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (this->_size >= this->_capacity)
    {
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        reserve_error = this->reserve_internal_unlocked(new_capacity);
        if (reserve_error != FT_ERR_SUCCESS)
        {
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], value);
    this->_size += 1;
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::push_back(ElementType &&value)
{
    bool   lock_acquired;
    ft_size_t new_capacity;
    int32_t lock_error;
    int32_t reserve_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (this->_size >= this->_capacity)
    {
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        reserve_error = this->reserve_internal_unlocked(new_capacity);
        if (reserve_error != FT_ERR_SUCCESS)
        {
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], ft_move(value));
    this->_size += 1;
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::pop_back()
{
    bool lock_acquired;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (this->_size > 0)
    {
        ::destroy_at(&this->_data[this->_size - 1]);
        this->_size -= 1;
        }
    else
    {
    }
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType& ft_vector<ElementType>::operator[](ft_size_t index)
{
    static ElementType default_instance = ElementType();
    bool               lock_acquired;
    ElementType       *ref_pointer;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_vector<ElementType>::set_last_operation_error(lock_error);
        return (default_instance);
    }
    if (index >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_vector<ElementType>::set_last_operation_error(FT_ERR_OUT_OF_RANGE);
        return (default_instance);
    }
    ref_pointer = &this->_data[index];
    this->unlock_internal(lock_acquired);
    ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (*ref_pointer);
}

template <typename ElementType>
const ElementType& ft_vector<ElementType>::operator[](ft_size_t index) const
{
    static ElementType default_instance = ElementType();
    bool               lock_acquired;
    const ElementType *ref_pointer;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_vector<ElementType>::set_last_operation_error(lock_error);
        return (default_instance);
    }
    if (index >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_vector<ElementType>::set_last_operation_error(FT_ERR_OUT_OF_RANGE);
        return (default_instance);
    }
    ref_pointer = &this->_data[index];
    this->unlock_internal(lock_acquired);
    ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (*ref_pointer);
}

template <typename ElementType>
void ft_vector<ElementType>::clear()
{
    bool lock_acquired;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->destroy_elements_unlocked(0, this->_size);
    this->_size = 0;
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::reserve(ft_size_t new_capacity)
{
    bool lock_acquired;
    int32_t lock_error;
    int32_t reserve_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    reserve_error = this->reserve_internal_unlocked(new_capacity);
    if (reserve_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::resize(ft_size_t new_size, const ElementType& value)
{
    bool   lock_acquired;
    ft_size_t index;
    int32_t lock_error;
    int32_t reserve_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (new_size < this->_size)
    {
        this->destroy_elements_unlocked(new_size, this->_size);
    }
    else if (new_size > this->_size)
    {
        reserve_error = this->reserve_internal_unlocked(new_size);
        if (reserve_error != FT_ERR_SUCCESS)
        {
            this->unlock_internal(lock_acquired);
            return ;
        }
        index = this->_size;
        while (index < new_size)
        {
            construct_at(&this->_data[index], value);
            ++index;
        }
    }
    this->_size = new_size;
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::insert(iterator pos, const ElementType& value)
{
    bool       lock_acquired;
    ft_size_t index;
    ft_size_t new_capacity;
    ft_size_t move_index;
    iterator   end_iterator;
    iterator   result_iterator;
    int32_t lock_error;
    int32_t reserve_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (this->_data + this->_size);
    }
    index = pos - this->_data;
    if (index > this->_size)
    {
        end_iterator = this->_data + this->_size;
        this->unlock_internal(lock_acquired);
        return (end_iterator);
    }
    if (this->_size >= this->_capacity)
    {
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        reserve_error = this->reserve_internal_unlocked(new_capacity);
        if (reserve_error != FT_ERR_SUCCESS)
        {
            end_iterator = this->_data + this->_size;
            this->unlock_internal(lock_acquired);
            return (end_iterator);
        }
        pos = this->_data + index;
    }
    move_index = this->_size;
    while (move_index > index)
    {
        construct_at(&this->_data[move_index], this->_data[move_index - 1]);
        ::destroy_at(&this->_data[move_index - 1]);
        move_index -= 1;
    }
    construct_at(&this->_data[index], value);
    this->_size += 1;
    result_iterator = &this->_data[index];
    this->unlock_internal(lock_acquired);
    return (result_iterator);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::erase(iterator pos)
{
    bool       lock_acquired;
    ft_size_t index;
    ft_size_t shift_index;
    iterator   end_iterator;
    iterator   result_iterator;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (this->_data + this->_size);
    }
    index = pos - this->_data;
    if (index >= this->_size)
    {
        end_iterator = this->_data + this->_size;
        this->unlock_internal(lock_acquired);
        return (end_iterator);
    }
    ::destroy_at(&this->_data[index]);
    shift_index = index;
    while (shift_index < this->_size - 1)
    {
        construct_at(&this->_data[shift_index], ft_move(this->_data[shift_index + 1]));
        ::destroy_at(&this->_data[shift_index + 1]);
        shift_index += 1;
    }
    this->_size -= 1;
    if (index >= this->_size)
        result_iterator = this->_data + this->_size;
    else
        result_iterator = &this->_data[index];
    this->unlock_internal(lock_acquired);
    return (result_iterator);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::begin()
{
    bool     lock_acquired;
    iterator iterator_result;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (this->_data);
    }
    iterator_result = this->_data;
    this->unlock_internal(lock_acquired);
    return (iterator_result);
}

template <typename ElementType>
typename ft_vector<ElementType>::const_iterator ft_vector<ElementType>::begin() const
{
    bool            lock_acquired;
    const_iterator  iterator_result;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (this->_data);
    }
    iterator_result = this->_data;
    this->unlock_internal(lock_acquired);
    return (iterator_result);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::end()
{
    bool     lock_acquired;
    iterator iterator_result;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (this->_data + this->_size);
    }
    iterator_result = this->_data + this->_size;
    this->unlock_internal(lock_acquired);
    return (iterator_result);
}

template <typename ElementType>
typename ft_vector<ElementType>::const_iterator ft_vector<ElementType>::end() const
{
    bool            lock_acquired;
    const_iterator  iterator_result;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (this->_data + this->_size);
    }
    iterator_result = this->_data + this->_size;
    this->unlock_internal(lock_acquired);
    return (iterator_result);
}

#ifdef LIBFT_TEST_BUILD
template <typename ElementType>
pt_recursive_mutex* ft_vector<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

template <typename ElementType>
void ft_vector<ElementType>::destroy_elements_unlocked(ft_size_t from, ft_size_t to)
{
    ft_size_t index = from;

    while (index < to)
    {
        ::destroy_at(&this->_data[index]);
        ++index;
    }
    return ;
}

template <typename ElementType>
ElementType* ft_vector<ElementType>::small_data()
{
    if constexpr (SMALL_BUFFER_AVAILABLE != false)
        return (reinterpret_cast<ElementType*>(this->_inline_storage.buffer));
    return (ft_nullptr);
}

template <typename ElementType>
const ElementType* ft_vector<ElementType>::small_data() const
{
    if constexpr (SMALL_BUFFER_AVAILABLE != false)
        return (reinterpret_cast<const ElementType*>(this->_inline_storage.buffer));
    return (ft_nullptr);
}

template <typename ElementType>
bool ft_vector<ElementType>::using_small_buffer() const
{
    if constexpr (SMALL_BUFFER_AVAILABLE != false)
    {
        const ElementType* small_pointer = this->small_data();
        return (this->_data == small_pointer);
    }
    return (false);
}

template <typename ElementType>
void ft_vector<ElementType>::reset_to_small_buffer()
{
    if constexpr (SMALL_BUFFER_AVAILABLE != false)
    {
        this->_data = this->small_data();
        this->_capacity = SMALL_CAPACITY;
    }
    else
    {
        this->_data = ft_nullptr;
        this->_capacity = 0;
    }
    return ;
}

template <typename ElementType>
int32_t ft_vector<ElementType>::reserve_internal_unlocked(ft_size_t new_capacity)
{
    if (new_capacity <= this->_capacity)
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(new_capacity * sizeof(ElementType)));
    if (new_data == ft_nullptr)
    {
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_NO_MEMORY));
    }
    bool had_small_buffer = this->using_small_buffer();
    ElementType* old_data = this->_data;
    ft_size_t current_size = this->_size;
    ft_size_t index = 0;
    try
    {
        while (index < current_size)
        {
            construct_at(&new_data[index], ft_move(old_data[index]));
            ++index;
        }
    }
    catch (...)
    {
        while (index > 0)
        {
            --index;
            ::destroy_at(&new_data[index]);
        }
        cma_free(new_data);
        throw;
    }
    ft_size_t destroy_index = 0;
    while (destroy_index < current_size)
    {
        ::destroy_at(&old_data[destroy_index]);
        ++destroy_index;
    }
    if (had_small_buffer == false && old_data != ft_nullptr)
        cma_free(old_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}

template <typename ElementType>
int32_t ft_vector<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    }
    int32_t result = this->_mutex->lock();
    if (result != FT_ERR_SUCCESS)
        return (ft_vector<ElementType>::set_last_operation_error(result));
    if (lock_acquired)
        *lock_acquired = true;
    return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}

template <typename ElementType>
int32_t ft_vector<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (ft_vector<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    return (ft_vector<ElementType>::set_last_operation_error(this->_mutex->unlock()));
}


template <typename ElementType>
ElementType ft_vector<ElementType>::release_at(ft_size_t index)
{
    bool lock_acquired;
    ElementType detached;
    ft_size_t   shift_index;
    int32_t lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (ElementType());
    }
    if (index >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    detached = ft_move(this->_data[index]);
    shift_index = index;
    while (shift_index < this->_size - 1)
    {
        this->_data[shift_index] = ft_move(this->_data[shift_index + 1]);
        shift_index += 1;
    }
    this->_size -= 1;
    this->unlock_internal(lock_acquired);
    return (detached);
}

#endif
