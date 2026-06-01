#include "application_auth_service.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno_internal.hpp"
#include "../System_utils/system_utils.hpp"
#include <unistd.h>

static const char *application_auth_user_key_prefix = "application/auth/users/";
static const char *application_auth_login_approval_key_prefix = "application/auth/approvals/";
static const char *application_auth_login_approval_requirement_key_prefix = "application/auth/settings/approval_required/";
static const char *application_auth_manual_login_approval_setting_key = "application/auth/settings/manual_login_approval";
static const char *application_auth_login_signal_one_time_password_key_prefix = "application/auth/login_signal/";
static const ft_size_t application_auth_salt_length = 16U;
static const ft_size_t application_auth_digest_length = 32U;
static const ft_size_t application_auth_login_signal_one_time_password_length = 16U;
static const int64_t application_auth_login_signal_default_timeout_seconds = 300LL;

static int32_t application_auth_encode_hex(const uint8_t *buffer, ft_size_t buffer_size, ft_string &output)
{
    char *encoded_buffer;
    int32_t error_code;

    encoded_buffer = encoding_hex_encode(buffer, buffer_size, FT_FALSE);
    if (encoded_buffer == ft_nullptr)
        return (encoding_get_error());
    (void)output.destroy();
    error_code = output.initialize(encoded_buffer);
    cma_free(encoded_buffer);
    return (error_code);
}

static int32_t application_auth_build_database_path(const char *database_root_path, const char *database_relative_path, ft_string &database_path)
{
    ft_string *joined_path;
    int32_t error_code;

    joined_path = filesystem_safe_join_path(database_root_path, database_relative_path);
    if (joined_path == ft_nullptr)
        return (FT_ERR_INVALID_PATH);
    (void)database_path.destroy();
    error_code = database_path.initialize(joined_path->c_str());
    (void)joined_path->destroy();
    delete joined_path;
    return (error_code);
}

static int32_t application_auth_write_descriptor(int32_t file_descriptor, const char *buffer, ft_size_t length)
{
    int64_t write_result;

    write_result = su_write(file_descriptor, buffer, length);
    if (write_result < 0)
        return (FT_ERR_IO);
    if (write_result != static_cast<int64_t>(length))
        return (FT_ERR_IO);
    return (FT_ERR_SUCCESS);
}

application_auth_service::application_auth_service() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _credential_store()
    , _settings()
    , _database_path()
{
    return ;
}

application_auth_service::~application_auth_service() noexcept
{
    (void)this->destroy();
    return ;
}

ft_bool application_auth_service::is_initialised() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        return (FT_TRUE);
    return (FT_FALSE);
}

int32_t application_auth_service::destroy() noexcept
{
    int32_t error_code;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    error_code = this->_credential_store.destroy();
    if (this->_settings.destroy() != FT_ERR_SUCCESS && error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_INVALID_OPERATION;
    if (this->_database_path.destroy() != FT_ERR_SUCCESS && error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_INVALID_OPERATION;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (error_code);
}

int32_t application_auth_service::initialize(const application_settings &settings) noexcept
{
    ft_string database_root_path;
    ft_string database_relative_path;
    ft_string encryption_key;
    ft_string encryption_algorithm_name;
    int32_t error_code;
    ft_bool encryption_enabled;
    ft_bool manual_login_approval_enabled;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "application_auth_service::initialize", "initialize called on initialised instance");
    if (settings.is_initialised() != FT_TRUE)
        errno_abort_lifecycle(FT_CLASS_STATE_UNINITIALISED, "application_auth_service::initialize(settings)", "source settings are uninitialised");
    error_code = settings.get_database_root_path(database_root_path);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = settings.get_database_relative_path(database_relative_path);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = settings.get_encryption_key(encryption_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = settings.get_encryption_algorithm_name(encryption_algorithm_name);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = settings.is_encryption_enabled(encryption_enabled);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = application_auth_build_database_path(database_root_path.c_str(), database_relative_path.c_str(), this->_database_path);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    error_code = this->_credential_store.initialize(this->_database_path.c_str(),
        encryption_key.c_str(), encryption_enabled, encryption_algorithm_name.c_str());
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    error_code = this->_settings.initialize(settings);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    error_code = this->load_manual_login_approval_enabled(manual_login_approval_enabled);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    error_code = this->_settings.set_manual_login_approval_enabled(manual_login_approval_enabled);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::initialize(const char *database_root_path, const char *database_relative_path,
    const char *encryption_key, ft_bool enable_encryption, const char *encryption_algorithm_name) noexcept
{
    application_settings settings;
    int32_t error_code;

    error_code = settings.initialize(database_root_path, database_relative_path, encryption_key,
        enable_encryption, encryption_algorithm_name);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (this->initialize(settings));
}

int32_t application_auth_service::initialize_from_copy(const application_auth_service &other) noexcept
{
    int32_t error_code;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state, "application_auth_service::initialize(copy)", "source is uninitialised");
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        error_code = this->destroy();
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        error_code = this->destroy();
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
    }
    error_code = this->_settings.initialize(other._settings);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    error_code = this->_database_path.initialize(other._database_path);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    error_code = this->_credential_store.initialize(other._credential_store);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::initialize_from_move(application_auth_service &other) noexcept
{
    int32_t error_code;

    error_code = this->initialize_from_copy(other);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = other.destroy();
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (error_code);
    }
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::initialize(const application_auth_service &other) noexcept
{
    return (this->initialize_from_copy(other));
}

int32_t application_auth_service::initialize(application_auth_service &&other) noexcept
{
    return (this->initialize_from_move(other));
}

int32_t application_auth_service::move(application_auth_service &other) noexcept
{
    return (this->initialize_from_move(other));
}

int32_t application_auth_service::build_user_key(const char *username, ft_string &key_output) const noexcept
{
    int32_t error_code;

    if (username == ft_nullptr || username[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    (void)key_output.destroy();
    error_code = key_output.initialize(application_auth_user_key_prefix);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = key_output.append(username);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)key_output.destroy();
        return (error_code);
    }
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::build_login_approval_key(const char *username, ft_string &key_output) const noexcept
{
    int32_t error_code;

    if (username == ft_nullptr || username[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    (void)key_output.destroy();
    error_code = key_output.initialize(application_auth_login_approval_key_prefix);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = key_output.append(username);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)key_output.destroy();
        return (error_code);
    }
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::build_user_approval_requirement_key(const char *username, ft_string &key_output) const noexcept
{
    int32_t error_code;

    if (username == ft_nullptr || username[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    (void)key_output.destroy();
    error_code = key_output.initialize(application_auth_login_approval_requirement_key_prefix);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = key_output.append(username);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)key_output.destroy();
        return (error_code);
    }
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::build_manual_login_approval_setting_key(ft_string &key_output) const noexcept
{
    int32_t error_code;

    (void)key_output.destroy();
    error_code = key_output.initialize(application_auth_manual_login_approval_setting_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::build_login_signal_one_time_password_key(const char *username, ft_string &key_output) const noexcept
{
    int32_t error_code;

    if (username == ft_nullptr || username[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    (void)key_output.destroy();
    error_code = key_output.initialize(application_auth_login_signal_one_time_password_key_prefix);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = key_output.append(username);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)key_output.destroy();
        return (error_code);
    }
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::generate_salt_hex(ft_string &salt_hex) const noexcept
{
    uint8_t salt_buffer[application_auth_salt_length];
    int32_t error_code;

    error_code = encryption_fill_secure_buffer(salt_buffer, application_auth_salt_length);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (application_auth_encode_hex(salt_buffer, application_auth_salt_length, salt_hex));
}

int32_t application_auth_service::generate_login_signal_one_time_password(ft_string &one_time_password_output) const noexcept
{
    uint8_t password_buffer[application_auth_login_signal_one_time_password_length];
    int32_t error_code;

    error_code = encryption_fill_secure_buffer(password_buffer, application_auth_login_signal_one_time_password_length);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (application_auth_encode_hex(password_buffer, application_auth_login_signal_one_time_password_length, one_time_password_output));
}

int32_t application_auth_service::build_login_signal_one_time_password_hash(const char *one_time_password, ft_string &hash_hex) const noexcept
{
    uint8_t digest_buffer[application_auth_digest_length];

    if (one_time_password == ft_nullptr || one_time_password[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    sha256_hash(one_time_password, ft_strlen(one_time_password), digest_buffer);
    return (application_auth_encode_hex(digest_buffer, application_auth_digest_length, hash_hex));
}

int32_t application_auth_service::write_login_signal_one_time_password(const char *username, const char *one_time_password) const noexcept
{
    ft_string message;
    int32_t error_code;

    if (username == ft_nullptr || one_time_password == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    error_code = message.initialize(username);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = message.append(": ");
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)message.destroy();
        return (error_code);
    }
    error_code = message.append(one_time_password);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)message.destroy();
        return (error_code);
    }
    error_code = message.append('\n');
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)message.destroy();
        return (error_code);
    }
    {
        int32_t login_signal_output_file_descriptor;

        error_code = this->_settings.get_login_signal_output_file_descriptor(login_signal_output_file_descriptor);
        if (error_code != FT_ERR_SUCCESS)
        {
            (void)message.destroy();
            return (error_code);
        }
        error_code = application_auth_write_descriptor(login_signal_output_file_descriptor,
            message.c_str(), message.size());
    }
    (void)message.destroy();
    return (error_code);
}

int32_t application_auth_service::build_password_hash(const char *password, const ft_string &salt_hex, ft_string &hash_hex) const noexcept
{
    ft_string hash_input;
    uint8_t digest_buffer[application_auth_digest_length];
    int32_t error_code;

    if (password == ft_nullptr || password[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    error_code = hash_input.initialize(salt_hex);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = hash_input.append(':');
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = hash_input.append(password);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    sha256_hash(hash_input.c_str(), hash_input.size(), digest_buffer);
    return (application_auth_encode_hex(digest_buffer, application_auth_digest_length, hash_hex));
}

int32_t application_auth_service::build_credential_record(const char *password, ft_string &record_output) const noexcept
{
    ft_string salt_hex;
    ft_string hash_hex;
    int32_t error_code;

    error_code = this->generate_salt_hex(salt_hex);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->build_password_hash(password, salt_hex, hash_hex);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    (void)record_output.destroy();
    error_code = record_output.initialize(salt_hex);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = record_output.append('|');
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = record_output.append(hash_hex);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)record_output.destroy();
        return (error_code);
    }
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::parse_credential_record(const char *record_string, ft_string &salt_hex, ft_string &hash_hex) const noexcept
{
    ft_size_t delimiter_index;
    int32_t error_code;

    if (record_string == ft_nullptr || record_string[0] == '\0')
        return (FT_ERR_CONFIGURATION);
    delimiter_index = 0;
    while (record_string[delimiter_index] != '\0' && record_string[delimiter_index] != '|')
        delimiter_index++;
    if (delimiter_index == 0 || record_string[delimiter_index] != '|')
        return (FT_ERR_CONFIGURATION);
    (void)salt_hex.destroy();
    error_code = salt_hex.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = salt_hex.append(record_string, delimiter_index);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    (void)hash_hex.destroy();
    error_code = hash_hex.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = hash_hex.append(record_string + delimiter_index + 1);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (hash_hex.empty() == FT_TRUE)
        return (FT_ERR_CONFIGURATION);
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::load_manual_login_approval_enabled(ft_bool &enabled) const noexcept
{
    ft_string setting_key;
    const char *setting_value;
    int32_t error_code;

    error_code = this->build_manual_login_approval_setting_key(setting_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    setting_value = this->_credential_store.kv_get(setting_key.c_str());
    if (setting_value == ft_nullptr)
    {
        enabled = FT_FALSE;
        return (FT_ERR_SUCCESS);
    }
    if (ft_strcmp(setting_value, "1") == FT_ERR_SUCCESS)
    {
        enabled = FT_TRUE;
        return (FT_ERR_SUCCESS);
    }
    if (ft_strcmp(setting_value, "0") == FT_ERR_SUCCESS)
    {
        enabled = FT_FALSE;
        return (FT_ERR_SUCCESS);
    }
    return (FT_ERR_CONFIGURATION);
}

int32_t application_auth_service::load_user_login_approval_required(const char *username, ft_bool &required) const noexcept
{
    ft_string setting_key;
    const char *setting_value;
    int32_t error_code;

    error_code = this->build_user_approval_requirement_key(username, setting_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    setting_value = this->_credential_store.kv_get(setting_key.c_str());
    if (setting_value == ft_nullptr)
    {
        required = FT_FALSE;
        return (FT_ERR_SUCCESS);
    }
    if (ft_strcmp(setting_value, "1") == FT_ERR_SUCCESS)
    {
        required = FT_TRUE;
        return (FT_ERR_SUCCESS);
    }
    if (ft_strcmp(setting_value, "0") == FT_ERR_SUCCESS)
    {
        required = FT_FALSE;
        return (FT_ERR_SUCCESS);
    }
    return (FT_ERR_CONFIGURATION);
}

int32_t application_auth_service::register_user(const char *username, const char *password) noexcept
{
    ft_string user_key;
    ft_string credential_record;
    const char *existing_record;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::register_user");
    error_code = this->build_user_key(username, user_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    existing_record = this->_credential_store.kv_get(user_key.c_str());
    if (existing_record != ft_nullptr)
        return (FT_ERR_ALREADY_EXISTS);
    error_code = this->build_credential_record(password, credential_record);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (this->_credential_store.kv_set(user_key.c_str(), credential_record.c_str()));
}

int32_t application_auth_service::authenticate_user(const char *username, const char *password, ft_bool &authenticated) const noexcept
{
    ft_string user_key;
    ft_string salt_hex;
    ft_string stored_hash_hex;
    ft_string computed_hash_hex;
    const char *record_string;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::authenticate_user");
    authenticated = FT_FALSE;
    error_code = this->build_user_key(username, user_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    record_string = this->_credential_store.kv_get(user_key.c_str());
    if (record_string == ft_nullptr)
        return (FT_ERR_NOT_FOUND);
    error_code = this->parse_credential_record(record_string, salt_hex, stored_hash_hex);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->build_password_hash(password, salt_hex, computed_hash_hex);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (ft_strcmp(stored_hash_hex.c_str(), computed_hash_hex.c_str()) != 0)
        return (FT_ERR_PERMISSION_DENIED);
    {
        ft_bool manual_login_approval_enabled;

        error_code = this->_settings.is_manual_login_approval_enabled(manual_login_approval_enabled);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        if (manual_login_approval_enabled == FT_TRUE)
        {
            ft_bool approved;

            approved = FT_FALSE;
            error_code = this->is_login_approved(username, approved);
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
            if (approved == FT_FALSE)
                return (FT_ERR_PERMISSION_DENIED);
        }
        else
        {
            ft_bool approval_required;
            ft_bool approved;

            approval_required = FT_FALSE;
            error_code = this->load_user_login_approval_required(username, approval_required);
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
            if (approval_required == FT_TRUE)
            {
                approved = FT_FALSE;
                error_code = this->is_login_approved(username, approved);
                if (error_code != FT_ERR_SUCCESS)
                    return (error_code);
                if (approved == FT_FALSE)
                    return (FT_ERR_PERMISSION_DENIED);
            }
        }
    }
    authenticated = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::user_exists(const char *username, ft_bool &exists) const noexcept
{
    ft_string user_key;
    const char *record_string;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::user_exists");
    error_code = this->build_user_key(username, user_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    record_string = this->_credential_store.kv_get(user_key.c_str());
    if (record_string != ft_nullptr)
        exists = FT_TRUE;
    else
        exists = FT_FALSE;
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::remove_user(const char *username) noexcept
{
    ft_string user_key;
    ft_string approval_key;
    ft_string approval_requirement_key;
    ft_string login_signal_key;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::remove_user");
    error_code = this->build_user_key(username, user_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->build_login_approval_key(username, approval_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->build_user_approval_requirement_key(username, approval_requirement_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->build_login_signal_one_time_password_key(username, login_signal_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    (void)this->_credential_store.kv_delete(approval_key.c_str());
    (void)this->_credential_store.kv_delete(approval_requirement_key.c_str());
    (void)this->_credential_store.kv_delete(login_signal_key.c_str());
    return (this->_credential_store.kv_delete(user_key.c_str()));
}

int32_t application_auth_service::set_manual_login_approval_enabled(ft_bool enabled) noexcept
{
    ft_string setting_key;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::set_manual_login_approval_enabled");
    error_code = this->build_manual_login_approval_setting_key(setting_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (enabled == FT_TRUE)
        error_code = this->_credential_store.kv_set(setting_key.c_str(), "1");
    else
        error_code = this->_credential_store.kv_set(setting_key.c_str(), "0");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->_settings.set_manual_login_approval_enabled(enabled);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::is_manual_login_approval_enabled(ft_bool &enabled) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::is_manual_login_approval_enabled");
    return (this->_settings.is_manual_login_approval_enabled(enabled));
}

int32_t application_auth_service::set_login_approval_required_for_user(const char *username, ft_bool enabled) noexcept
{
    ft_string user_key;
    ft_string approval_requirement_key;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::set_login_approval_required_for_user");
    error_code = this->build_user_key(username, user_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (this->_credential_store.kv_get(user_key.c_str()) == ft_nullptr)
        return (FT_ERR_NOT_FOUND);
    error_code = this->build_user_approval_requirement_key(username, approval_requirement_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (enabled == FT_TRUE)
        error_code = this->_credential_store.kv_set(approval_requirement_key.c_str(), "1");
    else
        error_code = this->_credential_store.kv_delete(approval_requirement_key.c_str());
    if (error_code == FT_ERR_NOT_FOUND)
        return (FT_ERR_SUCCESS);
    return (error_code);
}

int32_t application_auth_service::is_login_approval_required_for_user(const char *username, ft_bool &enabled) const noexcept
{
    ft_string approval_requirement_key;
    const char *approval_requirement_value;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::is_login_approval_required_for_user");
    error_code = this->build_user_approval_requirement_key(username, approval_requirement_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    approval_requirement_value = this->_credential_store.kv_get(approval_requirement_key.c_str());
    if (approval_requirement_value == ft_nullptr)
    {
        enabled = FT_FALSE;
        return (FT_ERR_SUCCESS);
    }
    if (ft_strcmp(approval_requirement_value, "1") == FT_ERR_SUCCESS)
    {
        enabled = FT_TRUE;
        return (FT_ERR_SUCCESS);
    }
    if (ft_strcmp(approval_requirement_value, "0") == FT_ERR_SUCCESS)
    {
        enabled = FT_FALSE;
        return (FT_ERR_SUCCESS);
    }
    return (FT_ERR_CONFIGURATION);
}

int32_t application_auth_service::approve_login(const char *username) noexcept
{
    ft_string user_key;
    ft_string approval_key;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::approve_login");
    if (username == ft_nullptr || username[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    error_code = this->build_user_key(username, user_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (this->_credential_store.kv_get(user_key.c_str()) == ft_nullptr)
        return (FT_ERR_NOT_FOUND);
    error_code = this->build_login_approval_key(username, approval_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (this->_credential_store.kv_set(approval_key.c_str(), "1"));
}

int32_t application_auth_service::revoke_login_approval(const char *username) noexcept
{
    ft_string approval_key;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::revoke_login_approval");
    error_code = this->build_login_approval_key(username, approval_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->_credential_store.kv_delete(approval_key.c_str());
    if (error_code == FT_ERR_NOT_FOUND)
        return (FT_ERR_SUCCESS);
    return (error_code);
}

int32_t application_auth_service::is_login_approved(const char *username, ft_bool &approved) const noexcept
{
    ft_string approval_key;
    const char *approval_value;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::is_login_approved");
    error_code = this->build_login_approval_key(username, approval_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    approval_value = this->_credential_store.kv_get(approval_key.c_str());
    if (approval_value == ft_nullptr)
    {
        approved = FT_FALSE;
        return (FT_ERR_SUCCESS);
    }
    approved = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::set_login_signal_output_file_descriptor(int32_t file_descriptor) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::set_login_signal_output_file_descriptor");
    return (this->_settings.set_login_signal_output_file_descriptor(file_descriptor));
}

int32_t application_auth_service::get_login_signal_output_file_descriptor(int32_t &file_descriptor) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::get_login_signal_output_file_descriptor");
    return (this->_settings.get_login_signal_output_file_descriptor(file_descriptor));
}

int32_t application_auth_service::set_login_signal_token_timeout_seconds(int64_t timeout_seconds) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::set_login_signal_token_timeout_seconds");
    return (this->_settings.set_login_signal_token_timeout_seconds(timeout_seconds));
}

int32_t application_auth_service::get_login_signal_token_timeout_seconds(int64_t &timeout_seconds) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::get_login_signal_token_timeout_seconds");
    return (this->_settings.get_login_signal_token_timeout_seconds(timeout_seconds));
}

int32_t application_auth_service::request_login_signal_one_time_password(const char *username) noexcept
{
    ft_string user_key;
    ft_string login_signal_key;
    ft_string one_time_password;
    ft_string one_time_password_hash;
    const char *existing_record;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::request_login_signal_one_time_password");
    error_code = this->build_user_key(username, user_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    existing_record = this->_credential_store.kv_get(user_key.c_str());
    if (existing_record == ft_nullptr)
        return (FT_ERR_NOT_FOUND);
    error_code = this->build_login_signal_one_time_password_key(username, login_signal_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->generate_login_signal_one_time_password(one_time_password);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->build_login_signal_one_time_password_hash(one_time_password.c_str(), one_time_password_hash);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    {
        int64_t login_signal_token_timeout_seconds;

        error_code = this->_settings.get_login_signal_token_timeout_seconds(login_signal_token_timeout_seconds);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = this->_credential_store.kv_set(login_signal_key.c_str(), one_time_password_hash.c_str(),
            login_signal_token_timeout_seconds);
    }
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->write_login_signal_one_time_password(username, one_time_password.c_str());
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)this->_credential_store.kv_delete(login_signal_key.c_str());
        return (error_code);
    }
    return (FT_ERR_SUCCESS);
}

int32_t application_auth_service::authenticate_login_signal_one_time_password(const char *username, const char *one_time_password, ft_bool &authenticated) noexcept
{
    ft_string login_signal_key;
    ft_string provided_hash;
    const char *stored_hash;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "application_auth_service::authenticate_login_signal_one_time_password");
    authenticated = FT_FALSE;
    error_code = this->build_login_signal_one_time_password_key(username, login_signal_key);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    stored_hash = this->_credential_store.kv_get(login_signal_key.c_str());
    if (stored_hash == ft_nullptr)
        return (FT_ERR_NOT_FOUND);
    error_code = this->build_login_signal_one_time_password_hash(one_time_password, provided_hash);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (ft_strcmp(stored_hash, provided_hash.c_str()) != FT_ERR_SUCCESS)
        return (FT_ERR_PERMISSION_DENIED);
    error_code = this->_credential_store.kv_delete(login_signal_key.c_str());
    if (error_code != FT_ERR_SUCCESS && error_code != FT_ERR_NOT_FOUND)
        return (error_code);
    authenticated = FT_TRUE;
    return (FT_ERR_SUCCESS);
}
