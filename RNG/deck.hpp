#ifndef RNG_DECK_HPP
#define RNG_DECK_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Template/vector.hpp"
#include "../Template/swap.hpp"
#include "../Template/move.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "rng.hpp"
#include <climits>

template<typename ElementType>
class ft_deck : public ft_vector<ElementType*>
{
    private:
        mutable pt_recursive_mutex *_mutex;

        int     prepare_thread_safety();
        void    teardown_thread_safety();

    public:
        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        ft_deck() noexcept;
        ~ft_deck() noexcept;

        ft_deck(const ft_deck&) = delete;
        ft_deck &operator=(const ft_deck&) = delete;

        ft_deck(ft_deck&&) = delete;
        ft_deck &operator=(ft_deck&&) = delete;

        ElementType *popRandomElement() noexcept;
        ElementType *getRandomElement() const noexcept;
        void shuffle() noexcept;
        ElementType *drawTopElement() noexcept;
        ElementType *peekTopElement() const noexcept;
};


template<typename ElementType>
ft_deck<ElementType>::ft_deck() noexcept
    : ft_vector<ElementType*>()
    , _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    return ;
}

template<typename ElementType>
ft_deck<ElementType>::~ft_deck() noexcept
{
    this->disable_thread_safety();
    return ;
}

template<typename ElementType>
int ft_deck<ElementType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template<typename ElementType>
void ft_deck<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template<typename ElementType>
int ft_deck<ElementType>::enable_thread_safety()
{
    int base_result = ft_vector<ElementType*>::enable_thread_safety();
    if (base_result != FT_ERR_SUCCESSS)
        return (base_result);
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template<typename ElementType>
void ft_deck<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_vector<ElementType*>::disable_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
bool ft_deck<ElementType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template<typename ElementType>
ElementType *ft_deck<ElementType>::popRandomElement() noexcept
{
    bool lock_acquired = false;
    int lock_error = FT_ERR_SUCCESSS;
    if (this->_mutex != ft_nullptr)
    {
        lock_error = pt_recursive_mutex_lock_with_error(*this->_mutex);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_acquired = true;
    }
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_error;
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    int operation_error = FT_ERR_SUCCESSS;
    ElementType *element = ft_nullptr;
    do
    {
        size_t deck_size = this->size();
        int size_error = ft_global_error_stack_peek_last_error();
        if (size_error != FT_ERR_SUCCESSS)
        {
            operation_error = size_error;
            break;
        }
        if (deck_size == 0)
        {
            operation_error = FT_ERR_EMPTY;
            break;
        }
        if (deck_size > static_cast<size_t>(INT_MAX))
        {
            operation_error = FT_ERR_OUT_OF_RANGE;
            break;
        }
        int roll_result = ft_dice_roll(1, static_cast<int>(deck_size));
        if (roll_result < 1 || ft_errno != FT_ERR_SUCCESSS)
        {
            operation_error = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_OUT_OF_RANGE;
            break;
        }
        size_t index = static_cast<size_t>(roll_result - 1);
        element = this->release_at(index);
        int release_error = ft_global_error_stack_peek_last_error();
        if (release_error != FT_ERR_SUCCESSS)
        {
            operation_error = release_error;
            element = ft_nullptr;
            break;
        }
    } while (false);
    int unlock_error = FT_ERR_SUCCESSS;
    if (lock_acquired)
        unlock_error = pt_recursive_mutex_unlock_with_error(*this->_mutex);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    ft_errno = operation_error;
    ft_global_error_stack_push(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (ft_nullptr);
    return (element);
}

template<typename ElementType>
ElementType *ft_deck<ElementType>::getRandomElement() const noexcept
{
    bool lock_acquired = false;
    int lock_error = FT_ERR_SUCCESSS;
    if (this->_mutex != ft_nullptr)
    {
        lock_error = pt_recursive_mutex_lock_with_error(*this->_mutex);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_acquired = true;
    }
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_error;
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    int operation_error = FT_ERR_SUCCESSS;
    ElementType *element = ft_nullptr;
    do
    {
        size_t deck_size = this->size();
        int size_error = ft_global_error_stack_peek_last_error();
        if (size_error != FT_ERR_SUCCESSS)
        {
            operation_error = size_error;
            break;
        }
        if (deck_size == 0)
        {
            operation_error = FT_ERR_EMPTY;
            break;
        }
        if (deck_size > static_cast<size_t>(INT_MAX))
        {
            operation_error = FT_ERR_OUT_OF_RANGE;
            break;
        }
        int roll_result = ft_dice_roll(1, static_cast<int>(deck_size));
        if (roll_result < 1 || ft_errno != FT_ERR_SUCCESSS)
        {
            operation_error = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_OUT_OF_RANGE;
            break;
        }
        size_t index = static_cast<size_t>(roll_result - 1);
        element = (*this)[index];
        int access_error = ft_global_error_stack_peek_last_error();
        if (access_error != FT_ERR_SUCCESSS)
        {
            operation_error = access_error;
            element = ft_nullptr;
            break;
        }
    } while (false);
    int unlock_error = FT_ERR_SUCCESSS;
    if (lock_acquired)
        unlock_error = pt_recursive_mutex_unlock_with_error(*this->_mutex);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    ft_errno = operation_error;
    ft_global_error_stack_push(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (ft_nullptr);
    return (element);
}

template<typename ElementType>
ElementType *ft_deck<ElementType>::drawTopElement() noexcept
{
    bool lock_acquired = false;
    int lock_error = FT_ERR_SUCCESSS;
    if (this->_mutex != ft_nullptr)
    {
        lock_error = pt_recursive_mutex_lock_with_error(*this->_mutex);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_acquired = true;
    }
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_error;
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    int operation_error = FT_ERR_SUCCESSS;
    ElementType *element = ft_nullptr;
    do
    {
        size_t deck_size = this->size();
        int size_error = ft_global_error_stack_peek_last_error();
        if (size_error != FT_ERR_SUCCESSS)
        {
            operation_error = size_error;
            break;
        }
        if (deck_size == 0)
        {
            operation_error = FT_ERR_EMPTY;
            break;
        }
        if (deck_size > static_cast<size_t>(INT_MAX))
        {
            operation_error = FT_ERR_OUT_OF_RANGE;
            break;
        }
        size_t index = deck_size - 1;
        element = this->release_at(index);
        int release_error = ft_global_error_stack_peek_last_error();
        if (release_error != FT_ERR_SUCCESSS)
        {
            operation_error = release_error;
            element = ft_nullptr;
            break;
        }
    } while (false);
    int unlock_error = FT_ERR_SUCCESSS;
    if (lock_acquired)
        unlock_error = pt_recursive_mutex_unlock_with_error(*this->_mutex);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    ft_errno = operation_error;
    ft_global_error_stack_push(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (ft_nullptr);
    return (element);
}

template<typename ElementType>
ElementType *ft_deck<ElementType>::peekTopElement() const noexcept
{
    bool lock_acquired = false;
    int lock_error = FT_ERR_SUCCESSS;
    if (this->_mutex != ft_nullptr)
    {
        lock_error = pt_recursive_mutex_lock_with_error(*this->_mutex);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_acquired = true;
    }
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_error;
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    int operation_error = FT_ERR_SUCCESSS;
    ElementType *element = ft_nullptr;
    do
    {
        size_t deck_size = this->size();
        int size_error = ft_global_error_stack_peek_last_error();
        if (size_error != FT_ERR_SUCCESSS)
        {
            operation_error = size_error;
            break;
        }
        if (deck_size == 0)
        {
            operation_error = FT_ERR_EMPTY;
            break;
        }
        if (deck_size > static_cast<size_t>(INT_MAX))
        {
            operation_error = FT_ERR_OUT_OF_RANGE;
            break;
        }
        size_t index = deck_size - 1;
        ElementType *current_element = (*this)[index];
        int access_error = ft_global_error_stack_peek_last_error();
        if (access_error != FT_ERR_SUCCESSS)
        {
            operation_error = access_error;
            break;
        }
        element = current_element;
    } while (false);
    int unlock_error = FT_ERR_SUCCESSS;
    if (lock_acquired)
        unlock_error = pt_recursive_mutex_unlock_with_error(*this->_mutex);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    ft_errno = operation_error;
    ft_global_error_stack_push(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (ft_nullptr);
    return (element);
}

template<typename ElementType>
void ft_deck<ElementType>::shuffle() noexcept
{
    bool lock_acquired = false;
    int lock_error = FT_ERR_SUCCESSS;
    if (this->_mutex != ft_nullptr)
    {
        lock_error = pt_recursive_mutex_lock_with_error(*this->_mutex);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_acquired = true;
    }
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_error;
        ft_global_error_stack_push(lock_error);
        return ;
    }
    int operation_error = FT_ERR_SUCCESSS;
    do
    {
        size_t deck_size = this->size();
        int size_error = ft_global_error_stack_peek_last_error();
        if (size_error != FT_ERR_SUCCESSS)
        {
            operation_error = size_error;
            break;
        }
        if (deck_size == 0)
        {
            operation_error = FT_ERR_EMPTY;
            break;
        }
        if (deck_size > static_cast<size_t>(INT_MAX))
        {
            operation_error = FT_ERR_OUT_OF_RANGE;
            break;
        }
        size_t index = deck_size - 1;
        while (index > 0)
        {
            if (index + 1 > static_cast<size_t>(INT_MAX))
            {
                operation_error = FT_ERR_OUT_OF_RANGE;
                break;
            }
            int roll_result = ft_dice_roll(1, static_cast<int>(index + 1));
            if (roll_result < 1 || ft_errno != FT_ERR_SUCCESSS)
            {
                operation_error = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_OUT_OF_RANGE;
                break;
            }
            size_t random_index = static_cast<size_t>(roll_result - 1);
            ElementType *first_element = (*this)[index];
            int first_error = ft_global_error_stack_peek_last_error();
            if (first_error != FT_ERR_SUCCESSS)
            {
                operation_error = first_error;
                break;
            }
            ElementType *second_element = (*this)[random_index];
            int second_error = ft_global_error_stack_peek_last_error();
            if (second_error != FT_ERR_SUCCESSS)
            {
                operation_error = second_error;
                break;
            }
            ft_swap(first_element, second_element);
            (*this)[index] = first_element;
            int assign_first_error = ft_global_error_stack_peek_last_error();
            if (assign_first_error != FT_ERR_SUCCESSS)
            {
                operation_error = assign_first_error;
                break;
            }
            (*this)[random_index] = second_element;
            int assign_second_error = ft_global_error_stack_peek_last_error();
            if (assign_second_error != FT_ERR_SUCCESSS)
            {
                operation_error = assign_second_error;
                break;
            }
            index -= 1;
        }
    } while (false);
    int unlock_error = FT_ERR_SUCCESSS;
    if (lock_acquired)
        unlock_error = pt_recursive_mutex_unlock_with_error(*this->_mutex);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    ft_errno = operation_error;
    ft_global_error_stack_push(operation_error);
    return ;
}

#endif
