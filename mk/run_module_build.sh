#!/usr/bin/env bash
set -u

total_modules="$1"
progress_index="$2"
module_path="$3"
log_file="$4"
shift 4
if [ "${1:-}" = "--" ]; then
    shift
fi

progress_script="mk/progress.sh"
modules=${LIBFT_PROGRESS_MODULES:-$module_path}
status_file="Test/.libft_build_status_$$_${progress_index}"

cleanup_on_interrupt() {
    "$progress_script" finish
    rm -f "$status_file" "$log_file"
    exit 130
}
trap cleanup_on_interrupt INT TERM

# shellcheck disable=SC2086
"$progress_script" init "$total_modules" $modules
: > "$log_file"
rm -f "$status_file"

last_count=0
last_total=1
last_file=""
while IFS= read -r line; do
    printf '%s\n' "$line" >> "$log_file"
    case "$line" in
        *"Building file "*)
            count=$(printf '%s\n' "$line" | sed -n 's/.*(\([0-9][0-9]*\)\/\([0-9][0-9]*\)).*/\1/p')
            total=$(printf '%s\n' "$line" | sed -n 's/.*(\([0-9][0-9]*\)\/\([0-9][0-9]*\)).*/\2/p')
            file=$(printf '%s\n' "$line" | sed -n 's/.*Building file \(.*\) ([0-9][0-9]*\/[0-9][0-9]*).*/\1/p')
            if [ -n "$count" ] && [ -n "$total" ]; then
                last_count="$count"
                last_total="$total"
                last_file="$file"
                "$progress_script" update "$total_modules" "$progress_index" "$module_path" "$count" "$total" "build" "$file"
            fi
            ;;
    esac
done < <( "$@" 2>&1; printf '%s\n' "$?" > "$status_file" )

status=1
if [ -f "$status_file" ]; then
    status=$(cat "$status_file")
fi
rm -f "$status_file"

if [ "$status" -eq 0 ]; then
    "$progress_script" update "$total_modules" "$progress_index" "$module_path" "$last_total" "$last_total" "done" ""
    rm -f "$log_file"
else
    "$progress_script" update "$total_modules" "$progress_index" "$module_path" "$last_count" "$last_total" "failed" "$last_file"
    "$progress_script" finish
    cat "$log_file"
    rm -f "$log_file"
fi
exit "$status"
