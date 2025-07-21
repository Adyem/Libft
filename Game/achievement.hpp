#ifndef ACHIEVEMENT_HPP
# define ACHIEVEMENT_HPP

class ft_achievement
{
    private:
        int _id;
        int _goal;
        int _progress;

    public:
        ft_achievement() noexcept;
        virtual ~ft_achievement() = default;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_goal() const noexcept;
        void set_goal(int goal) noexcept;

        int get_progress() const noexcept;
        void set_progress(int progress) noexcept;
        void add_progress(int value) noexcept;

        bool is_complete() const noexcept;
};

#endif
