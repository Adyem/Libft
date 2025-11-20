#ifndef FT_WORLD_REGION_HPP
# define FT_WORLD_REGION_HPP

#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_world_region
{
    private:
        int _world_id;
        ft_vector<int> _region_ids;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_world_region &first, const ft_world_region &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_world_region() noexcept;
        ft_world_region(int world_id, const ft_vector<int> &region_ids) noexcept;
        virtual ~ft_world_region() noexcept;
        ft_world_region(const ft_world_region &other) noexcept;
        ft_world_region &operator=(const ft_world_region &other) noexcept;
        ft_world_region(ft_world_region &&other) noexcept;
        ft_world_region &operator=(ft_world_region &&other) noexcept;

        int get_world_id() const noexcept;
        void set_world_id(int world_id) noexcept;

        const ft_vector<int> &get_region_ids() const noexcept;
        ft_vector<int> &get_region_ids() noexcept;
        void set_region_ids(const ft_vector<int> &region_ids) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
