# Secure Coding Checklist

Use this checklist during design reviews, code reviews, and release readiness checks to ensure the library maintains strong security guarantees.

## General Practices

- Validate all inputs using the Libft parsing helpers before they reach lower-level modules.
- Treat every `_error_code` as actionable: propagate errors immediately and avoid silently ignoring failures.
- Prefer `cma_alloc_limit_guard` and `cma_allocation_guard` for transient buffers so allocations are tracked and freed even when exceptions occur.
- Enable `_api_logging` only in sanitized environments; use `ft_log_add_redaction`
  or `ft_log_add_redaction_with_replacement` to mask secrets before emitting
  verbose traces.
- Document threat considerations in `Docs/architecture_decision_records/` for major feature changes.

## Handling Secrets

- Allocate key material with CMA helpers and clear memory using `cma_secure_zero` once the secret leaves scope.
- Store credentials in environment variables only when `System_utils` sandboxing is active; otherwise, rely on secure storage backends.
- Never print secrets to logs or test output. Use dependency injection to provide mock credentials in unit tests.

## Networking Module

- Enforce TLS defaults in `ft_http_client`, `ft_http_server`, and WebSocket helpers. Refuse plaintext connections unless explicitly permitted.
- Verify certificates with platform trust stores and pin known-good fingerprints where possible.
- Normalize and validate headers before passing them to application handlers to prevent header injection.
- Rate-limit unauthenticated requests using the event loop utilities to mitigate brute-force attempts.

## Storage Module

- Guard write operations behind access checks and capability tokens.
- Enable encryption for `kv_store` values and rotate keys regularly using the re-encryption helpers.
- Validate checksums before accepting persisted blobs and reject stale versions using monotonic counters.

## Encryption Module

- Use authenticated encryption (e.g., AES-GCM) and verify authentication tags before consuming decrypted data.
- Generate nonces via `rng_generate_bytes` and ensure they are never reused for the same key.
- Keep algorithm selections centralized so deprecating weak primitives requires touching a single configuration point.

## Testing and Tooling

- Add regression tests whenever a security bug is fixed to prevent regressions.
- Run static analysis and sanitizers in CI builds to surface memory and undefined-behaviour issues.
- Review `.git-blame-ignore-revs` before large formatting rewrites so future audits remain focused on behavioural changes.
