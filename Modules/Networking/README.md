# Networking

The `Networking` module provides portable socket wrappers, DNS resolution, event loops, UDP helpers, socket classes, HTTP/HTTPS clients and servers, WebSocket clients/servers, HTTP/2 support, QUIC experiments, and TLS/AEAD utilities.

## Low-Level Socket Wrappers

- `nw_socket`, `nw_bind`, `nw_connect`, `nw_accept`, `nw_listen`, `nw_close`, and `nw_shutdown` - Portable wrappers for core socket operations.
- `nw_send`, `nw_recv`, `nw_sendto`, and `nw_recvfrom` - Portable send/receive wrappers.
- `nw_inet_pton` - Parses textual IP addresses into network byte order.
- `nw_set_nonblocking` - Enables non-blocking mode.
- `nw_poll` - Waits on read/write descriptor sets.
- `t_nw_socket_hook` and `nw_set_socket_hook` - Test hook for socket creation.
- `networking_check_socket_after_send(...)` and `networking_check_ssl_after_send(...)` - Convert send/SSL state into project error codes.

## DNS and Event Loops

- `networking_resolved_address` - Resolved socket address plus length.
- `networking_dns_resolve(...)` - Resolves all matching addresses into a vector.
- `networking_dns_resolve_first(...)` - Resolves the first matching address.
- `networking_dns_enable_thread_safety()`, `networking_dns_disable_thread_safety()`, and `networking_dns_is_thread_safe()` - Manage resolver synchronization.
- `networking_dns_clear_cache()` and `networking_dns_set_error(...)` - Clear cache or set resolver error state.
- `event_loop` - Descriptor-set event loop with read/write arrays and optional mutex.
- `event_loop_init`, `clear`, `add_socket`, `remove_socket`, `run`, thread-safety helpers, `lock`, and `unlock` - Manage event loop state.
- UDP event-loop helpers wait for read/write readiness or perform timed receive/send through an `udp_socket`.

## Socket Configuration and Classes

- `SocketType` - Server, client, or raw socket role.
- `SocketConfig` - Lifecycle configuration with IP, port, backlog, protocol, family, reuse-address flag, non-blocking flag, timeouts, and multicast fields.
- `socket_config_prepare_thread_safety`, `teardown`, `lock`, and `unlock` - Synchronize public socket configuration structs.
- `ft_socket_handle` - Lifecycle RAII-style socket descriptor holder with open/close/reset/release behavior.
- `ft_socket` - Lifecycle socket object for configure, bind/connect/listen/accept/send/receive/close workflows.
- `udp_socket` - Lifecycle UDP socket wrapper supporting bind, sendto/recvfrom, multicast, close, and optional thread safety.

## HTTP, HTTP/2, and TLS

- `http_client.hpp` - HTTP client helpers for request/response exchange.
- `http_server.hpp` and `ft_http_server` - HTTP server lifecycle class with route/listen/stop behavior.
- `http2_header_field` - HTTP/2 header key/value pair.
- `http2_frame` - HTTP/2 frame metadata and payload container.
- `http2_settings_state` - HTTP/2 settings values and acknowledgement state.
- `http2_stream_state` - Per-stream state data.
- `http2_stream_manager` - Tracks stream ids and stream lifecycle.
- `networking_tls_aead.hpp` - TLS-related AEAD helpers when OpenSSL support is enabled.
- `ssl_wrapper.hpp` - OpenSSL wrapper functions and lifecycle helpers.
- `openssl_support.hpp` - Compile-time OpenSSL availability and includes.

## WebSocket and QUIC

- `ft_websocket_client` - Lifecycle WebSocket client with connect, send, receive, close, and optional thread safety.
- `ft_websocket_server` - Lifecycle WebSocket server with listen/accept/broadcast/client management and optional thread safety.
- `s_connection_state` - WebSocket server client connection state.
- `quic_feature_configuration` - Feature flags/settings for experimental QUIC.
- `quic_datagram_plaintext` - Plaintext datagram payload holder.
- `quic_experimental_session` - Experimental lifecycle QUIC session abstraction.
