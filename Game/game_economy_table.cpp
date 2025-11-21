#include "game_economy_table.hpp"
#include "game_economy_helpers.hpp"
#include "../Template/move.hpp"

int ft_economy_table::lock_pair(const ft_economy_table &first, const ft_economy_table &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_economy_table *ordered_first;
    const ft_economy_table *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_economy_table *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_economy_sleep_backoff();
    }
}

ft_economy_table::ft_economy_table() noexcept
    : _error_code(ER_SUCCESS)
{
    return ;
}

ft_economy_table::~ft_economy_table() noexcept
{
    return ;
}

ft_economy_table::ft_economy_table(ft_economy_table &&other) noexcept
    : _error_code(ER_SUCCESS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    entry_errno = ft_errno;
    if (ft_economy_table::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_price_definitions = ft_move(other._price_definitions);
    this->_rarity_bands = ft_move(other._rarity_bands);
    this->_vendor_profiles = ft_move(other._vendor_profiles);
    this->_currency_rates = ft_move(other._currency_rates);
    this->_error_code = other._error_code;
    other._price_definitions.clear();
    other._rarity_bands.clear();
    other._vendor_profiles.clear();
    other._currency_rates.clear();
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return ;
}

ft_economy_table &ft_economy_table::operator=(ft_economy_table &&other) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    if (ft_economy_table::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_price_definitions = ft_move(other._price_definitions);
    this->_rarity_bands = ft_move(other._rarity_bands);
    this->_vendor_profiles = ft_move(other._vendor_profiles);
    this->_currency_rates = ft_move(other._currency_rates);
    this->_error_code = other._error_code;
    other._price_definitions.clear();
    other._rarity_bands.clear();
    other._vendor_profiles.clear();
    other._currency_rates.clear();
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_map<int, ft_price_definition> &ft_economy_table::get_price_definitions() noexcept
{
    return (this->_price_definitions);
}

const ft_map<int, ft_price_definition> &ft_economy_table::get_price_definitions() const noexcept
{
    return (this->_price_definitions);
}

ft_map<int, ft_rarity_band> &ft_economy_table::get_rarity_bands() noexcept
{
    return (this->_rarity_bands);
}

const ft_map<int, ft_rarity_band> &ft_economy_table::get_rarity_bands() const noexcept
{
    return (this->_rarity_bands);
}

ft_map<int, ft_vendor_profile> &ft_economy_table::get_vendor_profiles() noexcept
{
    return (this->_vendor_profiles);
}

const ft_map<int, ft_vendor_profile> &ft_economy_table::get_vendor_profiles() const noexcept
{
    return (this->_vendor_profiles);
}

ft_map<int, ft_currency_rate> &ft_economy_table::get_currency_rates() noexcept
{
    return (this->_currency_rates);
}

const ft_map<int, ft_currency_rate> &ft_economy_table::get_currency_rates() const noexcept
{
    return (this->_currency_rates);
}

void ft_economy_table::set_price_definitions(const ft_map<int, ft_price_definition> &price_definitions) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_price_definitions = price_definitions;
    this->set_error(this->_price_definitions.get_error());
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

void ft_economy_table::set_rarity_bands(const ft_map<int, ft_rarity_band> &rarity_bands) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_rarity_bands = rarity_bands;
    this->set_error(this->_rarity_bands.get_error());
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

void ft_economy_table::set_vendor_profiles(ft_map<int, ft_vendor_profile> &&vendor_profiles) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_vendor_profiles = ft_move(vendor_profiles);
    this->set_error(this->_vendor_profiles.get_error());
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

void ft_economy_table::set_currency_rates(const ft_map<int, ft_currency_rate> &currency_rates) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_currency_rates = currency_rates;
    this->set_error(this->_currency_rates.get_error());
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

int ft_economy_table::register_price_definition(const ft_price_definition &definition) noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (definition.get_error() != ER_SUCCESS)
    {
        this->set_error(definition.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (definition.get_error());
    }
    identifier = definition.get_item_id();
    this->_price_definitions.insert(identifier, definition);
    if (this->_price_definitions.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_price_definitions.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (this->_price_definitions.get_error());
    }
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_economy_table::register_rarity_band(const ft_rarity_band &band) noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (band.get_error() != ER_SUCCESS)
    {
        this->set_error(band.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (band.get_error());
    }
    identifier = band.get_rarity();
    this->_rarity_bands.insert(identifier, band);
    if (this->_rarity_bands.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_rarity_bands.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (this->_rarity_bands.get_error());
    }
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_economy_table::register_vendor_profile(const ft_vendor_profile &profile) noexcept
{
    int entry_errno;
    int identifier;
    ft_vendor_profile stored_profile;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (profile.get_error() != ER_SUCCESS)
    {
        this->set_error(profile.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (profile.get_error());
    }
    identifier = profile.get_vendor_id();
    stored_profile.set_vendor_id(identifier);
    stored_profile.set_buy_markup(profile.get_buy_markup());
    stored_profile.set_sell_multiplier(profile.get_sell_multiplier());
    stored_profile.set_tax_rate(profile.get_tax_rate());
    this->_vendor_profiles.insert(identifier, ft_move(stored_profile));
    if (this->_vendor_profiles.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_vendor_profiles.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (this->_vendor_profiles.get_error());
    }
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_economy_table::register_currency_rate(const ft_currency_rate &rate) noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (rate.get_error() != ER_SUCCESS)
    {
        this->set_error(rate.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (rate.get_error());
    }
    identifier = rate.get_currency_id();
    this->_currency_rates.insert(identifier, rate);
    if (this->_currency_rates.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_currency_rates.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (this->_currency_rates.get_error());
    }
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_economy_table::fetch_price_definition(int item_id, ft_price_definition &definition) const noexcept
{
    int entry_errno;
    const ft_economy_table *self;
    const Pair<int, ft_price_definition> *entry;

    self = this;
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_economy_table *>(self)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    entry = self->_price_definitions.find(item_id);
    if (entry == self->_price_definitions.end())
    {
        const_cast<ft_economy_table *>(self)->set_error(FT_ERR_NOT_FOUND);
        game_economy_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    definition = entry->value;
    const_cast<ft_economy_table *>(self)->set_error(entry->value.get_error());
    game_economy_restore_errno(guard, entry_errno);
    return (entry->value.get_error());
}

int ft_economy_table::fetch_rarity_band(int rarity, ft_rarity_band &band) const noexcept
{
    int entry_errno;
    const ft_economy_table *self;
    const Pair<int, ft_rarity_band> *entry;

    self = this;
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_economy_table *>(self)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    entry = self->_rarity_bands.find(rarity);
    if (entry == self->_rarity_bands.end())
    {
        const_cast<ft_economy_table *>(self)->set_error(FT_ERR_NOT_FOUND);
        game_economy_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    band = entry->value;
    const_cast<ft_economy_table *>(self)->set_error(entry->value.get_error());
    game_economy_restore_errno(guard, entry_errno);
    return (entry->value.get_error());
}

int ft_economy_table::fetch_vendor_profile(int vendor_id, ft_vendor_profile &profile) const noexcept
{
    int entry_errno;
    const ft_economy_table *self;
    const Pair<int, ft_vendor_profile> *entry;

    self = this;
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_economy_table *>(self)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    entry = self->_vendor_profiles.find(vendor_id);
    if (entry == self->_vendor_profiles.end())
    {
        const_cast<ft_economy_table *>(self)->set_error(FT_ERR_NOT_FOUND);
        game_economy_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    profile.set_vendor_id(entry->value.get_vendor_id());
    profile.set_buy_markup(entry->value.get_buy_markup());
    profile.set_sell_multiplier(entry->value.get_sell_multiplier());
    profile.set_tax_rate(entry->value.get_tax_rate());
    const_cast<ft_economy_table *>(self)->set_error(entry->value.get_error());
    game_economy_restore_errno(guard, entry_errno);
    return (entry->value.get_error());
}

int ft_economy_table::fetch_currency_rate(int currency_id, ft_currency_rate &rate) const noexcept
{
    int entry_errno;
    const ft_economy_table *self;
    const Pair<int, ft_currency_rate> *entry;

    self = this;
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_economy_table *>(self)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    entry = self->_currency_rates.find(currency_id);
    if (entry == self->_currency_rates.end())
    {
        const_cast<ft_economy_table *>(self)->set_error(FT_ERR_NOT_FOUND);
        game_economy_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    rate = entry->value;
    const_cast<ft_economy_table *>(self)->set_error(entry->value.get_error());
    game_economy_restore_errno(guard, entry_errno);
    return (entry->value.get_error());
}

int ft_economy_table::get_error() const noexcept
{
    int entry_errno;
    int error_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_economy_table *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_value = this->_error_code;
    const_cast<ft_economy_table *>(this)->set_error(error_value);
    game_economy_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *ft_economy_table::get_error_str() const noexcept
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

void ft_economy_table::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}
