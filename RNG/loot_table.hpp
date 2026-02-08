#ifndef RNG_LOOT_TABLE_HPP
#define RNG_LOOT_TABLE_HPP

#include "../Template/vector.hpp"
#include "../Template/move.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "rng.hpp"
#include <climits>

template<typename ElementType>
class ft_loot_entry
{
    private:
        ElementType *_item;
        int _weight;
        int _rarity;
        mutable pt_recursive_mutex *_mutex;

        void        teardown_thread_safety();
        int         prepare_thread_safety();
        int         lock_entry(bool *lock_acquired) const noexcept;
        int         unlock_entry(bool lock_acquired) const noexcept;

    public:
        ft_loot_entry() noexcept;
        ft_loot_entry(ElementType *item, int weight, int rarity) noexcept;
        ft_loot_entry(const ft_loot_entry &other) noexcept;
        ft_loot_entry &operator=(const ft_loot_entry &other) noexcept;
        ~ft_loot_entry() noexcept;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;

        void set_item(ElementType *item) noexcept;
        ElementType *get_item() const noexcept;

        void set_weight(int weight) noexcept;
        int get_weight() const noexcept;

        void set_rarity(int rarity) noexcept;
        int get_rarity() const noexcept;

        int get_effective_weight() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

template<typename ElementType>
class ft_loot_table : public ft_vector<ft_loot_entry<ElementType> >
{
    private:
        mutable pt_recursive_mutex *_mutex;

        void        teardown_thread_safety();
        int         prepare_thread_safety();

        int compute_total_weight_locked(size_t entry_count, int *total_weight) const noexcept;
        int locate_entry_by_roll_locked(int roll, size_t entry_count, size_t *result_index) const noexcept;

    public:
        ft_loot_table() noexcept;
        ~ft_loot_table() noexcept;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;

        void addElement(ElementType *elem, int weight, int rarity);
        ElementType *getRandomLoot() const;
        ElementType *popRandomLoot();

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};
template<typename ElementType>
int ft_loot_entry<ElementType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template<typename ElementType>
void ft_loot_entry<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template<typename ElementType>
int ft_loot_entry<ElementType>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template<typename ElementType>
void ft_loot_entry<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
bool ft_loot_entry<ElementType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

#ifdef LIBFT_TEST_BUILD
template<typename ElementType>
pt_recursive_mutex *ft_loot_entry<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

template<typename ElementType>
inline int ft_loot_entry<ElementType>::lock_entry(bool *lock_acquired) const noexcept
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int lock_error = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template<typename ElementType>
inline int ft_loot_entry<ElementType>::unlock_entry(bool lock_acquired) const noexcept
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template<typename ElementType>
ft_loot_entry<ElementType>::ft_loot_entry() noexcept
    : _item(ft_nullptr)
    , _weight(1)
    , _rarity(0)
    , _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType>::ft_loot_entry(ElementType *item, int weight, int rarity) noexcept
    : _item(ft_nullptr)
    , _weight(1)
    , _rarity(0)
    , _mutex(ft_nullptr)
{
    if (weight <= 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_item = item;
    this->_weight = weight;
    if (rarity < 0)
        rarity = 0;
    this->_rarity = rarity;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType>::ft_loot_entry(const ft_loot_entry<ElementType> &other) noexcept
    : _item(ft_nullptr)
    , _weight(1)
    , _rarity(0)
    , _mutex(ft_nullptr)
{
    bool lock_acquired = false;
    int lock_result = other.lock_entry(&lock_acquired);
    if (lock_result != 0)
    {
        int error_code = ft_global_error_stack_peek_last_error();
        ft_errno = error_code;
        return ;
    }
    this->_item = other._item;
    this->_weight = other._weight;
    this->_rarity = other._rarity;
    int unlock_error = other.unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = unlock_error;
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType> &ft_loot_entry<ElementType>::operator=(const ft_loot_entry<ElementType> &other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired = false;
    bool other_lock_acquired = false;
    int lock_result = FT_ERR_SUCCESSS;
    if (this < &other)
    {
        lock_result = this->lock_entry(&this_lock_acquired);
        if (lock_result == 0)
            lock_result = other.lock_entry(&other_lock_acquired);
    }
    else
    {
        lock_result = other.lock_entry(&other_lock_acquired);
        if (lock_result == 0)
            lock_result = this->lock_entry(&this_lock_acquired);
    }
    if (lock_result != 0)
    {
        int error_code = ft_global_error_stack_peek_last_error();
        ft_errno = error_code;
        if (other_lock_acquired)
            other.unlock_entry(other_lock_acquired);
        if (this_lock_acquired)
            this->unlock_entry(this_lock_acquired);
        return (*this);
    }
    this->_item = other._item;
    this->_weight = other._weight;
    this->_rarity = other._rarity;
    int first_unlock = this->unlock_entry(this_lock_acquired);
    int second_unlock = other.unlock_entry(other_lock_acquired);
    int final_error = FT_ERR_SUCCESSS;
    if (first_unlock != FT_ERR_SUCCESSS)
        final_error = first_unlock;
    else if (second_unlock != FT_ERR_SUCCESSS)
        final_error = second_unlock;
    ft_errno = final_error;
    ft_global_error_stack_push(final_error);
    return (*this);
}

template<typename ElementType>
ft_loot_entry<ElementType>::~ft_loot_entry() noexcept
{
    this->teardown_thread_safety();
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
void ft_loot_entry<ElementType>::set_item(ElementType *item) noexcept
{
    bool lock_acquired = false;
    int lock_result = this->lock_entry(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_result;
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_item = item;
    int unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = unlock_error;
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
ElementType *ft_loot_entry<ElementType>::get_item() const noexcept
{
    bool lock_acquired = false;
    int lock_result = this->lock_entry(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_result;
        ft_global_error_stack_push(lock_result);
        return (ft_nullptr);
    }
    ElementType *item_pointer = this->_item;
    int unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = unlock_error;
        ft_global_error_stack_push(unlock_error);
        return (ft_nullptr);
    }
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (item_pointer);
}

template<typename ElementType>
void ft_loot_entry<ElementType>::set_weight(int weight) noexcept
{
    if (weight <= 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    bool lock_acquired = false;
    int lock_result = this->lock_entry(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_result;
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_weight = weight;
    int unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = unlock_error;
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
int ft_loot_entry<ElementType>::get_weight() const noexcept
{
    bool lock_acquired = false;
    int lock_result = this->lock_entry(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_result;
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    int weight_value = this->_weight;
    int unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = unlock_error;
        ft_global_error_stack_push(unlock_error);
        return (0);
    }
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (weight_value);
}

template<typename ElementType>
void ft_loot_entry<ElementType>::set_rarity(int rarity) noexcept
{
    if (rarity < 0)
        rarity = 0;
    bool lock_acquired = false;
    int lock_result = this->lock_entry(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_result;
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_rarity = rarity;
    int unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = unlock_error;
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
int ft_loot_entry<ElementType>::get_rarity() const noexcept
{
    bool lock_acquired = false;
    int lock_result = this->lock_entry(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_result;
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    int rarity_value = this->_rarity;
    int unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = unlock_error;
        ft_global_error_stack_push(unlock_error);
        return (0);
    }
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (rarity_value);
}

template<typename ElementType>
int ft_loot_entry<ElementType>::get_effective_weight() const noexcept
{
    bool lock_acquired = false;
    int lock_result = this->lock_entry(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_errno = lock_result;
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    int effective_weight = this->_weight / (this->_rarity + 1);
    if (effective_weight < 1)
        effective_weight = 1;
    int unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_errno = unlock_error;
        ft_global_error_stack_push(unlock_error);
        return (0);
    }
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (effective_weight);
}

template<typename ElementType>
int ft_loot_table<ElementType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template<typename ElementType>
void ft_loot_table<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template<typename ElementType>
int ft_loot_table<ElementType>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template<typename ElementType>
void ft_loot_table<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
bool ft_loot_table<ElementType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

#ifdef LIBFT_TEST_BUILD
template<typename ElementType>
pt_recursive_mutex *ft_loot_table<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

template<typename ElementType>
int ft_loot_table<ElementType>::compute_total_weight_locked(size_t entry_count, int *total_weight) const noexcept
{
    if (total_weight == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    int computed_total = 0;
    size_t index = 0;
    while (index < entry_count)
    {
        const ft_loot_entry<ElementType> &entry = (*this)[index];
        int entry_error = ft_global_error_stack_peek_last_error();
        if (entry_error != FT_ERR_SUCCESSS)
        {
            ft_errno = entry_error;
            return (entry_error);
        }
        int effective_weight = entry.get_effective_weight();
        int weight_error = ft_global_error_stack_peek_last_error();
        if (weight_error != FT_ERR_SUCCESSS)
        {
            ft_errno = weight_error;
            return (weight_error);
        }
        if (effective_weight <= 0)
            effective_weight = 1;
        if (INT_MAX - computed_total < effective_weight)
        {
            ft_errno = FT_ERR_OUT_OF_RANGE;
            return (FT_ERR_OUT_OF_RANGE);
        }
        computed_total += effective_weight;
        index += 1;
    }
    *total_weight = computed_total;
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}

template<typename ElementType>
int ft_loot_table<ElementType>::locate_entry_by_roll_locked(int roll, size_t entry_count, size_t *result_index) const noexcept
{
    if (result_index == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    size_t index = 0;
    int accumulated = 0;
    while (index < entry_count)
    {
        const ft_loot_entry<ElementType> &entry = (*this)[index];
        int entry_error = ft_global_error_stack_peek_last_error();
        if (entry_error != FT_ERR_SUCCESSS)
        {
            ft_errno = entry_error;
            return (entry_error);
        }
        int effective_weight = entry.get_effective_weight();
        int weight_error = ft_global_error_stack_peek_last_error();
        if (weight_error != FT_ERR_SUCCESSS)
        {
            ft_errno = weight_error;
            return (weight_error);
        }
        if (effective_weight <= 0)
            effective_weight = 1;
        if (INT_MAX - accumulated < effective_weight)
        {
            ft_errno = FT_ERR_OUT_OF_RANGE;
            return (FT_ERR_OUT_OF_RANGE);
        }
        accumulated += effective_weight;
        if (roll <= accumulated)
        {
            *result_index = index;
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        index += 1;
    }
    ft_errno = FT_ERR_OUT_OF_RANGE;
    return (FT_ERR_OUT_OF_RANGE);
}

template<typename ElementType>
ft_loot_table<ElementType>::ft_loot_table() noexcept
    : ft_vector<ft_loot_entry<ElementType> >()
    , _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
ft_loot_table<ElementType>::~ft_loot_table() noexcept
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename ElementType>
void ft_loot_table<ElementType>::addElement(ElementType *elem, int weight, int rarity)
{
    if (weight <= 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (rarity < 0)
        rarity = 0;
    ft_loot_entry<ElementType> entry(elem, weight, rarity);
    int entry_error = ft_global_error_stack_peek_last_error();
    if (entry_error != FT_ERR_SUCCESSS)
    {
        ft_errno = entry_error;
        return ;
    }
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
        this->push_back(entry);
        operation_error = ft_global_error_stack_peek_last_error();
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

template<typename ElementType>
ElementType *ft_loot_table<ElementType>::getRandomLoot() const
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
    ElementType *loot_item = ft_nullptr;
    do
    {
        size_t entry_count = this->size();
        int size_error = ft_global_error_stack_peek_last_error();
        if (size_error != FT_ERR_SUCCESSS)
        {
            operation_error = size_error;
            break;
        }
        if (entry_count == 0)
        {
            operation_error = FT_ERR_EMPTY;
            ft_errno = FT_ERR_EMPTY;
            break;
        }
        int total_weight = 0;
        operation_error = this->compute_total_weight_locked(entry_count, &total_weight);
        if (operation_error != FT_ERR_SUCCESSS)
            break;
        int roll = ft_dice_roll(1, total_weight);
        if (roll < 1 || ft_errno != FT_ERR_SUCCESSS)
        {
            operation_error = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_OUT_OF_RANGE;
            break;
        }
        size_t selected_index = 0;
        operation_error = this->locate_entry_by_roll_locked(roll, entry_count, &selected_index);
        if (operation_error != FT_ERR_SUCCESSS)
            break;
        const ft_loot_entry<ElementType> &selected_entry = (*this)[selected_index];
        int entry_error = ft_global_error_stack_peek_last_error();
        if (entry_error != FT_ERR_SUCCESSS)
        {
            operation_error = entry_error;
            break;
        }
        loot_item = selected_entry.get_item();
        int item_error = ft_global_error_stack_peek_last_error();
        if (item_error != FT_ERR_SUCCESSS)
        {
            operation_error = item_error;
            loot_item = ft_nullptr;
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
    return (loot_item);
}

template<typename ElementType>
ElementType *ft_loot_table<ElementType>::popRandomLoot()
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
    ElementType *loot_item = ft_nullptr;
    do
    {
        size_t entry_count = this->size();
        int size_error = ft_global_error_stack_peek_last_error();
        if (size_error != FT_ERR_SUCCESSS)
        {
            operation_error = size_error;
            break;
        }
        if (entry_count == 0)
        {
            operation_error = FT_ERR_EMPTY;
            ft_errno = FT_ERR_EMPTY;
            break;
        }
        int total_weight = 0;
        operation_error = this->compute_total_weight_locked(entry_count, &total_weight);
        if (operation_error != FT_ERR_SUCCESSS)
            break;
        int roll = ft_dice_roll(1, total_weight);
        if (roll < 1 || ft_errno != FT_ERR_SUCCESSS)
        {
            operation_error = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_OUT_OF_RANGE;
            break;
        }
        size_t selected_index = 0;
        operation_error = this->locate_entry_by_roll_locked(roll, entry_count, &selected_index);
        if (operation_error != FT_ERR_SUCCESSS)
            break;
        ft_loot_entry<ElementType> removed_entry = this->release_at(selected_index);
        int release_error = ft_global_error_stack_peek_last_error();
        if (release_error != FT_ERR_SUCCESSS)
        {
            operation_error = release_error;
            break;
        }
        loot_item = removed_entry.get_item();
        int item_error = ft_global_error_stack_peek_last_error();
        if (item_error != FT_ERR_SUCCESSS)
        {
            operation_error = item_error;
            loot_item = ft_nullptr;
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
    return (loot_item);
}

#endif
