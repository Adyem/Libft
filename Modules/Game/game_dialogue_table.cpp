#include "../PThread/pthread_internal.hpp"
#include "game_dialogue_table.hpp"
#include "../Printf/printf.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/map.hpp"
#include "../Template/pair.hpp"
#include "../Template/shared_ptr.hpp"
#include "game_dialogue_line.hpp"
#include "game_dialogue_script.hpp"

thread_local int32_t game_dialogue_table::_last_error = FT_ERR_SUCCESS;

static ft_sharedptr<game_dialogue_line> game_dialogue_table_clone_line(
    const game_dialogue_line &line)
{
    game_dialogue_line *cloned_line;
    int32_t initialize_error;

    if (line.is_initialised() == FT_FALSE)
        return (ft_sharedptr<game_dialogue_line>());
    cloned_line = new (std::nothrow) game_dialogue_line();
    if (cloned_line == ft_nullptr)
        return (ft_sharedptr<game_dialogue_line>());
    initialize_error = cloned_line->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete cloned_line;
        return (ft_sharedptr<game_dialogue_line>());
    }
    initialize_error = cloned_line->initialize(line.get_line_id(),
        line.get_speaker(), line.get_text(), line.get_next_line_ids());
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete cloned_line;
        return (ft_sharedptr<game_dialogue_line>());
    }
    return (ft_sharedptr<game_dialogue_line>(cloned_line));
}

static game_dialogue_line *game_dialogue_table_clone_line_raw(
    const game_dialogue_line &line)
{
    game_dialogue_line *cloned_line;
    int32_t initialize_error;

    if (line.is_initialised() == FT_FALSE)
        return (ft_nullptr);
    cloned_line = new (std::nothrow) game_dialogue_line();
    if (cloned_line == ft_nullptr)
        return (ft_nullptr);
    initialize_error = cloned_line->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete cloned_line;
        return (ft_nullptr);
    }
    initialize_error = cloned_line->initialize(line.get_line_id(),
        line.get_speaker(), line.get_text(), line.get_next_line_ids());
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete cloned_line;
        return (ft_nullptr);
    }
    return (cloned_line);
}

static void game_dialogue_table_clear_line_cache(
    ft_map<int32_t, game_dialogue_line *> &line_cache) noexcept
{
    const Pair<int32_t, game_dialogue_line *> *entry;
    const Pair<int32_t, game_dialogue_line *> *entry_end;
    ft_size_t count;
    ft_size_t index;

    count = line_cache.size();
    entry_end = line_cache.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        delete entry->value;
        entry++;
        index += 1;
    }
    line_cache.clear();
    return ;
}

game_dialogue_table::game_dialogue_table() noexcept
    : _lines(), _line_cache(), _scripts(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_dialogue_table::~game_dialogue_table() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return ;
    (void)this->destroy();
    return ;
}

int32_t game_dialogue_table::set_error(int32_t error_code) noexcept
{
    game_dialogue_table::_last_error = error_code;
    return (error_code);
}

int32_t game_dialogue_table::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_dialogue_table::get_error");
    return (game_dialogue_table::_last_error);
}

const char *game_dialogue_table::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_dialogue_table::get_error_str");
    return (ft_strerror(this->get_error()));
}

int32_t game_dialogue_table::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_dialogue_table::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    int32_t lines_error = this->_lines.initialize();
    if (lines_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(lines_error);
        return (lines_error);
    }
    int32_t line_cache_error = this->_line_cache.initialize();
    if (line_cache_error != FT_ERR_SUCCESS)
    {
        (void)this->_lines.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(line_cache_error);
        return (line_cache_error);
    }
    int32_t scripts_error = this->_scripts.initialize();
    if (scripts_error != FT_ERR_SUCCESS)
    {
        (void)this->_lines.destroy();
        (void)this->_line_cache.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(scripts_error);
        return (scripts_error);
    }
    this->_lines.clear();
    game_dialogue_table_clear_line_cache(this->_line_cache);
    this->_scripts.clear();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::initialize(const game_dialogue_table &other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    ft_size_t lines_count;
    ft_size_t scripts_count;
    ft_size_t index;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *line_entry;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *lines_end;
    const Pair<int32_t, game_dialogue_line *> *line_cache_entry;
    const Pair<int32_t, game_dialogue_line *> *line_cache_end;
    const Pair<int32_t, game_dialogue_script> *script_entry;
    const Pair<int32_t, game_dialogue_script> *scripts_end;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_dialogue_table::initialize(copy)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
        {
            this->set_error(destroy_error);
            return (destroy_error);
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(static_cast<uint32_t>(other.get_error()));
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->set_error(initialize_error);
            return (initialize_error);
        }
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    lines_count = other._lines.size();
    lines_end = other._lines.end();
    line_entry = lines_end - lines_count;
    index = 0;
    while (index < lines_count)
    {
        this->_lines.insert(line_entry->key, line_entry->value);
        line_entry++;
        index += 1;
    }
    lines_count = other._line_cache.size();
    line_cache_end = other._line_cache.end();
    line_cache_entry = line_cache_end - lines_count;
    index = 0;
    while (index < lines_count)
    {
        this->_line_cache.insert(line_cache_entry->key,
            game_dialogue_table_clone_line_raw(*line_cache_entry->value));
        line_cache_entry++;
        index += 1;
    }
    scripts_count = other._scripts.size();
    scripts_end = other._scripts.end();
    script_entry = scripts_end - scripts_count;
    index = 0;
    while (index < scripts_count)
    {
        this->_scripts.insert(script_entry->key, script_entry->value);
        script_entry++;
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::initialize(game_dialogue_table &&other) noexcept
{
    return (this->move(other));
}

int32_t game_dialogue_table::move(game_dialogue_table &other) noexcept
{
    int32_t initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_dialogue_table::move",
            "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    initialize_error = this->initialize(static_cast<const game_dialogue_table &>(other));
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    if (other._initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_lines.clear();
    game_dialogue_table_clear_line_cache(this->_line_cache);
    this->_scripts.clear();
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_dialogue_table::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::enable_thread_safety");
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

int32_t game_dialogue_table::disable_thread_safety() noexcept
{
    int32_t destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_dialogue_table::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_dialogue_table::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::lock");
    return (this->lock_internal(lock_acquired));
}

void game_dialogue_table::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_dialogue_table::register_line(const game_dialogue_line &line) noexcept
{
    ft_bool lock_acquired;
    ft_sharedptr<game_dialogue_line> stored_line;
    game_dialogue_line *cached_line;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::register_line");
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (line.is_initialised() == FT_FALSE)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    stored_line = game_dialogue_table_clone_line(line);
    if (stored_line == ft_sharedptr<game_dialogue_line>())
    {

        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    cached_line = game_dialogue_table_clone_line_raw(line);
    if (cached_line == ft_nullptr)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    this->_lines.insert(line.get_line_id(), ft_move(stored_line));
    this->_line_cache.insert(line.get_line_id(), cached_line);
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::register_script(const game_dialogue_script &script) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::register_script");
    int32_t lock_error;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_scripts.insert(script.get_script_id(), script);
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::fetch_line(int32_t line_id,
    game_dialogue_line &out_line) const noexcept
{
    const Pair<int32_t, game_dialogue_line *> *entry;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *line_entry;
    const game_dialogue_line *source_line;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t output_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::fetch_line");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    source_line = ft_nullptr;
    entry = this->_line_cache.find(line_id);
    if (entry != this->_line_cache.end()
        && entry->value != ft_nullptr
        && entry->value->is_initialised() == FT_TRUE)
    {
        source_line = entry->value;
    }
    else
    {
        line_entry = this->_lines.find(line_id);
        if (line_entry == this->_lines.end()
            || line_entry->value == ft_sharedptr<game_dialogue_line>()
            || line_entry->value->is_initialised() == FT_FALSE)
        {
            this->set_error(FT_ERR_NOT_FOUND);
            (void)this->unlock_internal(lock_acquired);
            return (FT_ERR_NOT_FOUND);
        }
        source_line = line_entry->value.get();
    }
    if (source_line == ft_nullptr || source_line->is_initialised() == FT_FALSE)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        (void)this->unlock_internal(lock_acquired);
        return (FT_ERR_INVALID_STATE);
    }
    output_error = out_line.initialize(source_line->get_line_id(),
        source_line->get_speaker(), source_line->get_text(),
        source_line->get_next_line_ids());
    if (output_error != FT_ERR_SUCCESS)
    {
        this->set_error(output_error);
        (void)this->unlock_internal(lock_acquired);
        return (output_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::fetch_script(int32_t script_id,
    game_dialogue_script &out_script) const noexcept
{
    const Pair<int32_t, game_dialogue_script> *entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::fetch_script");
    ft_bool lock_acquired;
    int32_t lock_error;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    entry = this->_scripts.find(script_id);
    if (entry == this->_scripts.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);

        (void)this->unlock_internal(lock_acquired);
        return (FT_ERR_NOT_FOUND);
    }
    int32_t initialize_error = out_script.initialize(entry->value);
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);

        (void)this->unlock_internal(lock_acquired);
        return (initialize_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

ft_map<int32_t, ft_sharedptr<game_dialogue_line> > &game_dialogue_table::get_lines() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::get_lines");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_lines);
}

const ft_map<int32_t, ft_sharedptr<game_dialogue_line> > &game_dialogue_table::get_lines() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::get_lines const");
    const_cast<game_dialogue_table *>(this)->set_error(FT_ERR_SUCCESS);
    return (this->_lines);
}

void game_dialogue_table::set_lines(
    const ft_map<int32_t, ft_sharedptr<game_dialogue_line> > &lines) noexcept
{
    ft_bool lock_acquired;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *entry;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *entry_end;
    game_dialogue_line *cached_line;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::set_lines");
    int32_t lock_error;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_lines.clear();
    game_dialogue_table_clear_line_cache(this->_line_cache);
    if (lines.size() == 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        (void)this->unlock_internal(lock_acquired);
        return ;
    }
    count = lines.size();
    entry_end = lines.end();
    entry = entry_end - count;
    index = 0;
    error_code = FT_ERR_SUCCESS;
    while (index < count)
    {
        this->_lines.insert(entry->key, entry->value);
        if (this->_lines.get_error() != FT_ERR_SUCCESS && error_code == FT_ERR_SUCCESS)
            error_code = this->_lines.get_error();
        if (entry->value != ft_sharedptr<game_dialogue_line>()
            && entry->value->is_initialised() == FT_TRUE)
        {
            cached_line = game_dialogue_table_clone_line_raw(*entry->value.get());
            if (cached_line != ft_nullptr)
            {
                this->_line_cache.insert(entry->key, cached_line);
                if (this->_line_cache.get_error() != FT_ERR_SUCCESS
                    && error_code == FT_ERR_SUCCESS)
                {
                    error_code = this->_line_cache.get_error();
                    delete cached_line;
                }
            }
            else if (error_code == FT_ERR_SUCCESS)
                error_code = FT_ERR_NO_MEMORY;
        }
        entry++;
        index += 1;
    }
    this->set_error(static_cast<uint32_t>(error_code));

    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int32_t, game_dialogue_script> &game_dialogue_table::get_scripts() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::get_scripts");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_scripts);
}

const ft_map<int32_t, game_dialogue_script> &game_dialogue_table::get_scripts() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::get_scripts const");
    const_cast<game_dialogue_table *>(this)->set_error(FT_ERR_SUCCESS);
    return (this->_scripts);
}

void game_dialogue_table::set_scripts(
    const ft_map<int32_t, game_dialogue_script> &scripts) noexcept
{
    ft_bool lock_acquired;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, game_dialogue_script> *entry;
    const Pair<int32_t, game_dialogue_script> *entry_end;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_table::set_scripts");
    int32_t lock_error;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_scripts.clear();
    count = scripts.size();
    entry_end = scripts.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_scripts.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return ;
}
