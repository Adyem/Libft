#ifndef LOOT_TABLE_HPP
#define LOOT_TABLE_HPP

#include "../Template/vector.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Errno/errno.hpp"
#include "RNG.hpp"
#include <climits>

template<typename ElementType>
struct ft_loot_entry
{
    ElementType *item;
    int weight;
};

template<typename ElementType>
class ft_loot_table : public ft_vector<ft_loot_entry<ElementType> >
{
        public:
        void addElement(ElementType *elem, int weight);
        ElementType *getRandomLoot() const;
        ElementType *popRandomLoot();
};

template<typename ElementType>
void ft_loot_table<ElementType>::addElement(ElementType *elem, int weight)
{
    if (weight <= 0)
        return ;
    ft_loot_entry<ElementType> entry = {elem, weight};
    this->push_back(entry);
    return ;
}

template<typename ElementType>
ElementType *ft_loot_table<ElementType>::getRandomLoot() const
{
    if (this->size() == 0)
    {
        ft_errno = LOOT_TABLE_EMPTY;
        const_cast<ft_loot_table<ElementType>*>(this)->setError(LOOT_TABLE_EMPTY);
        return (ft_nullptr);
    }
    int totalWeight = 0;
    size_t index = 0;
    while (index < this->size())
    {
        if (INT_MAX - totalWeight < (*this)[index].weight)
            return (ft_nullptr);
        totalWeight += (*this)[index].weight;
        ++index;
    }
    int roll = ft_dice_roll(1, totalWeight);
    int accumulated = 0;
    index = 0;
    while (index < this->size())
    {
        accumulated += (*this)[index].weight;
        if (roll <= accumulated)
            return ((*this)[index].item);
        ++index;
    }
    return (ft_nullptr);
}

template<typename ElementType>
ElementType *ft_loot_table<ElementType>::popRandomLoot()
{
    if (this->size() == 0)
    {
        ft_errno = LOOT_TABLE_EMPTY;
        this->setError(LOOT_TABLE_EMPTY);
        return (ft_nullptr);
    }
    int totalWeight = 0;
    size_t index = 0;
    while (index < this->size())
    {
        if (INT_MAX - totalWeight < (*this)[index].weight)
            return (ft_nullptr);
        totalWeight += (*this)[index].weight;
        ++index;
    }
    int roll = ft_dice_roll(1, totalWeight);
    int accumulated = 0;
    index = 0;
    while (index < this->size())
    {
        accumulated += (*this)[index].weight;
        if (roll <= accumulated)
        {
            ElementType *elem = (*this)[index].item;
            this->release_at(index);
            return (elem);
        }
        ++index;
    }
    return (ft_nullptr);
}

#endif
