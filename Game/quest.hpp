#ifndef QUEST_HPP
# define QUEST_HPP

class ft_quest
{
    private:
        int _id;
        int _phases;
        int _current_phase;

    public:
        ft_quest() noexcept;
        virtual ~ft_quest() = default;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_phases() const noexcept;
        void set_phases(int phases) noexcept;

        int get_current_phase() const noexcept;
        void set_current_phase(int phase) noexcept;

        bool is_complete() const noexcept;
        void advance_phase() noexcept;
};

#endif
