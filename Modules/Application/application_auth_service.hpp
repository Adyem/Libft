#ifndef APPLICATION_AUTH_SERVICE_HPP
#define APPLICATION_AUTH_SERVICE_HPP

#include "../Errno/errno.hpp"
#include "../Filesystem/filesystem.hpp"
#include "../Storage/kv_store.hpp"
#include "../Encryption/encryption.hpp"
#include "../Encoding/encoding.hpp"

class application_auth_service
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        uint8_t _initialised_state;
        kv_store _credential_store;
        ft_string _database_root_path;
        ft_string _database_relative_path;
        ft_string _database_path;
        ft_string _encryption_key;
        ft_bool _encryption_enabled;
        ft_bool _manual_login_approval_enabled;

        int32_t initialize_from_copy(const application_auth_service &other) noexcept;
        int32_t initialize_from_move(application_auth_service &other) noexcept;
        int32_t build_user_key(const char *username, ft_string &key_output) const noexcept;
        int32_t build_login_approval_key(const char *username, ft_string &key_output) const noexcept;
        int32_t build_user_approval_requirement_key(const char *username, ft_string &key_output) const noexcept;
        int32_t build_manual_login_approval_setting_key(ft_string &key_output) const noexcept;
        int32_t generate_salt_hex(ft_string &salt_hex) const noexcept;
        int32_t build_password_hash(const char *password, const ft_string &salt_hex, ft_string &hash_hex) const noexcept;
        int32_t build_credential_record(const char *password, ft_string &record_output) const noexcept;
        int32_t parse_credential_record(const char *record_string, ft_string &salt_hex, ft_string &hash_hex) const noexcept;
        int32_t load_manual_login_approval_enabled(ft_bool &enabled) const noexcept;
        int32_t load_user_login_approval_required(const char *username, ft_bool &required) const noexcept;

    public:
        application_auth_service() noexcept;
        application_auth_service(const application_auth_service &other) noexcept = delete;
        application_auth_service(application_auth_service &&other) noexcept = delete;
        ~application_auth_service() noexcept;

        application_auth_service &operator=(const application_auth_service &other) noexcept = delete;
        application_auth_service &operator=(application_auth_service &&other) noexcept = delete;

        int32_t initialize(const char *database_root_path, const char *database_relative_path, const char *encryption_key = ft_nullptr, ft_bool enable_encryption = FT_FALSE) noexcept;
        int32_t initialize(const application_auth_service &other) noexcept;
        int32_t initialize(application_auth_service &&other) noexcept;
        int32_t destroy() noexcept;
        int32_t move(application_auth_service &other) noexcept;

        ft_bool is_initialised() const noexcept;

        int32_t register_user(const char *username, const char *password) noexcept;
        int32_t authenticate_user(const char *username, const char *password, ft_bool &authenticated) const noexcept;
        int32_t user_exists(const char *username, ft_bool &exists) const noexcept;
        int32_t remove_user(const char *username) noexcept;
        int32_t set_manual_login_approval_enabled(ft_bool enabled) noexcept;
        int32_t is_manual_login_approval_enabled(ft_bool &enabled) const noexcept;
        int32_t set_login_approval_required_for_user(const char *username, ft_bool enabled) noexcept;
        int32_t is_login_approval_required_for_user(const char *username, ft_bool &enabled) const noexcept;
        int32_t approve_login(const char *username) noexcept;
        int32_t revoke_login_approval(const char *username) noexcept;
        int32_t is_login_approved(const char *username, ft_bool &approved) const noexcept;
};

#endif
