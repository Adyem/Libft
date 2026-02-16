#ifndef RNG_DECK_HPP
#define RNG_DECK_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Template/vector.hpp"
#include "../Template/swap.hpp"
#include "../Template/pair.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "rng.hpp"
#include <climits>
#include <new>

template<typename ElementType>
class ft_deck : public ft_vector<ElementType*>
{
    private:
        mutable pt_recursive_mutex *_mutex;

        int lock_deck(bool *lock_acquired) const;
        int unlock_deck(bool lock_acquired) const;

    public:
        int enable_thread_safety();
        int disable_thread_safety();
        bool is_thread_safe() const;
        ft_deck() noexcept;
        ~ft_deck() noexcept;

        ft_deck(const ft_deck&) = delete;
        ft_deck &operator=(const ft_deck&) = delete;

        ft_deck(ft_deck&&) = delete;
        ft_deck &operator=(ft_deck&&) = delete;

        Pair<int, ElementType *> popRandomElement() noexcept;
        Pair<int, ElementType *> getRandomElement() const noexcept;
        int shuffle() noexcept;
        Pair<int, ElementType *> drawTopElement() noexcept;
        Pair<int, ElementType *> peekTopElement() const noexcept;
};

template<typename ElementType>
ft_deck<ElementType>::ft_deck() noexcept
    : ft_vector<ElementType*>()
    , _mutex(ft_nullptr)
{
    return ;
}

template<typename ElementType>
ft_deck<ElementType>::~ft_deck() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

template<typename ElementType>
int ft_deck<ElementType>::enable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int base_result;
    int mutex_error;

    base_result = ft_vector<ElementType*>::enable_thread_safety();
    if (base_result != FT_ERR_SUCCESS)
        return (base_result);
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

template<typename ElementType>
int ft_deck<ElementType>::disable_thread_safety()
{
    int destroy_error;
    int base_error;

    destroy_error = FT_ERR_SUCCESS;
    if (this->_mutex != ft_nullptr)
    {
        destroy_error = this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    base_error = ft_vector<ElementType*>::destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    return (base_error);
}

template<typename ElementType>
bool ft_deck<ElementType>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

template<typename ElementType>
int ft_deck<ElementType>::lock_deck(bool *lock_acquired) const
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template<typename ElementType>
int ft_deck<ElementType>::unlock_deck(bool lock_acquired) const
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

template<typename ElementType>
Pair<int, ElementType *> ft_deck<ElementType>::popRandomElement() noexcept
{
    bool lock_acquired;
    int operation_error;
    int unlock_error;
    size_t deck_size;
    int roll_result;
    size_t index;
    ElementType *element;

    element = ft_nullptr;
    lock_acquired = false;
    operation_error = this->lock_deck(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
    {
        deck_size = this->size();
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    if (operation_error == FT_ERR_SUCCESS && deck_size == 0)
        operation_error = FT_ERR_EMPTY;
    if (operation_error == FT_ERR_SUCCESS && deck_size > static_cast<size_t>(INT_MAX))
        operation_error = FT_ERR_OUT_OF_RANGE;
    if (operation_error == FT_ERR_SUCCESS)
    {
        roll_result = ft_dice_roll(1, static_cast<int>(deck_size));
        if (roll_result < 1)
            operation_error = FT_ERR_OUT_OF_RANGE;
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        index = static_cast<size_t>(roll_result - 1);
        element = this->release_at(index);
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    unlock_error = this->unlock_deck(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (operation_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(operation_error, ft_nullptr));
    return (Pair<int, ElementType *>(FT_ERR_SUCCESS, element));
}

template<typename ElementType>
Pair<int, ElementType *> ft_deck<ElementType>::getRandomElement() const noexcept
{
    bool lock_acquired;
    int operation_error;
    int unlock_error;
    size_t deck_size;
    int roll_result;
    size_t index;
    ElementType *element;

    element = ft_nullptr;
    lock_acquired = false;
    operation_error = this->lock_deck(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
    {
        deck_size = this->size();
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    if (operation_error == FT_ERR_SUCCESS && deck_size == 0)
        operation_error = FT_ERR_EMPTY;
    if (operation_error == FT_ERR_SUCCESS && deck_size > static_cast<size_t>(INT_MAX))
        operation_error = FT_ERR_OUT_OF_RANGE;
    if (operation_error == FT_ERR_SUCCESS)
    {
        roll_result = ft_dice_roll(1, static_cast<int>(deck_size));
        if (roll_result < 1)
            operation_error = FT_ERR_OUT_OF_RANGE;
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        index = static_cast<size_t>(roll_result - 1);
        element = (*this)[index];
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    unlock_error = this->unlock_deck(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (operation_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(operation_error, ft_nullptr));
    return (Pair<int, ElementType *>(FT_ERR_SUCCESS, element));
}

template<typename ElementType>
Pair<int, ElementType *> ft_deck<ElementType>::drawTopElement() noexcept
{
    bool lock_acquired;
    int operation_error;
    int unlock_error;
    size_t deck_size;
    size_t index;
    ElementType *element;

    element = ft_nullptr;
    lock_acquired = false;
    operation_error = this->lock_deck(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
    {
        deck_size = this->size();
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    if (operation_error == FT_ERR_SUCCESS && deck_size == 0)
        operation_error = FT_ERR_EMPTY;
    if (operation_error == FT_ERR_SUCCESS && deck_size > static_cast<size_t>(INT_MAX))
        operation_error = FT_ERR_OUT_OF_RANGE;
    if (operation_error == FT_ERR_SUCCESS)
    {
        index = deck_size - 1;
        element = this->release_at(index);
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    unlock_error = this->unlock_deck(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (operation_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(operation_error, ft_nullptr));
    return (Pair<int, ElementType *>(FT_ERR_SUCCESS, element));
}

template<typename ElementType>
Pair<int, ElementType *> ft_deck<ElementType>::peekTopElement() const noexcept
{
    bool lock_acquired;
    int operation_error;
    int unlock_error;
    size_t deck_size;
    size_t index;
    ElementType *element;

    element = ft_nullptr;
    lock_acquired = false;
    operation_error = this->lock_deck(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
    {
        deck_size = this->size();
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    if (operation_error == FT_ERR_SUCCESS && deck_size == 0)
        operation_error = FT_ERR_EMPTY;
    if (operation_error == FT_ERR_SUCCESS && deck_size > static_cast<size_t>(INT_MAX))
        operation_error = FT_ERR_OUT_OF_RANGE;
    if (operation_error == FT_ERR_SUCCESS)
    {
        index = deck_size - 1;
        element = (*this)[index];
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    unlock_error = this->unlock_deck(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (operation_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(operation_error, ft_nullptr));
    return (Pair<int, ElementType *>(FT_ERR_SUCCESS, element));
}

template<typename ElementType>
int ft_deck<ElementType>::shuffle() noexcept
{
    bool lock_acquired;
    int operation_error;
    int unlock_error;
    size_t deck_size;
    size_t index;
    int roll_result;
    size_t random_index;
    ElementType *first_element;
    ElementType *second_element;

    lock_acquired = false;
    operation_error = this->lock_deck(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
    {
        deck_size = this->size();
        operation_error = ft_vector<ElementType*>::last_operation_error();
    }
    if (operation_error == FT_ERR_SUCCESS && deck_size == 0)
        operation_error = FT_ERR_EMPTY;
    if (operation_error == FT_ERR_SUCCESS && deck_size > static_cast<size_t>(INT_MAX))
        operation_error = FT_ERR_OUT_OF_RANGE;
    if (operation_error == FT_ERR_SUCCESS)
    {
        index = deck_size - 1;
        while (index > 0)
        {
            if (index + 1 > static_cast<size_t>(INT_MAX))
            {
                operation_error = FT_ERR_OUT_OF_RANGE;
                break;
            }
            roll_result = ft_dice_roll(1, static_cast<int>(index + 1));
            if (roll_result < 1)
            {
                operation_error = FT_ERR_OUT_OF_RANGE;
                break;
            }
            random_index = static_cast<size_t>(roll_result - 1);
            first_element = (*this)[index];
            operation_error = ft_vector<ElementType*>::last_operation_error();
            if (operation_error != FT_ERR_SUCCESS)
                break;
            second_element = (*this)[random_index];
            operation_error = ft_vector<ElementType*>::last_operation_error();
            if (operation_error != FT_ERR_SUCCESS)
                break;
            ft_swap(first_element, second_element);
            (*this)[index] = first_element;
            operation_error = ft_vector<ElementType*>::last_operation_error();
            if (operation_error != FT_ERR_SUCCESS)
                break;
            (*this)[random_index] = second_element;
            operation_error = ft_vector<ElementType*>::last_operation_error();
            if (operation_error != FT_ERR_SUCCESS)
                break;
            index -= 1;
        }
    }
    unlock_error = this->unlock_deck(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (operation_error);
}

#endif
