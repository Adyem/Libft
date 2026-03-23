#include "../PThread/pthread_internal.hpp"
#include "game_dialogue_line.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

static void game_dialogue_copy_int_vector(const ft_vector<int32_t> &source,
    ft_vector<int32_t> &destination)
{
    ft_vector<int32_t>::const_iterator entry;
    ft_vector<int32_t>::const_iterator end;

    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

thread_local int32_t game_dialogue_line::_last_error = FT_ERR_SUCCESS;

int32_t game_dialogue_line::set_error(int32_t error_code) noexcept
{
    game_dialogue_line::_last_error = error_code;
    return (error_code);
}

game_dialogue_line::game_dialogue_line() noexcept
    : _line_id(0), _speaker(), _text(), _next_line_ids(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_dialogue_line::game_dialogue_line(const game_dialogue_line &other) noexcept
    : _line_id(0), _speaker(), _text(), _next_line_ids(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;
    int32_t setup_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "game_dialogue_line::game_dialogue_line(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return ;
    }
    setup_error = this->initialize(other._line_id, other._speaker, other._text,
        other._next_line_ids);
    if (setup_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(setup_error);
        return ;
    }
    this->set_error(other.get_error());
    return ;
}

game_dialogue_line::game_dialogue_line(game_dialogue_line &&other) noexcept
    : _line_id(0), _speaker(), _text(), _next_line_ids(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "game_dialogue_line::game_dialogue_line(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
    }
    return ;
}

game_dialogue_line::~game_dialogue_line() noexcept
{
    (void)this->destroy();
    return ;
}

int32_t game_dialogue_line::initialize() noexcept
{
    int32_t speaker_initialize_error;
    int32_t text_initialize_error;
    int32_t next_line_ids_initialize_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_dialogue_line::initialize", "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_speaker.is_initialised() == FT_FALSE)
    {
        speaker_initialize_error = this->_speaker.initialize();
        if (speaker_initialize_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(speaker_initialize_error);
            return (speaker_initialize_error);
        }
    }
    if (this->_text.is_initialised() == FT_FALSE)
    {
        text_initialize_error = this->_text.initialize();
        if (text_initialize_error != FT_ERR_SUCCESS)
        {
            (void)this->_speaker.destroy();
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(text_initialize_error);
            return (text_initialize_error);
        }
    }
    if (this->_next_line_ids.is_initialised() != FT_CLASS_STATE_INITIALISED)
    {
        next_line_ids_initialize_error = this->_next_line_ids.initialize();
        if (next_line_ids_initialize_error != FT_ERR_SUCCESS)
        {
            (void)this->_text.destroy();
            (void)this->_speaker.destroy();
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(next_line_ids_initialize_error);
            return (next_line_ids_initialize_error);
        }
    }
    this->_line_id = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_line::initialize(int32_t line_id, const ft_string &speaker,
    const ft_string &text, const ft_vector<int32_t> &next_line_ids) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::initialize(values)");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_line_id = line_id;
    this->_speaker = speaker;
    this->_text = text;
    game_dialogue_copy_int_vector(next_line_ids, this->_next_line_ids);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_line::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_line_id = 0;
    if (this->_speaker.is_initialised() == FT_TRUE)
        this->_speaker.clear();
    if (this->_text.is_initialised() == FT_TRUE)
        this->_text.clear();
    if (this->_next_line_ids.is_initialised() == FT_CLASS_STATE_INITIALISED)
        this->_next_line_ids.clear();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_dialogue_line::move(game_dialogue_line &other) noexcept
{
    int32_t destroy_error;
    int32_t initialize_error;
    int32_t source_error;
    int32_t source_destroy_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_dialogue_line::move",
            "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    destroy_error = this->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_line_id = other._line_id;
    this->_speaker = other._speaker;
    this->_text = other._text;
    game_dialogue_copy_int_vector(other._next_line_ids, this->_next_line_ids);
    source_error = other.get_error();
    source_destroy_error = other.destroy();
    if (source_destroy_error != FT_ERR_SUCCESS)
    {
        this->set_error(source_destroy_error);
        return (source_destroy_error);
    }
    this->set_error(source_error);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_line::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void game_dialogue_line::unlock_internal(ft_bool lock_acquired) const noexcept
{

    if (lock_acquired == FT_FALSE)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int32_t game_dialogue_line::get_line_id() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::get_line_id");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    result = this->_line_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

void game_dialogue_line::set_line_id(int32_t line_id) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::set_line_id");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_line_id = line_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &game_dialogue_line::get_speaker() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const ft_string *result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::get_speaker");
    result = &this->_speaker;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*result);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*result);
}

void game_dialogue_line::set_speaker(const ft_string &speaker) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::set_speaker");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_speaker = speaker;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &game_dialogue_line::get_text() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const ft_string *result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::get_text");
    result = &this->_text;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*result);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*result);
}

void game_dialogue_line::set_text(const ft_string &text) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::set_text");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_text = text;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_vector<int32_t> &game_dialogue_line::get_next_line_ids() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const ft_vector<int32_t> *result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::get_next_line_ids const");
    result = &this->_next_line_ids;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*result);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*result);
}

ft_vector<int32_t> &game_dialogue_line::get_next_line_ids() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_vector<int32_t> *result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::get_next_line_ids");
    result = &this->_next_line_ids;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*result);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*result);
}

void game_dialogue_line::set_next_line_ids(const ft_vector<int32_t> &next_line_ids) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::set_next_line_ids");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    game_dialogue_copy_int_vector(next_line_ids, this->_next_line_ids);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_dialogue_line::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::enable_thread_safety");
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

int32_t game_dialogue_line::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_dialogue_line::is_thread_safe() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_dialogue_line::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int32_t game_dialogue_line::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_dialogue_line::get_error");
    return (game_dialogue_line::_last_error);
}

const char *game_dialogue_line::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_dialogue_line::get_error_str");
    return (ft_strerror(this->get_error()));
}

ft_bool game_dialogue_line::is_initialised() const noexcept
{
    return (this->_initialised_state == FT_CLASS_STATE_INITIALISED);
}
