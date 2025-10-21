#ifndef RNG_LOOT_TABLE_HPP
#define RNG_LOOT_TABLE_HPP

#include "../Template/vector.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "rng.hpp"
#include <climits>

template<typename ElementType>
struct ft_loot_entry
{
    ElementType *item;
    int weight;
    int rarity;
};

template<typename ElementType>
class ft_loot_table : public ft_vector<ft_loot_entry<ElementType> >
{
    public:
        void addElement(ElementType *elem, int weight, int rarity);
        ElementType *getRandomLoot() const;
        ElementType *popRandomLoot();
};

template<typename ElementType>
void ft_loot_table<ElementType>::addElement(ElementType *elem, int weight, int rarity)
{
    if (weight <= 0)
        return ;
    if (rarity < 0)
        rarity = 0;
    ft_loot_entry<ElementType> entry = {elem, weight, rarity};
    this->push_back(entry);
    return ;
}

template<typename ElementType>
ElementType *ft_loot_table<ElementType>::getRandomLoot() const
{
    if (this->size() == 0)
    {
        ft_errno = FT_ERR_EMPTY;
        const_cast<ft_loot_table<ElementType>*>(this)->set_error(FT_ERR_EMPTY);
        return (ft_nullptr);
    }
    int total_weight = 0;
    size_t index = 0;
    while (index < this->size())
    {
        int effective = (*this)[index].weight / ((*this)[index].rarity + 1);
        if (effective < 1)
            effective = 1;
        if (INT_MAX - total_weight < effective)
        {
            ft_errno = FT_ERR_OUT_OF_RANGE;
            const_cast<ft_loot_table<ElementType>*>(this)->set_error(FT_ERR_OUT_OF_RANGE);
            return (ft_nullptr);
        }
        total_weight += effective;
        ++index;
    }
    int roll = ft_dice_roll(1, total_weight);
    int accumulated = 0;
    index = 0;
    while (index < this->size())
    {
        int effective = (*this)[index].weight / ((*this)[index].rarity + 1);
        if (effective < 1)
            effective = 1;
        accumulated += effective;
        if (roll <= accumulated)
        {
            ft_errno = ER_SUCCESS;
            const_cast<ft_loot_table<ElementType>*>(this)->set_error(ER_SUCCESS);
            return ((*this)[index].item);
        }
        ++index;
    }
    ft_errno = FT_ERR_OUT_OF_RANGE;
    const_cast<ft_loot_table<ElementType>*>(this)->set_error(FT_ERR_OUT_OF_RANGE);
    return (ft_nullptr);
}

template<typename ElementType>
ElementType *ft_loot_table<ElementType>::popRandomLoot()
{
    if (this->size() == 0)
    {
        ft_errno = FT_ERR_EMPTY;
        this->set_error(FT_ERR_EMPTY);
        return (ft_nullptr);
    }
    int total_weight = 0;
    size_t index = 0;
    while (index < this->size())
    {
        int effective = (*this)[index].weight / ((*this)[index].rarity + 1);
        if (effective < 1)
            effective = 1;
        if (INT_MAX - total_weight < effective)
        {
            ft_errno = FT_ERR_OUT_OF_RANGE;
            this->set_error(FT_ERR_OUT_OF_RANGE);
            return (ft_nullptr);
        }
        total_weight += effective;
        ++index;
    }
    int roll = ft_dice_roll(1, total_weight);
    int accumulated = 0;
    index = 0;
    while (index < this->size())
    {
        int effective = (*this)[index].weight / ((*this)[index].rarity + 1);
        if (effective < 1)
            effective = 1;
        accumulated += effective;
        if (roll <= accumulated)
        {
            ElementType *elem = (*this)[index].item;
            this->release_at(index);
            ft_errno = ER_SUCCESS;
            this->set_error(ER_SUCCESS);
            return (elem);
        }
        ++index;
    }
    ft_errno = FT_ERR_OUT_OF_RANGE;
    this->set_error(FT_ERR_OUT_OF_RANGE);
    return (ft_nullptr);
}

#endif
