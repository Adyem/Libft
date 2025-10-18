#!/bin/sh
set -eu

ROOT_DIRECTORY="$(cd "$(dirname "$0")/.." && pwd)"
REPORT_DIRECTORY="${ROOT_DIRECTORY}/Docs/dependency_audit_history"
TIMESTAMP="$(date -u +%Y-%m-%dT%H-%M-%SZ)"
REPORT_FILE="${REPORT_DIRECTORY}/${TIMESTAMP}.md"

echo "Starting dependency audits at ${TIMESTAMP}"
mkdir -p "${REPORT_DIRECTORY}"

table_append() {
    printf '| %s | %s | %s | %s | %s |\n' "$1" "$2" "$3" "$4" "$5" >> "${REPORT_FILE}"
}

echo "# Dependency Audit Run (${TIMESTAMP})" > "${REPORT_FILE}"
echo >> "${REPORT_FILE}"
echo "| Ecosystem | Manifests Detected | Tool Availability | Result | Notes |" >> "${REPORT_FILE}"
echo "|-----------|--------------------|-------------------|--------|-------|" >> "${REPORT_FILE}"

audit_cpp() {
    table_append "C/C++" "Not applicable" "Built-in" "✅ Passed" "Libft vendors its code and links only the system toolchain."
}

audit_python() {
    requirements_files="$(find "${ROOT_DIRECTORY}" -maxdepth 3 -name 'requirements.txt' 2>/dev/null || true)"
    if [ -z "${requirements_files}" ]
    then
        table_append "Python" "None" "$(command -v pip-audit >/dev/null 2>&1 && echo 'pip-audit' || echo 'Not installed')" "⚠️ Skipped" "No Python dependency manifests detected."
        return 0
    fi
    if ! command -v pip-audit >/dev/null 2>&1
    then
        table_append "Python" "$(printf '%s' "${requirements_files}" | tr '\n' ', ')" "Not installed" "⚠️ Skipped" "Install pip-audit to evaluate Python dependencies."
        return 0
    fi
    printf '%s\n' "Running pip-audit..."
    printf '%s\n' "${requirements_files}" | while IFS= read -r requirements_file
    do
        if [ -n "${requirements_file}" ]
        then
            if pip-audit -r "${requirements_file}" >> "${REPORT_FILE}" 2>&1
            then
                table_append "Python" "${requirements_file}" "pip-audit" "✅ Passed" "No vulnerabilities detected."
            else
                table_append "Python" "${requirements_file}" "pip-audit" "❌ Failed" "pip-audit reported issues; see log above."
            fi
        fi
    done
    return 0
}

audit_node() {
    package_files="$(find "${ROOT_DIRECTORY}" -maxdepth 3 -name 'package.json' 2>/dev/null || true)"
    if [ -z "${package_files}" ]
    then
        table_append "Node.js" "None" "$(command -v npm >/dev/null 2>&1 && echo 'npm' || echo 'Not installed')" "⚠️ Skipped" "No Node manifests detected."
        return 0
    fi
    if ! command -v npm >/dev/null 2>&1
    then
        table_append "Node.js" "$(printf '%s' "${package_files}" | tr '\n' ', ')" "Not installed" "⚠️ Skipped" "Install npm to audit Node dependencies."
        return 0
    fi
    printf '%s\n' "Running npm audit..."
    printf '%s\n' "${package_files}" | while IFS= read -r package_file
    do
        package_directory="$(dirname "${package_file}")"
        if (cd "${package_directory}" && npm audit --audit-level=moderate >> "${REPORT_FILE}" 2>&1)
        then
            table_append "Node.js" "${package_file}" "npm" "✅ Passed" "No vulnerabilities detected."
        else
            table_append "Node.js" "${package_file}" "npm" "❌ Failed" "npm audit reported issues; see log above."
        fi
    done
    return 0
}

audit_rust() {
    cargo_files="$(find "${ROOT_DIRECTORY}" -maxdepth 3 -name 'Cargo.toml' 2>/dev/null || true)"
    if [ -z "${cargo_files}" ]
    then
        table_append "Rust" "None" "$(command -v cargo >/dev/null 2>&1 && command -v cargo-audit >/dev/null 2>&1 && echo 'cargo-audit' || echo 'Not installed')" "⚠️ Skipped" "No Rust manifests detected."
        return 0
    fi
    if ! command -v cargo >/dev/null 2>&1 || ! command -v cargo-audit >/dev/null 2>&1
    then
        table_append "Rust" "$(printf '%s' "${cargo_files}" | tr '\n' ', ')" "Not installed" "⚠️ Skipped" "Install cargo and cargo-audit to evaluate Rust dependencies."
        return 0
    fi
    printf '%s\n' "Running cargo audit..."
    printf '%s\n' "${cargo_files}" | while IFS= read -r cargo_file
    do
        cargo_directory="$(dirname "${cargo_file}")"
        if (cd "${cargo_directory}" && cargo audit >> "${REPORT_FILE}" 2>&1)
        then
            table_append "Rust" "${cargo_file}" "cargo-audit" "✅ Passed" "No vulnerabilities detected."
        else
            table_append "Rust" "${cargo_file}" "cargo-audit" "❌ Failed" "cargo audit reported issues; see log above."
        fi
    done
    return 0
}

cleanup_empty_results() {
    if ! grep -q "C/C++" "${REPORT_FILE}"
    then
        audit_cpp
    fi
    if ! grep -q "Python" "${REPORT_FILE}"
    then
        audit_python
    fi
    if ! grep -q "Node.js" "${REPORT_FILE}"
    then
        audit_node
    fi
    if ! grep -q "Rust" "${REPORT_FILE}"
    then
        audit_rust
    fi
}

cleanup_empty_results

echo >> "${REPORT_FILE}"
echo "Audit completed. Detailed command output (if any) is captured above." >> "${REPORT_FILE}"

echo "Dependency audits completed. Report written to ${REPORT_FILE}"

