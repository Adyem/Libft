#ifndef RNG_LOOT_TABLE_HPP
#define RNG_LOOT_TABLE_HPP

#include "../Template/vector.hpp"
#include "../Template/move.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"
#include "rng.hpp"
#include <climits>

template<typename ElementType>
class ft_loot_entry
{
    private:
        ElementType *_item;
        int _weight;
        int _rarity;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_entry(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;

    public:
        ft_loot_entry() noexcept;
        ft_loot_entry(ElementType *item, int weight, int rarity) noexcept;
        ft_loot_entry(const ft_loot_entry &other) noexcept;
        ft_loot_entry &operator=(const ft_loot_entry &other) noexcept;
        ~ft_loot_entry() noexcept;

        void set_item(ElementType *item) noexcept;
        ElementType *get_item() const noexcept;

        void set_weight(int weight) noexcept;
        int get_weight() const noexcept;

        void set_rarity(int rarity) noexcept;
        int get_rarity() const noexcept;

        int get_effective_weight() const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

template<typename ElementType>
class ft_loot_table : public ft_vector<ft_loot_entry<ElementType> >
{
    private:
        mutable pt_mutex _mutex;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_table(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;

        int compute_total_weight_locked(size_t entry_count, int *total_weight) const noexcept;
        int locate_entry_by_roll_locked(int roll, size_t entry_count, size_t *result_index) const noexcept;

    public:
        ft_loot_table() noexcept;
        ~ft_loot_table() noexcept;

        void addElement(ElementType *elem, int weight, int rarity);
        ElementType *getRandomLoot() const;
        ElementType *popRandomLoot();
};

template<typename ElementType>
void ft_loot_entry<ElementType>::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template<typename ElementType>
void ft_loot_entry<ElementType>::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

template<typename ElementType>
int ft_loot_entry<ElementType>::lock_entry(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);

    entry_errno = ft_errno;
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

template<typename ElementType>
void ft_loot_entry<ElementType>::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (current_errno != ER_SUCCESS)
    {
        ft_errno = current_errno;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType>::ft_loot_entry() noexcept
    : _item(ft_nullptr)
    , _weight(1)
    , _rarity(0)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType>::ft_loot_entry(ElementType *item, int weight, int rarity) noexcept
    : _item(item)
    , _weight(1)
    , _rarity(0)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    this->set_item(item);
    this->set_weight(weight);
    this->set_rarity(rarity);
    if (this->_error_code != ER_SUCCESS)
        return ;
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType>::ft_loot_entry(const ft_loot_entry<ElementType> &other) noexcept
    : _item(ft_nullptr)
    , _weight(1)
    , _rarity(0)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = other.lock_entry(other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_loot_entry<ElementType>::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_item = other._item;
    this->_weight = other._weight;
    this->_rarity = other._rarity;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(other_guard, entry_errno);
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType> &ft_loot_entry<ElementType>::operator=(const ft_loot_entry<ElementType> &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
    {
        this->set_error_unlocked(ER_SUCCESS);
        return (*this);
    }
    entry_errno = ft_errno;
    lock_error = this->lock_entry(this_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_loot_entry<ElementType>::restore_errno(this_guard, entry_errno);
        return (*this);
    }
    lock_error = other.lock_entry(other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_loot_entry<ElementType>::restore_errno(other_guard, entry_errno);
        ft_loot_entry<ElementType>::restore_errno(this_guard, entry_errno);
        return (*this);
    }
    this->_item = other._item;
    this->_weight = other._weight;
    this->_rarity = other._rarity;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(other_guard, entry_errno);
    ft_loot_entry<ElementType>::restore_errno(this_guard, entry_errno);
    return (*this);
}

template<typename ElementType>
ft_loot_entry<ElementType>::~ft_loot_entry() noexcept
{
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template<typename ElementType>
void ft_loot_entry<ElementType>::set_item(ElementType *item) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return ;
    }
    this->_item = item;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return ;
}

template<typename ElementType>
ElementType *ft_loot_entry<ElementType>::get_item() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    ElementType *item_pointer;

    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    item_pointer = this->_item;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return (item_pointer);
}

template<typename ElementType>
void ft_loot_entry<ElementType>::set_weight(int weight) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (weight <= 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return ;
    }
    this->_weight = weight;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return ;
}

template<typename ElementType>
int ft_loot_entry<ElementType>::get_weight() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int weight_value;

    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return (0);
    }
    weight_value = this->_weight;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return (weight_value);
}

template<typename ElementType>
void ft_loot_entry<ElementType>::set_rarity(int rarity) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (rarity < 0)
        rarity = 0;
    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return ;
    }
    this->_rarity = rarity;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return ;
}

template<typename ElementType>
int ft_loot_entry<ElementType>::get_rarity() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int rarity_value;

    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return (0);
    }
    rarity_value = this->_rarity;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return (rarity_value);
}

template<typename ElementType>
int ft_loot_entry<ElementType>::get_effective_weight() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int effective_weight;

    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return (0);
    }
    effective_weight = this->_weight / (this->_rarity + 1);
    if (effective_weight < 1)
        effective_weight = 1;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return (effective_weight);
}

template<typename ElementType>
int ft_loot_entry<ElementType>::get_error() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return (lock_error);
    }
    current_error = this->_error_code;
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return (current_error);
}

template<typename ElementType>
const char *ft_loot_entry<ElementType>::get_error_str() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    const char *error_string;

    entry_errno = ft_errno;
    lock_error = this->lock_entry(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
        return (ft_strerror(lock_error));
    }
    error_string = ft_strerror(this->_error_code);
    this->set_error_unlocked(ER_SUCCESS);
    ft_loot_entry<ElementType>::restore_errno(guard, entry_errno);
    return (error_string);
}

template<typename ElementType>
void ft_loot_table<ElementType>::set_error_unlocked(int error_code) const noexcept
{
    ft_vector<ft_loot_entry<ElementType> >::set_error(error_code);
    return ;
}

template<typename ElementType>
void ft_loot_table<ElementType>::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

template<typename ElementType>
int ft_loot_table<ElementType>::lock_table(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);

    entry_errno = ft_errno;
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

template<typename ElementType>
void ft_loot_table<ElementType>::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (current_errno != ER_SUCCESS)
    {
        ft_errno = current_errno;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template<typename ElementType>
int ft_loot_table<ElementType>::compute_total_weight_locked(size_t entry_count, int *total_weight) const noexcept
{
    size_t index;
    int computed_total;

    if (total_weight == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    computed_total = 0;
    index = 0;
    while (index < entry_count)
    {
        const ft_loot_entry<ElementType> &entry = (*this)[index];
        if (this->get_error() != ER_SUCCESS)
        {
            ft_errno = this->get_error();
            return (this->get_error());
        }
        int effective_weight = entry.get_effective_weight();
        if (entry.get_error() != ER_SUCCESS)
        {
            ft_errno = entry.get_error();
            return (entry.get_error());
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
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

template<typename ElementType>
int ft_loot_table<ElementType>::locate_entry_by_roll_locked(int roll, size_t entry_count, size_t *result_index) const noexcept
{
    size_t index;
    int accumulated;

    if (result_index == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    index = 0;
    accumulated = 0;
    while (index < entry_count)
    {
        const ft_loot_entry<ElementType> &entry = (*this)[index];
        if (this->get_error() != ER_SUCCESS)
        {
            ft_errno = this->get_error();
            return (this->get_error());
        }
        int effective_weight = entry.get_effective_weight();
        if (entry.get_error() != ER_SUCCESS)
        {
            ft_errno = entry.get_error();
            return (entry.get_error());
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        index += 1;
    }
    ft_errno = FT_ERR_OUT_OF_RANGE;
    return (FT_ERR_OUT_OF_RANGE);
}

template<typename ElementType>
ft_loot_table<ElementType>::ft_loot_table() noexcept
    : ft_vector<ft_loot_entry<ElementType> >()
    , _mutex()
{
    if (this->enable_thread_safety() != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template<typename ElementType>
ft_loot_table<ElementType>::~ft_loot_table() noexcept
{
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template<typename ElementType>
void ft_loot_table<ElementType>::addElement(ElementType *elem, int weight, int rarity)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    entry_errno = ft_errno;
    if (weight <= 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (rarity < 0)
        rarity = 0;
    ft_loot_entry<ElementType> entry(elem, weight, rarity);
    if (entry.get_error() != ER_SUCCESS)
    {
        this->set_error(entry.get_error());
        return ;
    }
    lock_error = this->lock_table(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return ;
    }
    this->push_back(entry);
    if (this->get_error() != ER_SUCCESS)
    {
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
    return ;
}

template<typename ElementType>
ElementType *ft_loot_table<ElementType>::getRandomLoot() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    size_t entry_count;
    int total_weight;
    int roll;
    size_t selected_index;

    entry_errno = ft_errno;
    lock_error = this->lock_table(guard);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_loot_table<ElementType> *>(this)->set_error(lock_error);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    entry_count = this->size();
    if (this->get_error() != ER_SUCCESS)
    {
        const_cast<ft_loot_table<ElementType> *>(this)->set_error(this->get_error());
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (entry_count == 0)
    {
        const_cast<ft_loot_table<ElementType> *>(this)->set_error(FT_ERR_EMPTY);
        ft_errno = FT_ERR_EMPTY;
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    total_weight = 0;
    lock_error = this->compute_total_weight_locked(entry_count, &total_weight);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_loot_table<ElementType> *>(this)->set_error(lock_error);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    roll = ft_dice_roll(1, total_weight);
    if (ft_errno != ER_SUCCESS)
    {
        const_cast<ft_loot_table<ElementType> *>(this)->set_error(ft_errno);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    lock_error = this->locate_entry_by_roll_locked(roll, entry_count, &selected_index);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_loot_table<ElementType> *>(this)->set_error(lock_error);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    const ft_loot_entry<ElementType> &selected_entry = (*this)[selected_index];
    if (this->get_error() != ER_SUCCESS)
    {
        const_cast<ft_loot_table<ElementType> *>(this)->set_error(this->get_error());
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    ElementType *loot_item = selected_entry.get_item();
    if (selected_entry.get_error() != ER_SUCCESS)
    {
        const_cast<ft_loot_table<ElementType> *>(this)->set_error(selected_entry.get_error());
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    const_cast<ft_loot_table<ElementType> *>(this)->set_error(ER_SUCCESS);
    ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
    return (loot_item);
}

template<typename ElementType>
ElementType *ft_loot_table<ElementType>::popRandomLoot()
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    size_t entry_count;
    int total_weight;
    int roll;
    size_t selected_index;
    ft_loot_entry<ElementType> removed_entry;
    ElementType *loot_item;

    entry_errno = ft_errno;
    lock_error = this->lock_table(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    entry_count = this->size();
    if (this->get_error() != ER_SUCCESS)
    {
        this->set_error(this->get_error());
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (entry_count == 0)
    {
        this->set_error(FT_ERR_EMPTY);
        ft_errno = FT_ERR_EMPTY;
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    total_weight = 0;
    lock_error = this->compute_total_weight_locked(entry_count, &total_weight);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    roll = ft_dice_roll(1, total_weight);
    if (ft_errno != ER_SUCCESS)
    {
        this->set_error(ft_errno);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    lock_error = this->locate_entry_by_roll_locked(roll, entry_count, &selected_index);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    removed_entry = this->release_at(selected_index);
    if (this->get_error() != ER_SUCCESS)
    {
        this->set_error(this->get_error());
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    loot_item = removed_entry.get_item();
    if (removed_entry.get_error() != ER_SUCCESS)
    {
        this->set_error(removed_entry.get_error());
        ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    ft_loot_table<ElementType>::restore_errno(guard, entry_errno);
    return (loot_item);
}

#endif
