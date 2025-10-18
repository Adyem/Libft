# Security Threat Models

This document outlines the primary threat scenarios for the Networking, Storage, and Encryption modules. Each section describes the attack surface, enumerates common risks, and recommends mitigations that developers should adopt when integrating these components.

## Networking Module

### Attack Surface
- HTTP client and server helpers in `Networking/http_client.cpp`, `Networking/http_server.cpp`, and related transports handle untrusted network inputs.
- WebSocket and socket primitives expose raw frame parsing code paths that interact directly with OS sockets.
- TLS wrappers in `Networking/api_tls_client.cpp` and `Networking/networking_tls.cpp` manage certificate stores, session tickets, and private keys.

### Threats and Mitigations
- **Input validation failures**: Always run received payloads through strict parsers (`http_headers_validate`, `ws_validate_frame`) and enforce limits from `networking_config.hpp` to prevent buffer exhaustion.
- **Transport downgrade**: When calling `ft_socket::connect` or the higher-level HTTP helpers, require TLS by default and enable ALPN negotiation so HTTP/2 and HTTP/3 selections cannot be forced back to plaintext.
- **Credential leakage**: Load certificates via the CMA-backed file helpers so sensitive keys remain in guarded allocations. Combine with `cma_alloc_limit_guard` to constrain attack impact.
- **Resource exhaustion**: Use the event loop backpressure metrics and asynchronous logger counters to detect queue growth. Configure keep-alive and connection pooling to prevent unbounded socket creation.
- **Cross-request contamination**: Assign a fresh `ft_logger` context per connection and sanitize propagated headers before forwarding requests downstream.

## Storage Module

### Attack Surface
- The key-value store in `Storage/kv_store.hpp` persists user-controlled blobs and metadata.
- Optional encryption layers in `storage_kv_store_encryption.cpp` protect serialized values, while operations in `storage_kv_store_operations.cpp` mediate concurrent reads and writes.

### Threats and Mitigations
- **Tampering and replay**: Sign serialized payloads with HMACs before calling `kv_store::put`, and store monotonic sequence numbers alongside values to detect replayed writes.
- **Unauthorized access**: Enforce access-control checks in the application layer before exposing `kv_store` handles. Use process-level sandboxing from the System utilities to restrict file-system reach.
- **Corrupted persistence**: Validate deserialized objects using checksum fields prior to returning from `kv_store::get`. Maintain periodic snapshots and fsync checkpoints to ensure crash consistency.
- **Key management drift**: Rotate encryption keys by wiring `kv_store::reencrypt` into deployment pipelines, ensuring the encryption helpers rewrap values without exposing plaintext to disk.

## Encryption Module

### Attack Surface
- Hashing, symmetric encryption, and HMAC utilities in `Encryption/basic_encryption.hpp` process raw key material.
- Randomness helpers in `RNG/rng.cpp` seed cryptographic operations and session identifiers.

### Threats and Mitigations
- **Weak randomness**: Seed RNG utilities from system entropy sources (`rng_seed_system`) and monitor failure codes surfaced through `_error_code` mirrors.
- **Key exposure**: Use the CMA allocation guards for key buffers so `cma_secure_zero` wipes
  memory immediately after use. Avoid logging secrets by default; when `_api_logging` is
  required, register redaction patterns with `ft_log_add_redaction` before emitting
  diagnostics.
- **Algorithm downgrade**: Prefer modern primitives (e.g., SHA-2, AES-GCM). When legacy compatibility is unavoidable, gate weaker modes behind explicit feature flags.
- **Implementation misuse**: Follow the secure coding checklist to ensure nonces are unique, authentication tags are verified, and error codes are handled before proceeding with sensitive operations.

## Operational Practices

- Schedule recurring reviews of this threat model alongside incident postmortems.
- Integrate the recommendations into automated linting or CI checks where possible (e.g., verifying TLS is enabled for production builds).
- Capture deviations in architecture decision records so compensating controls are documented.
