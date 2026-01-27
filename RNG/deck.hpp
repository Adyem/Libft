#ifndef RNG_DECK_HPP
#define RNG_DECK_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Template/vector.hpp"
#include "../Template/swap.hpp"
#include "../Template/move.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/lock_error_helpers.hpp"
#include "rng.hpp"
#include <climits>

template<typename ElementType>
class ft_deck : public ft_vector<ElementType*>
{
    private:
        mutable pt_mutex _mutex;

        void    set_error_unlocked(int error_code) const noexcept;
        void    set_error(int error_code) const noexcept;
        int     lock_deck(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void sleep_backoff() noexcept;
        static int lock_pair(const ft_deck<ElementType> &first, const ft_deck<ElementType> &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard) noexcept;

    public:
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
void ft_deck<ElementType>::set_error_unlocked(int error_code) const noexcept
{
    ft_vector<ElementType*>::set_error(error_code);
    return ;
}

template<typename ElementType>
void ft_deck<ElementType>::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

template<typename ElementType>
int ft_deck<ElementType>::lock_deck(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    {
        int lock_error = ft_unique_lock_pop_last_error(local_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            guard = ft_unique_lock<pt_mutex>();
            ft_errno = lock_error;
            return (lock_error);
        }
    }
    guard = ft_move(local_guard);
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}

template<typename ElementType>
void ft_deck<ElementType>::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

template<typename ElementType>
int ft_deck<ElementType>::lock_pair(const ft_deck<ElementType> &first, const ft_deck<ElementType> &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const ft_deck<ElementType> *ordered_first;
    const ft_deck<ElementType> *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        {
            int lock_error = ft_unique_lock_pop_last_error(single_guard);

            if (lock_error != FT_ERR_SUCCESSS)
            {
                ft_errno = lock_error;
                return (lock_error);
            }
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_deck<ElementType> *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);
        int lower_error = ft_unique_lock_pop_last_error(lower_guard);

        if (lower_error != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_error;
            return (lower_error);
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        int upper_error = ft_unique_lock_pop_last_error(upper_guard);
        if (upper_error == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_error;
            return (upper_error);
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_deck<ElementType>::sleep_backoff();
    }
}

template<typename ElementType>
ft_deck<ElementType>::ft_deck() noexcept
    : ft_vector<ElementType*>()
    , _mutex()
{
    if (this->enable_thread_safety() != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
ft_deck<ElementType>::~ft_deck() noexcept
{
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
ElementType *ft_deck<ElementType>::popRandomElement() noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    size_t deck_size;
    int roll_result;
    size_t index;
    ElementType *element;

    lock_error = this->lock_deck(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (ft_nullptr);
    }
    deck_size = this->size();
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->get_error());
        return (ft_nullptr);
    }
    if (deck_size == 0)
    {
        this->set_error(FT_ERR_EMPTY);
        return (ft_nullptr);
    }
    if (deck_size > static_cast<size_t>(INT_MAX))
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (ft_nullptr);
    }
    roll_result = ft_dice_roll(1, static_cast<int>(deck_size));
    if (ft_errno != FT_ERR_SUCCESSS || roll_result < 1)
    {
        this->set_error(ft_errno);
        return (ft_nullptr);
    }
    index = static_cast<size_t>(roll_result - 1);
    element = this->release_at(index);
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->get_error());
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (element);
}

template<typename ElementType>
ElementType *ft_deck<ElementType>::getRandomElement() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    size_t deck_size;
    int roll_result;
    size_t index;
    ElementType *element;

    lock_error = this->lock_deck(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(lock_error);
        return (ft_nullptr);
    }
    deck_size = this->size();
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(this->get_error());
        return (ft_nullptr);
    }
    if (deck_size == 0)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(FT_ERR_EMPTY);
        return (ft_nullptr);
    }
    if (deck_size > static_cast<size_t>(INT_MAX))
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        return (ft_nullptr);
    }
    roll_result = ft_dice_roll(1, static_cast<int>(deck_size));
    if (ft_errno != FT_ERR_SUCCESSS || roll_result < 1)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    index = static_cast<size_t>(roll_result - 1);
    element = (*this)[index];
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(this->get_error());
        return (ft_nullptr);
    }
    const_cast<ft_deck<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (element);
}

template<typename ElementType>
ElementType *ft_deck<ElementType>::drawTopElement() noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    size_t deck_size;
    size_t index;
    ElementType *element;

    lock_error = this->lock_deck(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (ft_nullptr);
    }
    deck_size = this->size();
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->get_error());
        return (ft_nullptr);
    }
    if (deck_size == 0)
    {
        this->set_error(FT_ERR_EMPTY);
        return (ft_nullptr);
    }
    index = deck_size - 1;
    element = this->release_at(index);
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->get_error());
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (element);
}

template<typename ElementType>
ElementType *ft_deck<ElementType>::peekTopElement() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    size_t deck_size;
    size_t index;
    ElementType *element;

    lock_error = this->lock_deck(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(lock_error);
        return (ft_nullptr);
    }
    deck_size = this->size();
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(this->get_error());
        return (ft_nullptr);
    }
    if (deck_size == 0)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(FT_ERR_EMPTY);
        return (ft_nullptr);
    }
    index = deck_size - 1;
    element = (*this)[index];
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_deck<ElementType> *>(this)->set_error(this->get_error());
        return (ft_nullptr);
    }
    const_cast<ft_deck<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (element);
}

template<typename ElementType>
void ft_deck<ElementType>::shuffle() noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    size_t deck_size;
    size_t index;

    lock_error = this->lock_deck(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return ;
    }
    deck_size = this->size();
    if (this->get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->get_error());
        return ;
    }
    if (deck_size == 0)
    {
        this->set_error(FT_ERR_EMPTY);
        return ;
    }
    if (deck_size > static_cast<size_t>(INT_MAX))
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    index = deck_size - 1;
    while (index > 0)
    {
        if (index + 1 > static_cast<size_t>(INT_MAX))
        {
            this->set_error(FT_ERR_OUT_OF_RANGE);
            return ;
        }
        int roll_result = ft_dice_roll(1, static_cast<int>(index + 1));
        if (ft_errno != FT_ERR_SUCCESSS || roll_result < 1)
        {
            this->set_error(ft_errno);
            return ;
        }
        size_t random_index = static_cast<size_t>(roll_result - 1);
        ElementType *first_element = (*this)[index];
        if (this->get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->get_error());
            return ;
        }
        ElementType *second_element = (*this)[random_index];
        if (this->get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->get_error());
            return ;
        }
        ft_swap(first_element, second_element);
        (*this)[index] = first_element;
        if (this->get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->get_error());
            return ;
        }
        (*this)[random_index] = second_element;
        if (this->get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->get_error());
            return ;
        }
        index -= 1;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

#endif
