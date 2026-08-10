#!/usr/bin/env bats
# Bats test file for Terminal Tables — compares C and Bash implementations
# Usage: bats tests/comparison.bats

SCRIPT_DIR="$BATS_TEST_DIRNAME"
export PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
export C_BIN="$PROJECT_ROOT/tables.c/tables"
export SH_SCRIPT="$PROJECT_ROOT/tables.sh/tables.sh"

setup() {
    TEST_TMPDIR=$(mktemp -d)
    CROOT="$TEST_TMPDIR/croot"
    SHROOT="$TEST_TMPDIR/shroot"

    mkdir -p "$CROOT/tables.c/tables.c"
    ln -sf "$C_BIN" "$CROOT/tables.c/tables"
    ln -sf "$C_BIN" "$CROOT/tables.c/tables.c/tables"
    mkdir -p "$CROOT/tables.c/tst"

    mkdir -p "$SHROOT/tables.sh/tst"
    ln -sf "$SH_SCRIPT" "$SHROOT/tables.sh/tables.sh"
}

teardown() {
    rm -rf "$TEST_TMPDIR"
}

normalize_output() {
    local output="$1"
    output=$(echo "$output" | sed 's/\x1b\[[0-9;]*m//g')
    output=$(echo "$output" | sed -E 's/[0-9]{4}-[0-9]{2}-[0-9]{2}/DATE/g')
    output=$(echo "$output" | sed -E 's/[0-9]{2}:[0-9]{2}:[0-9]{2}/TIME/g')
    output=$(echo "$output" | sed -E 's/TestC ([0-9]-[A-Z]+)/Test \1/g')
    output=$(echo "$output" | sed -E 's/^-+$//' | sed -E 's/^─+$//')
    echo "$output"
}

run_comparison() {
    local c_script="$1" sh_script="$2"

    local c_dest="$CROOT/tables.c/tst/$(basename "$c_script")"
    local sh_dest="$SHROOT/tables.sh/tst/$(basename "$sh_script")"

    cp "$c_script" "$c_dest"
    chmod +x "$c_dest"

    cp "$sh_script" "$sh_dest"
    chmod +x "$sh_dest"

    local c_raw c_out sh_raw sh_out
    c_raw=$(timeout 30 bash "$c_dest" 2>&1) || true
    c_out=$(normalize_output "$c_raw")

    sh_raw=$(timeout 120 bash "$sh_dest" 2>&1) || true
    sh_out=$(normalize_output "$sh_raw")

    [[ "$c_out" == "$sh_out" ]]
}

# Test Suite 01: Basic - Various datatypes and justifications
@test "Suite 01: Basic datatypes and justifications" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_01_basic.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_01_basic.sh"
}

# Test Suite 02: Summary
@test "Suite 02: Sum, min, max, avg, count, unique summaries" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_02_summary.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_02_summary.sh"
}

# Test Suite 03: Wrapping
@test "Suite 03: Text wrapping modes" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_03_wrapping.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_03_wrapping.sh"
}

# Test Suite 04: Complex
@test "Suite 04: Complex tables with multiple features" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_04_complex.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_04_complex.sh"
}

# Test Suite 05: Titles
@test "Suite 05: Title rendering and positioning" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_05_titles.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_05_titles.sh"
}

# Test Suite 06: Title positions
@test "Suite 06: Title position clipping (left/center/right)" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_06_title_positions.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_06_title_positions.sh"
}

# Test Suite 07: Footers
@test "Suite 07: Footer rendering and positioning" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_07_footers.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_07_footers.sh"
}

# Test Suite 08: Footer positions
@test "Suite 08: Footer position clipping (left/center/right)" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_08_footer_positions.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_08_footer_positions.sh"
}

# Test Suite 09: Showcase
@test "Suite 09: Showcase with multiple tables" {
    run_comparison \
        "$PROJECT_ROOT/tables.c/tst/tables_test_09_showcase.sh" \
        "$PROJECT_ROOT/tables.sh/tst/tables_test_09_showcase.sh"
}
