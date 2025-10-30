# RAII adapter samples

This document collects concrete adapters that wrap the library classes with
familiar RAII ownership semantics. Each adapter is small enough to drop into an
application and highlights the error-handling patterns that Libft expects. They
also double as executable examples that you can lift into unit tests.

## API module: streaming request session

The `api_streaming_handler` already tracks callbacks and mutex protection. The
`api_streaming_session` wrapper below binds the handler, ensures the callbacks
reset when the scope ends, and centralises request startup logic.

```cpp
#include "API/api.hpp"

class api_streaming_session
{
    private:
        api_streaming_handler _handler;
        bool _running;

    public:
        api_streaming_session()
        {
            this->_running = false;
            return ;
        }

        ~api_streaming_session()
        {
            if (!this->_running)
                return ;
            this->_handler.reset();
            this->_running = false;
            return ;
        }

        bool start(const char *ip, uint16_t port, const char *method,
                const char *path, api_stream_headers_callback headers_callback,
                api_stream_body_callback body_callback, void *user_data,
                json_group *payload = ft_nullptr,
                const char *headers = ft_nullptr, int timeout = 60000,
                const api_retry_policy *policy = ft_nullptr)
        {
            if (this->_running)
                this->_handler.reset();
            this->_handler.set_headers_callback(headers_callback);
            this->_handler.set_body_callback(body_callback);
            this->_handler.set_user_data(user_data);
            this->_running = api_request_stream(ip, port, method, path,
                    &this->_handler, payload, headers, timeout, policy);
            if (!this->_running)
                this->_handler.reset();
            return (this->_running);
        }
};
```

Usage highlights:

- The destructor guarantees that callbacks are torn down, preventing them from
  touching stale `user_data` pointers when the surrounding scope exits.
- The `start` helper forwards retry policies and records whether the request is
  live. Callers can reuse the session instance for sequential requests.

## PThread module: scoped scheduler worker

`ft_task_scheduler` exposes explicit `start` and `shutdown` members. Wrapping
those calls in a guard removes the need for manual shutdown calls when control
flows exit early.

```cpp
#include "PThread/task_scheduler.hpp"

class scoped_task_scheduler
{
    private:
        ft_task_scheduler _scheduler;
        bool _started;

    public:
        scoped_task_scheduler()
            : _scheduler(0), _started(false)
        {
            return ;
        }

        ~scoped_task_scheduler()
        {
            if (!this->_started)
                return ;
            this->_scheduler.shutdown();
            return ;
        }

        bool start(size_t thread_count)
        {
            if (this->_started)
                return (true);
            if (!this->_scheduler.start(thread_count))
                return (false);
            this->_started = true;
            return (true);
        }

        ft_task_scheduler &get()
        {
            return (this->_scheduler);
        }
};
```

The guard launches the scheduler lazily and always shuts it down. Because the
constructor and destructor are trivial, the adapter can live on the stack in
unit tests or in longer lived service objects.

## Networking module: socket lifetime wrapper

`ft_socket` already closes itself in its destructor, but embedding it in a
higher-level adapter lets you bundle connection semantics and error reporting
with one object. The `client_socket_guard` below demonstrates how to pair
configuration, connection, and teardown logic.

```cpp
#include "Networking/socket_class.hpp"

class client_socket_guard
{
    private:
        ft_socket _socket;
        bool _connected;

    public:
        client_socket_guard()
            : _socket(), _connected(false)
        {
            return ;
        }

        ~client_socket_guard()
        {
            if (!this->_connected)
                return ;
            this->_socket.close_socket();
            return ;
        }

        bool connect(const char *ip, uint16_t port, int timeout_ms)
        {
            SocketConfig config;

            config._type = SocketType::CLIENT;
            config._ip = ip;
            config._port = port;
            config._recv_timeout = timeout_ms;
            config._send_timeout = timeout_ms;
            this->_socket = ft_socket(config);
            if (this->_socket.get_error() != ER_SUCCESS)
                return (false);
            this->_connected = true;
            return (true);
        }

        ft_socket &get()
        {
            return (this->_socket);
        }
};
```

Adapters like these reduce the chances of leaking resources or skipping cleanup
steps. They also codify the error-handling rules from the individual modules so
new contributors can copy a known-good pattern when wrapping Libft types inside
application specific classes.
