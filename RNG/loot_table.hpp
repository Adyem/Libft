#ifndef RNG_LOOT_TABLE_HPP
#define RNG_LOOT_TABLE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../CMA/CMA.hpp"
#include "../Template/pair.hpp"
#include "rng.hpp"
#include <climits>
#include <new>

template<typename ElementType>
class ft_loot_entry
{
    private:
        ElementType *_item;
        int _weight;
        int _rarity;
        mutable pt_recursive_mutex *_mutex;

        int lock_entry(bool *lock_acquired) const noexcept;
        int unlock_entry(bool lock_acquired) const noexcept;

    public:
        ft_loot_entry() noexcept;
        ft_loot_entry(ElementType *item, int weight, int rarity) noexcept;
        ~ft_loot_entry() noexcept;

        ft_loot_entry(const ft_loot_entry &other) = delete;
        ft_loot_entry &operator=(const ft_loot_entry &other) = delete;
        ft_loot_entry(ft_loot_entry &&other) = delete;
        ft_loot_entry &operator=(ft_loot_entry &&other) = delete;

        int enable_thread_safety();
        int disable_thread_safety();
        bool is_thread_safe() const;

        int set_item(ElementType *item) noexcept;
        Pair<int, ElementType *> get_item() const noexcept;

        int set_weight(int weight) noexcept;
        Pair<int, int> get_weight() const noexcept;

        int set_rarity(int rarity) noexcept;
        Pair<int, int> get_rarity() const noexcept;

        Pair<int, int> get_effective_weight() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

template<typename ElementType>
class ft_loot_table
{
    private:
        ft_loot_entry<ElementType> **_entries;
        size_t _size;
        size_t _capacity;
        mutable pt_recursive_mutex *_mutex;

        int lock_table(bool *lock_acquired) const noexcept;
        int unlock_table(bool lock_acquired) const noexcept;
        int reserve_capacity(size_t requested_capacity);
        int compute_total_weight_locked(int *total_weight) const noexcept;
        int locate_entry_by_roll_locked(int roll, size_t *result_index) const noexcept;

    public:
        ft_loot_table() noexcept;
        ~ft_loot_table() noexcept;

        ft_loot_table(const ft_loot_table &other) = delete;
        ft_loot_table &operator=(const ft_loot_table &other) = delete;
        ft_loot_table(ft_loot_table &&other) = delete;
        ft_loot_table &operator=(ft_loot_table &&other) = delete;

        int enable_thread_safety();
        int disable_thread_safety();
        bool is_thread_safe() const;

        int addElement(ElementType *elem, int weight, int rarity);
        Pair<int, ElementType *> getRandomLoot() const;
        Pair<int, ElementType *> popRandomLoot();
        size_t size() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

template<typename ElementType>
ft_loot_entry<ElementType>::ft_loot_entry() noexcept
    : _item(ft_nullptr)
    , _weight(1)
    , _rarity(0)
    , _mutex(ft_nullptr)
{
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType>::ft_loot_entry(ElementType *item, int weight, int rarity) noexcept
    : _item(item)
    , _weight(1)
    , _rarity(0)
    , _mutex(ft_nullptr)
{
    if (weight > 0)
        this->_weight = weight;
    if (rarity >= 0)
        this->_rarity = rarity;
    return ;
}

template<typename ElementType>
ft_loot_entry<ElementType>::~ft_loot_entry() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

template<typename ElementType>
int ft_loot_entry<ElementType>::enable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

template<typename ElementType>
int ft_loot_entry<ElementType>::disable_thread_safety()
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

template<typename ElementType>
bool ft_loot_entry<ElementType>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

template<typename ElementType>
int ft_loot_entry<ElementType>::lock_entry(bool *lock_acquired) const noexcept
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
int ft_loot_entry<ElementType>::unlock_entry(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

template<typename ElementType>
int ft_loot_entry<ElementType>::set_item(ElementType *item) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    lock_acquired = false;
    lock_error = this->lock_entry(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_item = item;
    unlock_error = this->unlock_entry(lock_acquired);
    return (unlock_error);
}

template<typename ElementType>
Pair<int, ElementType *> ft_loot_entry<ElementType>::get_item() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;
    ElementType *item_pointer;

    lock_acquired = false;
    item_pointer = ft_nullptr;
    lock_error = this->lock_entry(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(lock_error, item_pointer));
    item_pointer = this->_item;
    unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(unlock_error, ft_nullptr));
    return (Pair<int, ElementType *>(FT_ERR_SUCCESS, item_pointer));
}

template<typename ElementType>
int ft_loot_entry<ElementType>::set_weight(int weight) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    if (weight <= 0)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = false;
    lock_error = this->lock_entry(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_weight = weight;
    unlock_error = this->unlock_entry(lock_acquired);
    return (unlock_error);
}

template<typename ElementType>
Pair<int, int> ft_loot_entry<ElementType>::get_weight() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;
    int weight_value;

    lock_acquired = false;
    weight_value = 0;
    lock_error = this->lock_entry(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (Pair<int, int>(lock_error, weight_value));
    weight_value = this->_weight;
    unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (Pair<int, int>(unlock_error, 0));
    return (Pair<int, int>(FT_ERR_SUCCESS, weight_value));
}

template<typename ElementType>
int ft_loot_entry<ElementType>::set_rarity(int rarity) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    if (rarity < 0)
        rarity = 0;
    lock_acquired = false;
    lock_error = this->lock_entry(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_rarity = rarity;
    unlock_error = this->unlock_entry(lock_acquired);
    return (unlock_error);
}

template<typename ElementType>
Pair<int, int> ft_loot_entry<ElementType>::get_rarity() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;
    int rarity_value;

    lock_acquired = false;
    rarity_value = 0;
    lock_error = this->lock_entry(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (Pair<int, int>(lock_error, rarity_value));
    rarity_value = this->_rarity;
    unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (Pair<int, int>(unlock_error, 0));
    return (Pair<int, int>(FT_ERR_SUCCESS, rarity_value));
}

template<typename ElementType>
Pair<int, int> ft_loot_entry<ElementType>::get_effective_weight() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;
    int effective_weight;

    lock_acquired = false;
    effective_weight = 0;
    lock_error = this->lock_entry(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (Pair<int, int>(lock_error, effective_weight));
    effective_weight = this->_weight / (this->_rarity + 1);
    if (effective_weight < 1)
        effective_weight = 1;
    unlock_error = this->unlock_entry(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (Pair<int, int>(unlock_error, 0));
    return (Pair<int, int>(FT_ERR_SUCCESS, effective_weight));
}

#ifdef LIBFT_TEST_BUILD
template<typename ElementType>
pt_recursive_mutex *ft_loot_entry<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

template<typename ElementType>
ft_loot_table<ElementType>::ft_loot_table() noexcept
    : _entries(ft_nullptr)
    , _size(0)
    , _capacity(0)
    , _mutex(ft_nullptr)
{
    (void)this->enable_thread_safety();
    return ;
}

template<typename ElementType>
ft_loot_table<ElementType>::~ft_loot_table() noexcept
{
    size_t index;

    index = 0;
    while (index < this->_size)
    {
        delete this->_entries[index];
        index += 1;
    }
    if (this->_entries != ft_nullptr)
        cma_free(this->_entries);
    this->_entries = ft_nullptr;
    this->_size = 0;
    this->_capacity = 0;
    (void)this->disable_thread_safety();
    return ;
}

template<typename ElementType>
int ft_loot_table<ElementType>::enable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

template<typename ElementType>
int ft_loot_table<ElementType>::disable_thread_safety()
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

template<typename ElementType>
bool ft_loot_table<ElementType>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

template<typename ElementType>
int ft_loot_table<ElementType>::lock_table(bool *lock_acquired) const noexcept
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
int ft_loot_table<ElementType>::unlock_table(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

template<typename ElementType>
int ft_loot_table<ElementType>::reserve_capacity(size_t requested_capacity)
{
    ft_loot_entry<ElementType> **new_entries;
    size_t index;

    if (requested_capacity <= this->_capacity)
        return (FT_ERR_SUCCESS);
    new_entries = static_cast<ft_loot_entry<ElementType> **>(
            cma_malloc(sizeof(ft_loot_entry<ElementType> *) * requested_capacity));
    if (new_entries == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    index = 0;
    while (index < this->_size)
    {
        new_entries[index] = this->_entries[index];
        index += 1;
    }
    if (this->_entries != ft_nullptr)
        cma_free(this->_entries);
    this->_entries = new_entries;
    this->_capacity = requested_capacity;
    return (FT_ERR_SUCCESS);
}

template<typename ElementType>
int ft_loot_table<ElementType>::compute_total_weight_locked(int *total_weight) const noexcept
{
    size_t index;
    int computed_total;
    Pair<int, int> effective_weight;

    if (total_weight == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    computed_total = 0;
    index = 0;
    while (index < this->_size)
    {
        effective_weight = this->_entries[index]->get_effective_weight();
        if (effective_weight.key != FT_ERR_SUCCESS)
            return (effective_weight.key);
        if (INT_MAX - computed_total < effective_weight.value)
            return (FT_ERR_OUT_OF_RANGE);
        computed_total += effective_weight.value;
        index += 1;
    }
    *total_weight = computed_total;
    return (FT_ERR_SUCCESS);
}

template<typename ElementType>
int ft_loot_table<ElementType>::locate_entry_by_roll_locked(int roll, size_t *result_index) const noexcept
{
    size_t index;
    int accumulated;
    Pair<int, int> effective_weight;

    if (result_index == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    accumulated = 0;
    index = 0;
    while (index < this->_size)
    {
        effective_weight = this->_entries[index]->get_effective_weight();
        if (effective_weight.key != FT_ERR_SUCCESS)
            return (effective_weight.key);
        if (INT_MAX - accumulated < effective_weight.value)
            return (FT_ERR_OUT_OF_RANGE);
        accumulated += effective_weight.value;
        if (roll <= accumulated)
        {
            *result_index = index;
            return (FT_ERR_SUCCESS);
        }
        index += 1;
    }
    return (FT_ERR_OUT_OF_RANGE);
}

template<typename ElementType>
int ft_loot_table<ElementType>::addElement(ElementType *elem, int weight, int rarity)
{
    bool lock_acquired;
    int lock_error;
    int operation_error;
    int unlock_error;
    size_t next_capacity;
    ft_loot_entry<ElementType> *entry_pointer;

    if (weight <= 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (rarity < 0)
        rarity = 0;
    lock_acquired = false;
    lock_error = this->lock_table(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    operation_error = FT_ERR_SUCCESS;
    if (this->_size == this->_capacity)
    {
        next_capacity = 4;
        if (this->_capacity > 0)
            next_capacity = this->_capacity * 2;
        operation_error = this->reserve_capacity(next_capacity);
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        entry_pointer = new (std::nothrow) ft_loot_entry<ElementType>(elem, weight, rarity);
        if (entry_pointer == ft_nullptr)
            operation_error = FT_ERR_NO_MEMORY;
        else
        {
            this->_entries[this->_size] = entry_pointer;
            this->_size += 1;
        }
    }
    unlock_error = this->unlock_table(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (operation_error);
}

template<typename ElementType>
Pair<int, ElementType *> ft_loot_table<ElementType>::getRandomLoot() const
{
    bool lock_acquired;
    int lock_error;
    int operation_error;
    int unlock_error;
    int total_weight;
    int roll;
    size_t selected_index;
    Pair<int, ElementType *> loot_result;

    loot_result = Pair<int, ElementType *>(FT_ERR_SUCCESS, ft_nullptr);
    lock_acquired = false;
    lock_error = this->lock_table(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(lock_error, ft_nullptr));
    operation_error = FT_ERR_SUCCESS;
    if (this->_size == 0)
        operation_error = FT_ERR_EMPTY;
    if (operation_error == FT_ERR_SUCCESS)
    {
        total_weight = 0;
        operation_error = this->compute_total_weight_locked(&total_weight);
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        roll = ft_dice_roll(1, total_weight);
        if (roll < 1)
            operation_error = FT_ERR_OUT_OF_RANGE;
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        selected_index = 0;
        operation_error = this->locate_entry_by_roll_locked(roll, &selected_index);
        if (operation_error == FT_ERR_SUCCESS)
            loot_result = this->_entries[selected_index]->get_item();
        if (operation_error == FT_ERR_SUCCESS && loot_result.key != FT_ERR_SUCCESS)
            operation_error = loot_result.key;
    }
    unlock_error = this->unlock_table(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (operation_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(operation_error, ft_nullptr));
    return (Pair<int, ElementType *>(FT_ERR_SUCCESS, loot_result.value));
}

template<typename ElementType>
Pair<int, ElementType *> ft_loot_table<ElementType>::popRandomLoot()
{
    bool lock_acquired;
    int lock_error;
    int operation_error;
    int unlock_error;
    int total_weight;
    int roll;
    size_t selected_index;
    size_t index;
    Pair<int, ElementType *> loot_result;

    loot_result = Pair<int, ElementType *>(FT_ERR_SUCCESS, ft_nullptr);
    lock_acquired = false;
    lock_error = this->lock_table(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(lock_error, ft_nullptr));
    operation_error = FT_ERR_SUCCESS;
    if (this->_size == 0)
        operation_error = FT_ERR_EMPTY;
    if (operation_error == FT_ERR_SUCCESS)
    {
        total_weight = 0;
        operation_error = this->compute_total_weight_locked(&total_weight);
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        roll = ft_dice_roll(1, total_weight);
        if (roll < 1)
            operation_error = FT_ERR_OUT_OF_RANGE;
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        selected_index = 0;
        operation_error = this->locate_entry_by_roll_locked(roll, &selected_index);
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        loot_result = this->_entries[selected_index]->get_item();
        if (loot_result.key != FT_ERR_SUCCESS)
            operation_error = loot_result.key;
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        delete this->_entries[selected_index];
        index = selected_index;
        while (index + 1 < this->_size)
        {
            this->_entries[index] = this->_entries[index + 1];
            index += 1;
        }
        this->_size -= 1;
        this->_entries[this->_size] = ft_nullptr;
    }
    unlock_error = this->unlock_table(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (operation_error != FT_ERR_SUCCESS)
        return (Pair<int, ElementType *>(operation_error, ft_nullptr));
    return (Pair<int, ElementType *>(FT_ERR_SUCCESS, loot_result.value));
}

template<typename ElementType>
size_t ft_loot_table<ElementType>::size() const noexcept
{
    return (this->_size);
}

#ifdef LIBFT_TEST_BUILD
template<typename ElementType>
pt_recursive_mutex *ft_loot_table<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
