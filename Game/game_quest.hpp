#ifndef GAME_QUEST_HPP
# define GAME_QUEST_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"
#include "game_item.hpp"

class ft_quest
{
    private:
        int _id;
        int _phases;
        int _current_phase;
        ft_string _description;
        ft_string _objective;
        int _reward_experience;
        ft_vector<ft_item> _reward_items;

    public:
        ft_quest() noexcept;
        virtual ~ft_quest() = default;
        ft_quest(const ft_quest &other) noexcept;
        ft_quest &operator=(const ft_quest &other) noexcept;
        ft_quest(ft_quest &&other) noexcept;
        ft_quest &operator=(ft_quest &&other) noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_phases() const noexcept;
        void set_phases(int phases) noexcept;

        int get_current_phase() const noexcept;
        void set_current_phase(int phase) noexcept;

        const ft_string &get_description() const noexcept;
        void set_description(const ft_string &description) noexcept;

        const ft_string &get_objective() const noexcept;
        void set_objective(const ft_string &objective) noexcept;

        int get_reward_experience() const noexcept;
        void set_reward_experience(int experience) noexcept;

        ft_vector<ft_item>       &get_reward_items() noexcept;
        const ft_vector<ft_item> &get_reward_items() const noexcept;
        void set_reward_items(const ft_vector<ft_item> &items) noexcept;

        bool is_complete() const noexcept;
        void advance_phase() noexcept;
};

#endif
