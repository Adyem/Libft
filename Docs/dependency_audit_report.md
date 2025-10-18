# Dependency Audit Report

## Overview
This report documents the dependency audits completed on 2025-10-18 for the Libft project. The goal is to ensure all third-party
components are tracked, evaluated for known vulnerabilities, and accompanied by remediation timelines when applicable.

## Audit Procedure
The `tools/run_dependency_audits.sh` helper orchestrates the set of audits:

- **C/C++ toolchain** – Verifies that the build links only against the system C/C++ standard libraries and the project's own
  modules. Because Libft vendors all functionality and does not rely on external package managers, there are no external
  dependencies to scan.
- **Python utilities** – Searches for `requirements.txt` files under `tools/` and `Docs/`. If any are found and `pip-audit`
  is available in the environment, the script runs `pip-audit -r` for each requirements file.
- **Node.js tooling** – Detects `package.json` manifests. When `npm` is present, `npm audit --audit-level=moderate` is executed
  against each manifest.
- **Rust helpers** – Checks for `Cargo.toml` manifests and invokes `cargo audit` when both a manifest and the cargo-audit plugin
  are available.

Every audit run writes structured results to `Docs/dependency_audit_history/` so we maintain a trail of past runs and can track
long-term trends.

## Current Status (2025-10-18)
| Ecosystem | Manifests Detected | Tool Availability | Result | Notes |
|-----------|--------------------|-------------------|--------|-------|
| C/C++     | Not applicable (no external package manager) | Built-in | ✅ Passed | No external libraries are consumed; internal modules rely solely on the system toolchain. |
| Python    | None | `pip-audit` not installed | ⚠️ Skipped | No Python dependencies found. Installing `pip-audit` enables automatic checks if requirements are introduced. |
| Node.js   | None | `npm` | ⚠️ Skipped | No Node manifests present. |
| Rust      | None | Not installed | ⚠️ Skipped | No Rust manifests present. |

## Remediation Tracking
No remediation items are currently required because no vulnerable dependencies were detected. Future issues will be tracked in
this section with expected fix versions and owners.

## Next Steps
- Schedule automated execution of `tools/run_dependency_audits.sh` in the continuous integration environment on a weekly cadence.
- Re-run the audits whenever new third-party components are introduced.
- Expand the script with additional ecosystem checks as the project grows (e.g., Go `go.mod`, Java `pom.xml`).

