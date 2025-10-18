# System utility security hardening

Services built on the System utilities module should apply defense-in-depth
controls that restrict privilege and limit the blast radius of compromise. This
note captures the baseline controls we expect production deployments to enable.

## Process isolation

- **chroot / pivot_root**: Run services inside a dedicated filesystem jail so a
  compromise cannot read or modify arbitrary host files. Ensure the jail contains
  only the minimal files the service needs (configuration, sockets, shared
  libraries) and mount everything read-only when possible.
- **Namespaces and containers**: On Linux, combine chroot with user, PID, and
  network namespaces. Drop ambient capabilities using `prctl(PR_CAPBSET_DROP, …)`
  and only retain the specific capabilities required (for example, `CAP_NET_BIND_SERVICE`).

## Syscall filtering

- **seccomp-bpf**: Install a syscall allow-list before processing untrusted
  input. Start from a minimal baseline (read, write, close, clock_gettime) and
  explicitly opt in to additional syscalls required by the service. Log and exit
  when the filter blocks an unexpected request so incidents become visible.
- **pledge / unveil**: On OpenBSD, use `pledge` to constrain API classes and
  `unveil` to restrict filesystem visibility. Match these policies with the
  service's configuration options and update regression tests when the syscall
  set changes.

## Memory and resource limits

- Set RLIMIT values (`RLIMIT_NOFILE`, `RLIMIT_CORE`, `RLIMIT_MEMLOCK`) before
  spawning worker threads. Ensure the limits match expectations documented in the
  module README.
- Enable address space layout randomization (ASLR) and stack canaries. When
  compiling the project, enable `-fstack-protector-strong` and `-D_FORTIFY_SOURCE=2`.
- Prefer position-independent executables (`-pie -fPIE`) when building service
  binaries so relocations remain randomized.

## Runtime monitoring

- Emit structured logs for privilege escalation attempts and sandbox violations.
- Surface health endpoints that check sandbox state (for example, verifying the
  seccomp filter loaded correctly) and expose metrics to operations dashboards.
- Integrate crash handlers that collect minimal diagnostics without leaking
  secrets from restricted filesystems.

## Deployment checklist

1. Create a dedicated service account with no interactive login privileges.
2. Provision a chroot directory owned by root with `0750` permissions.
3. Install configuration files with `0640` permissions and run-time sockets with
   `0660` permissions owned by the service account group.
4. Drop privileges via `setgid`/`setuid` before accepting network traffic.
5. Apply the seccomp filter immediately after initializing required subsystems.
6. Register liveness probes that assert the sandbox is active and file
   permissions match expectations.
7. Document incident response steps for reissuing keys and rebuilding the jail.
