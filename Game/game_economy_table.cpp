#include "../PThread/pthread_internal.hpp"
#include "game_economy_table.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_economy_table::_last_error = FT_ERR_SUCCESS;

ft_economy_table::ft_economy_table() noexcept
    : _price_definitions(), _rarity_bands(), _vendor_profiles(), _currency_rates(),
      _mutex(ft_nullptr), _initialized_state(ft_economy_table::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_economy_table::~ft_economy_table() noexcept
{
    if (this->_initialized_state == ft_economy_table::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_economy_table::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_economy_table lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_economy_table::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_economy_table::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

void ft_economy_table::set_error(int error_code) const noexcept
{
    ft_economy_table::_last_error = error_code;
    return ;
}

int ft_economy_table::initialize() noexcept
{
    if (this->_initialized_state == ft_economy_table::_state_initialized)
    {
        this->abort_lifecycle_error("ft_economy_table::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    int32_t error;
    error = this->_price_definitions.initialize();
    if (error)
    {
        this->set_error(error);
        return (error);
    }
    error = this->_rarity_bands.initialize();
    if (error)
    {
        this->set_error(error);
        return (error);
    }
    error = this->_vendor_profiles.initialize();
    if (error)
    {
        this->set_error(error);
        return (error);
    }
    error = this->_currency_rates.initialize();
    if (error)
    {
        this->set_error(error);
        return (error);
    }
    this->_initialized_state = ft_economy_table::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::initialize(const ft_economy_table &other) noexcept
{
    int initialize_error;
    size_t count;
    size_t index;
    const Pair<int, ft_price_definition> *price_entry;
    const Pair<int, ft_price_definition> *price_end;
    const Pair<int, ft_rarity_band> *rarity_entry;
    const Pair<int, ft_rarity_band> *rarity_end;
    const Pair<int, ft_vendor_profile> *vendor_entry;
    const Pair<int, ft_vendor_profile> *vendor_end;
    const Pair<int, ft_currency_rate> *currency_entry;
    const Pair<int, ft_currency_rate> *currency_end;

    if (other._initialized_state != ft_economy_table::_state_initialized)
    {
        other.abort_lifecycle_error("ft_economy_table::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    count = other._price_definitions.size();
    price_end = other._price_definitions.end();
    price_entry = price_end - count;
    index = 0;
    while (index < count)
    {
        this->_price_definitions.insert(price_entry->key, price_entry->value);
        price_entry++;
        index += 1;
    }
    count = other._rarity_bands.size();
    rarity_end = other._rarity_bands.end();
    rarity_entry = rarity_end - count;
    index = 0;
    while (index < count)
    {
        this->_rarity_bands.insert(rarity_entry->key, rarity_entry->value);
        rarity_entry++;
        index += 1;
    }
    count = other._vendor_profiles.size();
    vendor_end = other._vendor_profiles.end();
    vendor_entry = vendor_end - count;
    index = 0;
    while (index < count)
    {
        this->_vendor_profiles.insert(vendor_entry->key, vendor_entry->value);
        vendor_entry++;
        index += 1;
    }
    count = other._currency_rates.size();
    currency_end = other._currency_rates.end();
    currency_entry = currency_end - count;
    index = 0;
    while (index < count)
    {
        this->_currency_rates.insert(currency_entry->key, currency_entry->value);
        currency_entry++;
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::initialize(ft_economy_table &&other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_economy_table::_state_initialized)
    {
        other.abort_lifecycle_error("ft_economy_table::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize(static_cast<const ft_economy_table &>(other));
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    other._price_definitions.clear();
    other._rarity_bands.clear();
    other._vendor_profiles.clear();
    other._currency_rates.clear();
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::destroy() noexcept
{
    int map_error;
    int disable_error;

    if (this->_initialized_state != ft_economy_table::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    map_error = this->_price_definitions.destroy();
    if (map_error != FT_ERR_SUCCESS)
    {
        this->set_error(map_error);
        return (map_error);
    }
    map_error = this->_rarity_bands.destroy();
    if (map_error != FT_ERR_SUCCESS)
    {
        this->set_error(map_error);
        return (map_error);
    }
    map_error = this->_vendor_profiles.destroy();
    if (map_error != FT_ERR_SUCCESS)
    {
        this->set_error(map_error);
        return (map_error);
    }
    map_error = this->_currency_rates.destroy();
    if (map_error != FT_ERR_SUCCESS)
    {
        this->set_error(map_error);
        return (map_error);
    }
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_economy_table::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_economy_table::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_economy_table::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_economy_table::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_economy_table::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_economy_table::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_economy_table::lock");
    int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_economy_table::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_economy_table::unlock");
    int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    (void)unlock_result;
    return ;
}

ft_map<int, ft_price_definition> &ft_economy_table::get_price_definitions() noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_price_definitions");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_price_definitions);
}

const ft_map<int, ft_price_definition> &ft_economy_table::get_price_definitions() const noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_price_definitions const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_price_definitions);
}

ft_map<int, ft_rarity_band> &ft_economy_table::get_rarity_bands() noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_rarity_bands");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_rarity_bands);
}

const ft_map<int, ft_rarity_band> &ft_economy_table::get_rarity_bands() const noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_rarity_bands const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_rarity_bands);
}

ft_map<int, ft_vendor_profile> &ft_economy_table::get_vendor_profiles() noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_vendor_profiles");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_vendor_profiles);
}

const ft_map<int, ft_vendor_profile> &ft_economy_table::get_vendor_profiles() const noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_vendor_profiles const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_vendor_profiles);
}

ft_map<int, ft_currency_rate> &ft_economy_table::get_currency_rates() noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_currency_rates");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_currency_rates);
}

const ft_map<int, ft_currency_rate> &ft_economy_table::get_currency_rates() const noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_currency_rates const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_currency_rates);
}

void ft_economy_table::set_price_definitions(
    const ft_map<int, ft_price_definition> &price_definitions) noexcept
{
    this->abort_if_not_initialized("ft_economy_table::set_price_definitions");
    this->_price_definitions.clear();
    {
        size_t count;
        size_t index;
        const Pair<int, ft_price_definition> *entry;
        const Pair<int, ft_price_definition> *entry_end;

        count = price_definitions.size();
        entry_end = price_definitions.end();
        entry = entry_end - count;
        index = 0;
        while (index < count)
        {
            this->_price_definitions.insert(entry->key, entry->value);
            entry++;
            index += 1;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_economy_table::set_rarity_bands(
    const ft_map<int, ft_rarity_band> &rarity_bands) noexcept
{
    this->abort_if_not_initialized("ft_economy_table::set_rarity_bands");
    this->_rarity_bands.clear();
    {
        size_t count;
        size_t index;
        const Pair<int, ft_rarity_band> *entry;
        const Pair<int, ft_rarity_band> *entry_end;

        count = rarity_bands.size();
        entry_end = rarity_bands.end();
        entry = entry_end - count;
        index = 0;
        while (index < count)
        {
            this->_rarity_bands.insert(entry->key, entry->value);
            entry++;
            index += 1;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_economy_table::set_vendor_profiles(
    const ft_map<int, ft_vendor_profile> &vendor_profiles) noexcept
{
    this->abort_if_not_initialized("ft_economy_table::set_vendor_profiles");
    this->_vendor_profiles.clear();
    {
        size_t count;
        size_t index;
        const Pair<int, ft_vendor_profile> *entry;
        const Pair<int, ft_vendor_profile> *entry_end;

        count = vendor_profiles.size();
        entry_end = vendor_profiles.end();
        entry = entry_end - count;
        index = 0;
        while (index < count)
        {
            this->_vendor_profiles.insert(entry->key, entry->value);
            entry++;
            index += 1;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_economy_table::set_currency_rates(
    const ft_map<int, ft_currency_rate> &currency_rates) noexcept
{
    this->abort_if_not_initialized("ft_economy_table::set_currency_rates");
    this->_currency_rates.clear();
    {
        size_t count;
        size_t index;
        const Pair<int, ft_currency_rate> *entry;
        const Pair<int, ft_currency_rate> *entry_end;

        count = currency_rates.size();
        entry_end = currency_rates.end();
        entry = entry_end - count;
        index = 0;
        while (index < count)
        {
            this->_currency_rates.insert(entry->key, entry->value);
            entry++;
            index += 1;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_economy_table::register_price_definition(
    const ft_price_definition &definition) noexcept
{
    this->abort_if_not_initialized("ft_economy_table::register_price_definition");
    this->_price_definitions.insert(definition.get_item_id(), definition);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::register_rarity_band(const ft_rarity_band &band) noexcept
{
    this->abort_if_not_initialized("ft_economy_table::register_rarity_band");
    this->_rarity_bands.insert(band.get_rarity(), band);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::register_vendor_profile(
    const ft_vendor_profile &profile) noexcept
{
    this->abort_if_not_initialized("ft_economy_table::register_vendor_profile");
    this->_vendor_profiles.insert(profile.get_vendor_id(), profile);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::register_currency_rate(const ft_currency_rate &rate) noexcept
{
    this->abort_if_not_initialized("ft_economy_table::register_currency_rate");
    this->_currency_rates.insert(rate.get_currency_id(), rate);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::fetch_price_definition(int item_id,
    ft_price_definition &definition) const noexcept
{
    const Pair<int, ft_price_definition> *entry;

    this->abort_if_not_initialized("ft_economy_table::fetch_price_definition");
    entry = this->_price_definitions.find(item_id);
    if (entry == this->_price_definitions.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    definition.set_item_id(entry->value.get_item_id());
    definition.set_rarity(entry->value.get_rarity());
    definition.set_base_value(entry->value.get_base_value());
    definition.set_minimum_value(entry->value.get_minimum_value());
    definition.set_maximum_value(entry->value.get_maximum_value());
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::fetch_rarity_band(int rarity,
    ft_rarity_band &band) const noexcept
{
    const Pair<int, ft_rarity_band> *entry;

    this->abort_if_not_initialized("ft_economy_table::fetch_rarity_band");
    entry = this->_rarity_bands.find(rarity);
    if (entry == this->_rarity_bands.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    band.set_rarity(entry->value.get_rarity());
    band.set_value_multiplier(entry->value.get_value_multiplier());
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::fetch_vendor_profile(int vendor_id,
    ft_vendor_profile &profile) const noexcept
{
    const Pair<int, ft_vendor_profile> *entry;

    this->abort_if_not_initialized("ft_economy_table::fetch_vendor_profile");
    entry = this->_vendor_profiles.find(vendor_id);
    if (entry == this->_vendor_profiles.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    profile.initialize(entry->value);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_economy_table::fetch_currency_rate(int currency_id,
    ft_currency_rate &rate) const noexcept
{
    const Pair<int, ft_currency_rate> *entry;

    this->abort_if_not_initialized("ft_economy_table::fetch_currency_rate");
    entry = this->_currency_rates.find(currency_id);
    if (entry == this->_currency_rates.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    rate.set_currency_id(entry->value.get_currency_id());
    rate.set_rate_to_base(entry->value.get_rate_to_base());
    rate.set_display_precision(entry->value.get_display_precision());
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_economy_table::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_economy_table::get_mutex_for_validation");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_mutex);
}
#endif

int ft_economy_table::get_error() const noexcept
{
    return (ft_economy_table::_last_error);
}

const char *ft_economy_table::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}
