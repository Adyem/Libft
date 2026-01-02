#ifndef FT_VECTOR_HPP
#define FT_VECTOR_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "constructor.hpp"
#include <cstddef>
#include <type_traits>
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

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
#include "../Libft/libft.hpp"
#include "move.hpp"

template <typename ElementType>
class ft_vector
{
    private:
        static const size_t SMALL_CAPACITY = 8;
        static const bool SMALL_BUFFER_AVAILABLE = ft_is_complete<ElementType>::value;

        ft_vector_inline_storage<ElementType, SMALL_CAPACITY, SMALL_BUFFER_AVAILABLE> _inline_storage;
        ElementType    *_data;
        size_t          _size;
        size_t          _capacity;
        mutable int     _error_code;
        mutable pt_mutex    *_mutex;
        bool                _thread_safe_enabled;

        void    destroy_elements_unlocked(size_t from, size_t to);
        void    reserve_internal_unlocked(size_t new_capacity);
        ElementType    *small_data();
        const ElementType  *small_data() const;
        bool    using_small_buffer() const;
        void    reset_to_small_buffer();
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

    protected:
        void    set_error(int error_code) const;
        ElementType release_at(size_t index);

    public:
        using iterator = ElementType*;
        using const_iterator = const ElementType*;

        ft_vector(size_t initial_capacity = 0);
        ~ft_vector();

        ft_vector(const ft_vector&) = delete;
        ft_vector& operator=(const ft_vector&) = delete;

        ft_vector(ft_vector&& other) noexcept;
        ft_vector& operator=(ft_vector&& other) noexcept;

        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

        size_t size() const;
        size_t capacity() const;
        bool empty() const;
        int get_error() const;
        const char* get_error_str() const;

        void push_back(const ElementType &value);
        void push_back(ElementType &&value);
        void pop_back();

        ElementType& operator[](size_t index);
        const ElementType& operator[](size_t index) const;

        void clear();
        void reserve(size_t new_capacity);
        void resize(size_t new_size, const ElementType& value = ElementType());

        iterator insert(iterator pos, const ElementType& value);
        iterator erase(iterator pos);
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
};

template <typename ElementType>
ft_vector<ElementType>::ft_vector(size_t initial_capacity)
    : _data(ft_nullptr),
      _size(0),
      _capacity(0),
      _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    void     *memory;
    pt_mutex *mutex_pointer;

    this->reset_to_small_buffer();
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return ;
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    if (initial_capacity > SMALL_CAPACITY)
    {
        this->reserve_internal_unlocked(initial_capacity);
        if (this->_capacity < initial_capacity)
            return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_vector<ElementType>::~ft_vector()
{
    bool lock_acquired;
    bool small_buffer_in_use;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        small_buffer_in_use = this->using_small_buffer();
        this->destroy_elements_unlocked(0, this->_size);
        if (this->_data != ft_nullptr && small_buffer_in_use == false)
            cma_free(this->_data);
        this->_data = ft_nullptr;
        this->_capacity = 0;
        this->_size = 0;
        this->set_error(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
    }
    else
        this->set_error(ft_errno);
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_vector<ElementType>::ft_vector(ft_vector<ElementType>&& other) noexcept
    : _data(ft_nullptr),
      _size(0),
      _capacity(0),
      _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool     other_thread_safe;
    void     *memory;
    pt_mutex *mutex_pointer;

    other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
    if (other_thread_safe)
    {
        memory = cma_malloc(sizeof(pt_mutex));
        if (memory == ft_nullptr)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
        mutex_pointer = new(memory) pt_mutex();
        if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
        {
            int mutex_error;

            mutex_error = mutex_pointer->get_error();
            mutex_pointer->~pt_mutex();
            cma_free(memory);
            this->set_error(mutex_error);
            return ;
        }
        this->_mutex = mutex_pointer;
        this->_thread_safe_enabled = true;
    }
    if (other.using_small_buffer() != false)
    {
        this->reset_to_small_buffer();
        size_t move_index = 0;
        while (move_index < other._size)
        {
            construct_at(&this->_data[move_index], ft_move(other._data[move_index]));
            destroy_at(&other._data[move_index]);
            ++move_index;
        }
        this->_size = other._size;
    }
    else
    {
        this->_data = other._data;
        this->_size = other._size;
        this->_capacity = other._capacity;
    }
    this->_error_code = other._error_code;
    other._size = 0;
    other.reset_to_small_buffer();
    other._error_code = FT_ERR_SUCCESSS;
}

template <typename ElementType>
ft_vector<ElementType>& ft_vector<ElementType>::operator=(ft_vector<ElementType>&& other) noexcept
{
    if (this != &other)
    {
        bool lock_acquired;
        bool other_thread_safe;

        lock_acquired = false;
        if (this->lock_internal(&lock_acquired) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        this->destroy_elements_unlocked(0, this->_size);
        if (this->_data != ft_nullptr && this->using_small_buffer() == false)
            cma_free(this->_data);
        this->_data = ft_nullptr;
        this->_size = 0;
        this->_capacity = 0;
        this->unlock_internal(lock_acquired);
        this->teardown_thread_safety();
        this->_mutex = ft_nullptr;
        this->_thread_safe_enabled = false;
        other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
        if (other.using_small_buffer() != false)
        {
            this->reset_to_small_buffer();
            this->_size = 0;
            size_t move_index = 0;
            while (move_index < other._size)
            {
                construct_at(&this->_data[move_index], ft_move(other._data[move_index]));
                destroy_at(&other._data[move_index]);
                ++move_index;
                this->_size++;
            }
        }
        else
        {
            this->_data = other._data;
            this->_size = other._size;
            this->_capacity = other._capacity;
        }
        this->_error_code = other._error_code;
        if (other_thread_safe && this->_mutex == ft_nullptr)
        {
            void *memory;
            pt_mutex *mutex_pointer;

            memory = cma_malloc(sizeof(pt_mutex));
            if (memory == ft_nullptr)
            {
                this->set_error(FT_ERR_NO_MEMORY);
                return (*this);
            }
            mutex_pointer = new(memory) pt_mutex();
            if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
            {
                int mutex_error;

                mutex_error = mutex_pointer->get_error();
                mutex_pointer->~pt_mutex();
                cma_free(memory);
                this->set_error(mutex_error);
                return (*this);
            }
            this->_mutex = mutex_pointer;
            this->_thread_safe_enabled = true;
        }
        if (other_thread_safe == false && this->_mutex != ft_nullptr)
        {
            this->teardown_thread_safety();
            this->_mutex = ft_nullptr;
            this->_thread_safe_enabled = false;
        }
        other._size = 0;
        other.reset_to_small_buffer();
        other._error_code = FT_ERR_SUCCESSS;
    }
    this->set_error(this->_error_code);
    return (*this);
}

template <typename ElementType>
void ft_vector<ElementType>::destroy_elements_unlocked(size_t from, size_t to)
{
    size_t index = from;
    while (index < to)
    {
        destroy_at(&this->_data[index]);
        ++index;
    }
    return ;
}

template <typename ElementType>
size_t ft_vector<ElementType>::size() const
{
    bool   lock_acquired;
    size_t current_size;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_vector<ElementType> *>(this)->set_error(ft_errno);
        return (0);
    }
    current_size = this->_size;
    const_cast<ft_vector<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
size_t ft_vector<ElementType>::capacity() const
{
    bool   lock_acquired;
    size_t current_capacity;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_vector<ElementType> *>(this)->set_error(ft_errno);
        return (0);
    }
    current_capacity = this->_capacity;
    const_cast<ft_vector<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_capacity);
}

template <typename ElementType>
bool ft_vector<ElementType>::empty() const
{
    bool lock_acquired;
    bool is_empty;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_vector<ElementType> *>(this)->set_error(ft_errno);
        return (true);
    }
    is_empty = (this->_size == 0);
    const_cast<ft_vector<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename ElementType>
void ft_vector<ElementType>::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
bool ft_vector<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_vector<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ElementType>
int ft_vector<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_vector<ElementType> *>(this)->set_error(ft_errno);
    else
        const_cast<ft_vector<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
void ft_vector<ElementType>::unlock(bool lock_acquired) const
{
    ft_errno = FT_ERR_SUCCESSS;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != FT_ERR_SUCCESSS)
        const_cast<ft_vector<ElementType> *>(this)->set_error(this->_mutex->get_error());
    else
        const_cast<ft_vector<ElementType> *>(this)->set_error(ft_errno);
    return ;
}

template <typename ElementType>
int ft_vector<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_vector<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ElementType>
void ft_vector<ElementType>::push_back(const ElementType &value)
{
    bool   lock_acquired;
    size_t new_capacity;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (this->_size >= this->_capacity)
    {
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        this->reserve_internal_unlocked(new_capacity);
        if (this->_capacity < new_capacity)
        {
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], value);
    this->_size += 1;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::push_back(ElementType &&value)
{
    bool   lock_acquired;
    size_t new_capacity;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (this->_size >= this->_capacity)
    {
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        this->reserve_internal_unlocked(new_capacity);
        if (this->_capacity < new_capacity)
        {
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], ft_move(value));
    this->_size += 1;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::pop_back()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (this->_size > 0)
    {
        destroy_at(&this->_data[this->_size - 1]);
        this->_size -= 1;
        this->set_error(FT_ERR_SUCCESSS);
    }
    else
        this->set_error(FT_ERR_INVALID_OPERATION);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType& ft_vector<ElementType>::operator[](size_t index)
{
    static ElementType default_instance = ElementType();
    bool               lock_acquired;
    ElementType       *ref_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (default_instance);
    }
    if (index >= this->_size)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        return (default_instance);
    }
    ref_pointer = &this->_data[index];
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*ref_pointer);
}

template <typename ElementType>
const ElementType& ft_vector<ElementType>::operator[](size_t index) const
{
    static ElementType default_instance = ElementType();
    bool               lock_acquired;
    const ElementType *ref_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_vector<ElementType> *>(this)->set_error(ft_errno);
        return (default_instance);
    }
    if (index >= this->_size)
    {
        const_cast<ft_vector<ElementType> *>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        return (default_instance);
    }
    ref_pointer = &this->_data[index];
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*ref_pointer);
}

template <typename ElementType>
void ft_vector<ElementType>::clear()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->destroy_elements_unlocked(0, this->_size);
    this->_size = 0;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::reserve_internal_unlocked(size_t new_capacity)
{
    if (new_capacity <= this->_capacity)
        return ;
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(new_capacity * sizeof(ElementType)));
    if (new_data == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    bool had_small_buffer = this->using_small_buffer();
    ElementType* old_data = this->_data;
    size_t current_size = this->_size;
    size_t index = 0;
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
            destroy_at(&new_data[index]);
        }
        cma_free(new_data);
        throw;
    }
    size_t destroy_index = 0;
    while (destroy_index < current_size)
    {
        destroy_at(&old_data[destroy_index]);
        ++destroy_index;
    }
    if (had_small_buffer == false && old_data != ft_nullptr)
        cma_free(old_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    return ;
}

template <typename ElementType>
int ft_vector<ElementType>::lock_internal(bool *lock_acquired) const
{
    ft_errno = FT_ERR_SUCCESSS;
    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        if (this->_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = FT_ERR_SUCCESSS;
            if (this->_mutex->lock_state(&state_lock_acquired) == 0)
                this->_mutex->unlock_state(state_lock_acquired);
            ft_errno = FT_ERR_SUCCESSS;
            return (0);
        }
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

template <typename ElementType>
void ft_vector<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    ft_errno = FT_ERR_SUCCESSS;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::teardown_thread_safety()
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
void ft_vector<ElementType>::reserve(size_t new_capacity)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->reserve_internal_unlocked(new_capacity);
    if (this->_capacity >= new_capacity)
        this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::resize(size_t new_size, const ElementType& value)
{
    bool   lock_acquired;
    size_t index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (new_size < this->_size)
        this->destroy_elements_unlocked(new_size, this->_size);
    else if (new_size > this->_size)
    {
        this->reserve_internal_unlocked(new_size);
        if (this->_capacity < new_size)
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
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::insert(iterator pos, const ElementType& value)
{
    bool       lock_acquired;
    size_t     index;
    size_t     new_capacity;
    size_t     move_index;
    iterator   end_iterator;
    iterator   result_iterator;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (this->_data + this->_size);
    }
    index = pos - this->_data;
    if (index > this->_size)
    {
        end_iterator = this->_data + this->_size;
        this->set_error(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        return (end_iterator);
    }
    if (this->_size >= this->_capacity)
    {
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        this->reserve_internal_unlocked(new_capacity);
        if (this->_capacity < new_capacity)
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
        destroy_at(&this->_data[move_index - 1]);
        move_index -= 1;
    }
    construct_at(&this->_data[index], value);
    this->_size += 1;
    result_iterator = &this->_data[index];
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result_iterator);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::erase(iterator pos)
{
    bool       lock_acquired;
    size_t     index;
    size_t     shift_index;
    iterator   end_iterator;
    iterator   result_iterator;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (this->_data + this->_size);
    }
    index = pos - this->_data;
    if (index >= this->_size)
    {
        end_iterator = this->_data + this->_size;
        this->set_error(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        return (end_iterator);
    }
    destroy_at(&this->_data[index]);
    shift_index = index;
    while (shift_index < this->_size - 1)
    {
        construct_at(&this->_data[shift_index], ft_move(this->_data[shift_index + 1]));
        destroy_at(&this->_data[shift_index + 1]);
        shift_index += 1;
    }
    this->_size -= 1;
    if (index >= this->_size)
        result_iterator = this->_data + this->_size;
    else
        result_iterator = &this->_data[index];
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result_iterator);
}

template <typename ElementType>
ElementType ft_vector<ElementType>::release_at(size_t index)
{
    bool        lock_acquired;
    ElementType detached;
    size_t      shift_index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (ElementType());
    }
    if (index >= this->_size)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
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
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (detached);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::begin()
{
    bool     lock_acquired;
    iterator iterator_result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (this->_data);
    }
    iterator_result = this->_data;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (iterator_result);
}

template <typename ElementType>
typename ft_vector<ElementType>::const_iterator ft_vector<ElementType>::begin() const
{
    bool            lock_acquired;
    const_iterator  iterator_result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_vector<ElementType> *>(this)->set_error(ft_errno);
        return (this->_data);
    }
    iterator_result = this->_data;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (iterator_result);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::end()
{
    bool     lock_acquired;
    iterator iterator_result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (this->_data + this->_size);
    }
    iterator_result = this->_data + this->_size;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (iterator_result);
}

template <typename ElementType>
typename ft_vector<ElementType>::const_iterator ft_vector<ElementType>::end() const
{
    bool            lock_acquired;
    const_iterator  iterator_result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_vector<ElementType> *>(this)->set_error(ft_errno);
        return (this->_data + this->_size);
    }
    iterator_result = this->_data + this->_size;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (iterator_result);
}

#endif
