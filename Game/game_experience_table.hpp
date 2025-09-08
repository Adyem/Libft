#ifndef EXPERIENCE_TABLE_HPP
#define EXPERIENCE_TABLE_HPP

class ft_experience_table
{
    private:
        int         *_levels;
        int         _count;
        mutable int _error;

        void set_error(int err) const noexcept;
        bool is_valid(int count, const int *array) const noexcept;

    public:
        ft_experience_table(int count = 0) noexcept;
        ~ft_experience_table();

        int  get_count() const noexcept;
        int  get_level(int experience) const noexcept;
        int  get_value(int index) const noexcept;
        void set_value(int index, int value) noexcept;
        int  set_levels(const int *levels, int count) noexcept;
        int  generate_levels_total(int count, int base,
                                   double multiplier) noexcept;
        int  generate_levels_scaled(int count, int base,
                                    double multiplier) noexcept;
        int  resize(int new_count) noexcept;
        int  check_for_error() const noexcept;
        int  get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
