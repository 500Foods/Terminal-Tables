#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

export C_BIN="$PROJECT_ROOT/tables.c/tables"
export SH_SCRIPT="$PROJECT_ROOT/tables.sh/tables.sh"

pass_count=0
fail_count=0

export TMPDIR=$(mktemp -d)
export CROOT="$TMPDIR/croot"
export SHROOT="$TMPDIR/shroot"

cleanup() { rm -rf "$TMPDIR"; }
trap cleanup EXIT

normalize_output() {
    local output="$1"
    output=$(echo "$output" | sed 's/\x1b\[[0-9;]*m//g')
    output=$(echo "$output" | sed -E 's/[0-9]{4}-[0-9]{2}-[0-9]{2}/DATE/g')
    output=$(echo "$output" | sed -E 's/[0-9]{2}:[0-9]{2}:[0-9]{2}/TIME/g')
    output=$(echo "$output" | sed -E 's/TestC ([0-9]-[A-Z]+)/Test \1/g')
    output=$(echo "$output" | sed -E 's/^-+$//' | sed -E 's/^─+$//')
    echo "$output"
}

run_test_case() {
    local c_script="$1" sh_script="$2" suite_num="$3"

    local c_dest="$CROOT/tables.c/tst/$(basename "$c_script")"
    local sh_dest="$SHROOT/tables.sh/tst/$(basename "$sh_script")"

    mkdir -p "$CROOT/tables.c/tables.c" "$CROOT/tables.c/tst"
    rm -f "$CROOT/tables.c/tables" "$CROOT/tables.c/tables.c/tables"
    ln -sf "$C_BIN" "$CROOT/tables.c/tables"
    ln -sf "$C_BIN" "$CROOT/tables.c/tables.c/tables"
    cp "$c_script" "$c_dest"
    chmod +x "$c_dest"
    local c_raw c_out
    c_raw=$(timeout 30 bash "$c_dest" 2>&1) || true
    c_out=$(normalize_output "$c_raw")

    mkdir -p "$SHROOT/tables.sh/tst"
    rm -f "$SHROOT/tables.sh/tables.sh"
    ln -sf "$SH_SCRIPT" "$SHROOT/tables.sh/tables.sh"
    cp "$sh_script" "$sh_dest"
    chmod +x "$sh_dest"
    local sh_raw sh_out
    sh_raw=$(timeout 120 bash "$sh_dest" 2>&1) || true
    sh_out=$(normalize_output "$sh_raw")

    if [[ "$c_out" == "$sh_out" ]]; then
        echo -e "\033[0;32mPASS\033[0m"
        return 0
    else
        echo -e "\033[0;31mDIFF\033[0m"
        diff <(echo "$c_out") <(echo "$sh_out") | head -30
        return 1
    fi
}

echo "=== Terminal Tables Test Suite ==="
echo ""

SUITES=("$@")
if [[ ${#SUITES[@]} -eq 0 ]]; then
    SUITES=("01" "02" "03" "04" "05" "06" "07" "08" "09")
fi

for suite_num in "${SUITES[@]}"; do
    script_file=""
    for f in "$PROJECT_ROOT/tables.c/tst/tables_test_${suite_num}"_*.sh; do
        if [[ -f "$f" ]]; then
            script_file="$f"
            break
        fi
    done

    if [[ -z "$script_file" ]]; then
        echo -e "\033[0;33mSKIP Suite ${suite_num}: script not found\033[0m"
        continue
    fi

    suite_name=$(basename "$script_file" .sh)
    echo "--- Suite ${suite_num}: ${suite_name} ---"

    c_script="$script_file"
    base_name=$(basename "$script_file")
    sh_script="$PROJECT_ROOT/tables.sh/tst/$base_name"

    result=$(run_test_case "$c_script" "$sh_script" "$suite_num")
    if [[ $? -eq 0 ]]; then
        pass_count=$((pass_count + 1))
    else
        fail_count=$((fail_count + 1))
    fi
    echo "$result"
    echo ""
done

echo ""
echo "=== Summary ==="
echo -e "Passed: \033[0;32m$pass_count\033[0m"
echo -e "Failed: \033[0;31m$fail_count\033[0m"
echo "Total: $((pass_count + fail_count))"
