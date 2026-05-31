# Application

The `Application` module is the first top-level application-services layer in FullLibft.
It is intended for login/auth-style workflows that combine storage, encryption, and later
service-specific behavior without forcing higher layers to reassemble the same plumbing.

## `application_auth_service`

- `application_auth_service` - Lifecycle service that manages a credential database backed by `kv_store`.
- Lifecycle methods - `initialize`, copy/move initialization, `destroy`, and `move`.
- `is_initialised()` - Reports whether the service has been initialized.
- `register_user(...)` - Creates a salted password record for a new username.
- `authenticate_user(...)` - Verifies a username/password pair against the stored record.
- `user_exists(...)` - Checks whether a username already exists.
- `remove_user(...)` - Deletes a stored user record.
- `set_login_approval_required_for_user(...)` / `is_login_approval_required_for_user(...)` - Enables or disables manual approval for a specific user.
- `set_manual_login_approval_enabled(...)` / `is_manual_login_approval_enabled(...)` - Enables or disables the global approval override for all logins.
- `approve_login(...)` / `revoke_login_approval(...)` - Marks a user as approved or removes that approval.
- `is_login_approved(...)` - Reads whether a user currently has manual approval.

## Storage format

The starter implementation stores each account as a salted SHA-256 record in `kv_store`.
Manual approval state is persisted alongside the user records so approval can be enabled
per user, and a separate global override can force approval for every login when needed.
The store itself can be encrypted through the existing `Storage` module configuration.
This is intentionally small and easy to extend when the module grows to cover sessions,
roles, password resets, audit logging, or external identity providers.
