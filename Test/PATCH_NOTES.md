# Patch Notes

## 2026-06-15

- Added portable 64-bit decimal format helpers in `Modules/Basic/limits.hpp`.
- Documented the new 64-bit format macros in `Modules/Basic/README.md`.
- Switched integer formatting in the application auth service, system test runner, template serialization, and efficiency helpers to use the new decimal format macros.
- Updated the printf stress-allocation test to exercise the new formatting constants.
- Refreshed `api_request_circuit_debug.log` with the latest debug output.
