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
    private:
        ft_map<int, ft_price_definition> _price_definitions;
        ft_map<int, ft_rarity_band>      _rarity_bands;
        ft_map<int, ft_vendor_profile>   _vendor_profiles;
        ft_map<int, ft_currency_rate>    _currency_rates;
        pt_recursive_mutex                        *_mutex;
        uint8_t                          _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_economy_table() noexcept;
        ~ft_economy_table() noexcept;
        ft_economy_table(const ft_economy_table &other) noexcept = delete;
        ft_economy_table &operator=(const ft_economy_table &other) noexcept = delete;
        ft_economy_table(ft_economy_table &&other) noexcept = delete;
        ft_economy_table &operator=(ft_economy_table &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_economy_table &other) noexcept;
        int initialize(ft_economy_table &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

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

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
