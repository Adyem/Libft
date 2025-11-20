#ifndef GAME_ECONOMY_TABLE_HPP
# define GAME_ECONOMY_TABLE_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

#include "ft_price_definition.hpp"
#include "ft_rarity_band.hpp"
#include "ft_vendor_profile.hpp"
#include "ft_currency_rate.hpp"

class ft_economy_table
{
    private:
        ft_map<int, ft_price_definition> _price_definitions;
        ft_map<int, ft_rarity_band>      _rarity_bands;
        ft_map<int, ft_vendor_profile>   _vendor_profiles;
        ft_map<int, ft_currency_rate>    _currency_rates;
        mutable int                      _error_code;
        mutable pt_mutex                 _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_economy_table &first, const ft_economy_table &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_economy_table() noexcept;
        ~ft_economy_table() noexcept;
        ft_economy_table(const ft_economy_table &other) noexcept;
        ft_economy_table &operator=(const ft_economy_table &other) noexcept;
        ft_economy_table(ft_economy_table &&other) noexcept;
        ft_economy_table &operator=(ft_economy_table &&other) noexcept;

        ft_map<int, ft_price_definition> &get_price_definitions() noexcept;
        const ft_map<int, ft_price_definition> &get_price_definitions() const noexcept;
        ft_map<int, ft_rarity_band> &get_rarity_bands() noexcept;
        const ft_map<int, ft_rarity_band> &get_rarity_bands() const noexcept;
        ft_map<int, ft_vendor_profile> &get_vendor_profiles() noexcept;
        const ft_map<int, ft_vendor_profile> &get_vendor_profiles() const noexcept;
        ft_map<int, ft_currency_rate> &get_currency_rates() noexcept;
        const ft_map<int, ft_currency_rate> &get_currency_rates() const noexcept;

        void set_price_definitions(const ft_map<int, ft_price_definition> &price_definitions) noexcept;
        void set_rarity_bands(const ft_map<int, ft_rarity_band> &rarity_bands) noexcept;
        void set_vendor_profiles(const ft_map<int, ft_vendor_profile> &vendor_profiles) noexcept;
        void set_currency_rates(const ft_map<int, ft_currency_rate> &currency_rates) noexcept;

        int register_price_definition(const ft_price_definition &definition) noexcept;
        int register_rarity_band(const ft_rarity_band &band) noexcept;
        int register_vendor_profile(const ft_vendor_profile &profile) noexcept;
        int register_currency_rate(const ft_currency_rate &rate) noexcept;

        int fetch_price_definition(int item_id, ft_price_definition &definition) const noexcept;
        int fetch_rarity_band(int rarity, ft_rarity_band &band) const noexcept;
        int fetch_vendor_profile(int vendor_id, ft_vendor_profile &profile) const noexcept;
        int fetch_currency_rate(int currency_id, ft_currency_rate &rate) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
