#!/usr/bin/env bash
set -u

state_dir="Test/.libft_progress"
lock_dir="Test/.libft_progress.lock"
bar_width=24
if [ -t 1 ]; then
    color_on=$'\033[1;35m'
    color_off=$'\033[0m'
else
    color_on=""
    color_off=""
fi

lock_progress() {
    while ! mkdir "$lock_dir" 2>/dev/null; do
        sleep 0.02
    done
}

unlock_progress() {
    rmdir "$lock_dir" 2>/dev/null || true
}

make_bar() {
    local current="$1"
    local total="$2"
    local filled empty

    if [ "$total" -le 0 ]; then
        total=1
    fi
    filled=$((current * bar_width / total))
    if [ "$filled" -gt "$bar_width" ]; then
        filled="$bar_width"
    fi
    empty=$((bar_width - filled))
    printf '%*s' "$filled" '' | tr ' ' '#'
    printf '%*s' "$empty" '' | tr ' ' '-'
}

short_name() {
    local module="$1"
    module=${module#Modules/}
    module=${module%/*.a}
    module=${module%_debug}
    module=${module%_test}
    printf '%s' "$module"
}

init_progress() {
    local total="$1"
    mkdir -p "$state_dir"
    lock_progress
    if [ ! -f "$state_dir/initialized" ]; then
        printf '%s[LIBFT BUILD]%s Parallel module progress (%s modules)\n' "$color_on" "$color_off" "$total"
        : > "$state_dir/initialized"
    fi
    unlock_progress
}

render_line() {
    local total_modules="$1"
    local index="$2"
    local module="$3"
    local current="$4"
    local total_files="$5"
    local state="$6"
    local file="$7"
    local bar name percent

    if [ "$index" -le 0 ]; then
        return 0
    fi
    bar="$(make_bar "$current" "$total_files")"
    name="$(short_name "$module")"
    if [ "$total_files" -le 0 ]; then
        percent=0
    else
        percent=$((current * 100 / total_files))
    fi
    lock_progress
    printf '%s[LIBFT %02d/%02d]%s %-22s [%s] %3d%% %s %d/%d' \
        "$color_on" "$index" "$total_modules" "$color_off" "$name" "$bar" "$percent" "$state" "$current" "$total_files"
    if [ -n "$file" ]; then
        printf ' :: %.58s' "$file"
    fi
    printf '\n'
    unlock_progress
}

finish_progress() {
    if [ -d "$state_dir" ]; then
        lock_progress
        rm -rf "$state_dir"
        unlock_progress
    fi
    rmdir "$lock_dir" 2>/dev/null || true
}

case "${1:-}" in
    init)
        shift
        init_progress "$@"
        ;;
    update)
        shift
        render_line "$@"
        ;;
    finish)
        finish_progress
        ;;
    *)
        printf 'usage: %s init|update|finish ...\n' "$0" >&2
        exit 2
        ;;
esac
