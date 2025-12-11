#ifndef FT_REGION_DEFINITION_HPP
# define FT_REGION_DEFINITION_HPP

#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_region_definition
{
    private:
        int _region_id;
        ft_string _name;
        ft_string _description;
        int _recommended_level;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_region_definition &first, const ft_region_definition &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_region_definition() noexcept;
        ft_region_definition(int region_id, const ft_string &name, const ft_string &description,
                int recommended_level) noexcept;
        virtual ~ft_region_definition() noexcept;
        ft_region_definition(const ft_region_definition &other) noexcept;
        ft_region_definition &operator=(const ft_region_definition &other) noexcept;
        ft_region_definition(ft_region_definition &&other) noexcept;
        ft_region_definition &operator=(ft_region_definition &&other) noexcept;

        int get_region_id() const noexcept;
        void set_region_id(int region_id) noexcept;

        const ft_string &get_name() const noexcept;
        void set_name(const ft_string &name) noexcept;

        const ft_string &get_description() const noexcept;
        void set_description(const ft_string &description) noexcept;

        int get_recommended_level() const noexcept;
        void set_recommended_level(int recommended_level) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
