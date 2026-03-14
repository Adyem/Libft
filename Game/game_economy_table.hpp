#ifndef GAME_ECONOMY_TABLE_HPP
# define GAME_ECONOMY_TABLE_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

#include "ft_price_definition.hpp"
#include "ft_rarity_band.hpp"
#include "ft_vendor_profile.hpp"
#include "ft_currency_rate.hpp"

class ft_economy_table
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        ft_map<int32_t, ft_price_definition> _price_definitions;
        ft_map<int32_t, ft_rarity_band>      _rarity_bands;
        ft_map<int32_t, ft_vendor_profile>   _vendor_profiles;
        ft_map<int32_t, ft_currency_rate>    _currency_rates;
        pt_recursive_mutex                        *_mutex;
        uint8_t                          _initialised_state;
        static thread_local int32_t           _last_error;


        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        int32_t unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t set_error(int32_t error_code) noexcept;

    public:
        ft_economy_table() noexcept;
        ~ft_economy_table() noexcept;
        ft_economy_table(const ft_economy_table &other) noexcept;
        ft_economy_table &operator=(const ft_economy_table &other) noexcept = delete;
        ft_economy_table(ft_economy_table &&other) noexcept;
        ft_economy_table &operator=(ft_economy_table &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_economy_table &other) noexcept;
        int32_t initialize(ft_economy_table &&other) noexcept;
        int32_t move(ft_economy_table &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t lock(ft_bool *lock_acquired) const noexcept;
        void unlock(ft_bool lock_acquired) const noexcept;

        ft_map<int32_t, ft_price_definition> &get_price_definitions() noexcept;
        const ft_map<int32_t, ft_price_definition> &get_price_definitions() const noexcept;
        ft_map<int32_t, ft_rarity_band> &get_rarity_bands() noexcept;
        const ft_map<int32_t, ft_rarity_band> &get_rarity_bands() const noexcept;
        ft_map<int32_t, ft_vendor_profile> &get_vendor_profiles() noexcept;
        const ft_map<int32_t, ft_vendor_profile> &get_vendor_profiles() const noexcept;
        ft_map<int32_t, ft_currency_rate> &get_currency_rates() noexcept;
        const ft_map<int32_t, ft_currency_rate> &get_currency_rates() const noexcept;

        void set_price_definitions(const ft_map<int32_t, ft_price_definition> &price_definitions) noexcept;
        void set_rarity_bands(const ft_map<int32_t, ft_rarity_band> &rarity_bands) noexcept;
        void set_vendor_profiles(const ft_map<int32_t, ft_vendor_profile> &vendor_profiles) noexcept;
        void set_currency_rates(const ft_map<int32_t, ft_currency_rate> &currency_rates) noexcept;

        int32_t register_price_definition(const ft_price_definition &definition) noexcept;
        int32_t register_rarity_band(const ft_rarity_band &band) noexcept;
        int32_t register_vendor_profile(const ft_vendor_profile &profile) noexcept;
        int32_t register_currency_rate(const ft_currency_rate &rate) noexcept;

        int32_t fetch_price_definition(int32_t item_id, ft_price_definition &definition) const noexcept;
        int32_t fetch_rarity_band(int32_t rarity, ft_rarity_band &band) const noexcept;
        int32_t fetch_vendor_profile(int32_t vendor_id, ft_vendor_profile &profile) const noexcept;
        int32_t fetch_currency_rate(int32_t currency_id, ft_currency_rate &rate) const noexcept;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;

};

#endif
