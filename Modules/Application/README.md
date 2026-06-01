# Application

The `Application` module is the first top-level application-services layer in FullLibft.
It is intended for login/auth-style workflows that combine storage, encryption, and later
service-specific behavior without forcing higher layers to reassemble the same plumbing.

## `application_auth_service`

- `application_auth_service` - Lifecycle service that manages a credential database backed by `kv_store`.
- Lifecycle methods - `initialize`, copy/move initialization, `destroy`, and `move`.
- `is_initialised()` - Reports whether the service has been initialized.
- `initialize(const char *database_root_path, const char *database_relative_path, const char *encryption_key, ft_bool enable_encryption)` - Opens the store only when the relative database path stays inside the configured root directory.
- `register_user(...)` - Creates a salted password record for a new username.
- `authenticate_user(...)` - Verifies a username/password pair against the stored record.
- `user_exists(...)` - Checks whether a username already exists.
- `remove_user(...)` - Deletes a stored user record.
- `set_login_approval_required_for_user(...)` / `is_login_approval_required_for_user(...)` - Enables or disables manual approval for a specific user.
- `set_manual_login_approval_enabled(...)` / `is_manual_login_approval_enabled(...)` - Enables or disables the global approval override for all logins.
- `approve_login(...)` / `revoke_login_approval(...)` - Marks a user as approved or removes that approval.
- `is_login_approved(...)` - Reads whether a user currently has manual approval.

## Public Contract

| Method | Behavior | Return codes |
| --- | --- | --- |
| `initialize(database_root_path, database_relative_path, encryption_key, enable_encryption)` | Requires a non-empty root path and a non-empty relative database path. Rejects absolute paths and traversal outside the root. Creates the store path with `Filesystem::filesystem_safe_join_path(...)` before opening `kv_store`. | `FT_ERR_SUCCESS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_INVALID_PATH`, `FT_ERR_IO`, `FT_ERR_CONFIGURATION`, `FT_ERR_*` from storage or encoding helpers |
| `register_user(username, password)` | Fails if the user already exists. Stores a salted SHA-256 credential record. | `FT_ERR_SUCCESS`, `FT_ERR_ALREADY_EXISTS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage or encoding helpers |
| `authenticate_user(username, password, authenticated)` | Verifies the password first. If the user has a manual approval requirement or the global override is enabled, approval must also be present. Sets `authenticated` to `FT_FALSE` on failure. | `FT_ERR_SUCCESS`, `FT_ERR_NOT_FOUND`, `FT_ERR_PERMISSION_DENIED`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_CONFIGURATION`, `FT_ERR_*` from storage or encoding helpers |
| `user_exists(username, exists)` | Reports whether a user record exists. | `FT_ERR_SUCCESS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage or encoding helpers |
| `remove_user(username)` | Deletes the user record, the approval record, and the per-user approval requirement flag. | `FT_ERR_SUCCESS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage or encoding helpers |
| `set_login_approval_required_for_user(username, enabled)` | Enables or disables approval for one user. Requires the user to exist. | `FT_ERR_SUCCESS`, `FT_ERR_NOT_FOUND`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage helpers |
| `is_login_approval_required_for_user(username, enabled)` | Reports whether the per-user approval requirement is set. Missing keys are treated as disabled. | `FT_ERR_SUCCESS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_CONFIGURATION`, `FT_ERR_*` from storage helpers |
| `set_manual_login_approval_enabled(enabled)` | Stores the global override flag for all logins. When enabled, every login requires approval. | `FT_ERR_SUCCESS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage helpers |
| `is_manual_login_approval_enabled(enabled)` | Reports the cached global override flag. | `FT_ERR_SUCCESS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage helpers |
| `approve_login(username)` | Marks a user as approved. Requires the user to exist. | `FT_ERR_SUCCESS`, `FT_ERR_NOT_FOUND`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage helpers |
| `revoke_login_approval(username)` | Removes the stored approval record. Missing approval records are treated as success. | `FT_ERR_SUCCESS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage helpers |
| `is_login_approved(username, approved)` | Reports whether the user currently has approval. Missing keys are treated as not approved. | `FT_ERR_SUCCESS`, `FT_ERR_INVALID_ARGUMENT`, `FT_ERR_*` from storage helpers |

## Storage format

The starter implementation stores each account as a salted SHA-256 record in `kv_store`.
The database file location is selected manually, but it must be provided as a relative path
inside a caller-supplied root directory. That root check is enforced with the `Filesystem`
path helpers before the store is opened.
Manual approval state is persisted alongside the user records so approval can be enabled
per user, and a separate global override can force approval for every login when needed.
The store itself can be encrypted through the existing `Storage` module configuration.
This is intentionally small and easy to extend when the module grows to cover sessions,
roles, password resets, audit logging, or external identity providers.

## Storage Keys

- `application/auth/users/<username>` - Stored credential record for a user.
- `application/auth/approvals/<username>` - Manual approval flag for a user.
- `application/auth/settings/approval_required/<username>` - Per-user approval requirement flag.
- `application/auth/settings/manual_login_approval` - Global approval override flag.

## Approval Rules

- Per-user approval can be enabled for selected users only.
- The global override applies to every login and has priority over the per-user setting.
- When the global override is enabled, approval is required even if a user does not have a per-user requirement flag.
- Removing a user clears the stored credential, the approval record, and the per-user approval requirement flag.
