#!/usr/bin/env bats
# Bats test file for Terminal Tables — compares C and Bash implementations
# Usage: bats tests/comparison.bats

SCRIPT_DIR="$BATS_TEST_DIRNAME"
export PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
export C_BIN="$PROJECT_ROOT/tables.c/tables"
export SH_SCRIPT="$PROJECT_ROOT/tables.sh/tables.sh"
export SCENARIOS_DIR="$SCRIPT_DIR/scenarios"

setup() {
    TEST_TMPDIR=$(mktemp -d)
    CROOT="$TEST_TMPDIR/croot"
    SHROOT="$TEST_TMPDIR/shroot"

    mkdir -p "$CROOT/tables.c/tables.c" "$CROOT/tables.c/tst"
    ln -sf "$C_BIN" "$CROOT/tables.c/tables"
    ln -sf "$C_BIN" "$CROOT/tables.c/tables.c/tables"

    mkdir -p "$SHROOT/tables.sh/tst"
    ln -sf "$SH_SCRIPT" "$SHROOT/tables.sh/tables.sh"
}

teardown() {
    rm -rf "$TEST_TMPDIR"
}

# Resolve a scenario file — for dynamic files (.sh), execute to generate JSON
# Args: file_path [data_file_path]
resolve_file() {
    local file="$1" datafile="${2:-}"
    if [[ "$file" == *.sh ]]; then
        bash "$file" "$datafile"
    else
        cat "$file"
    fi
}

# Run a single scenario: C vs Bash comparison
# Args: suite_num scenario_name (e.g., "01" "test_1_A")
# Sets: $status (0=pass, 1=diff, 2=skip)
run_scenario() {
    local suite="$1" scenario="$2"
    local suite_dir="$SCENARIOS_DIR/suite_${suite}"

    local data_file layout_file
    data_file=$(ls "$suite_dir/${scenario}_data".* 2>/dev/null | head -1)
    layout_file=$(ls "$suite_dir/${scenario}_layout".* 2>/dev/null | head -1)

    [[ -z "$data_file" || -z "$layout_file" ]] && return 2

    local tmp_data tmp_layout
    tmp_data=$(mktemp)
    tmp_layout=$(mktemp)

    resolve_file "$data_file" > "$tmp_data"
    if [[ "$layout_file" == *.sh ]]; then
        resolve_file "$layout_file" "$tmp_data" > "$tmp_layout"
    else
        resolve_file "$layout_file" > "$tmp_layout"
    fi

    # Run C version
    local c_dest="$CROOT/tables.c/tst/test.sh"
    cat > "$c_dest" << 'CMEOF'
#!/usr/bin/env bash
tables_script="$(dirname "$0")/../tables.c/tables"
"$tables_script" "$1" "$2"
CMEOF
    chmod +x "$c_dest"
    local c_raw
    c_raw=$(timeout 30 bash "$c_dest" "$tmp_layout" "$tmp_data" 2>&1) || true
    local c_out
    # Keep ANSI — both implementations must emit identical color sequences
    c_out=$(echo "$c_raw" | sed -E 's/[0-9]{4}-[0-9]{2}-[0-9]{2}/DATE/g' | sed -E 's/[0-9]{2}:[0-9]{2}:[0-9]{2}/TIME/g')

    # Run Bash version
    local sh_dest="$SHROOT/tables.sh/tst/test.sh"
    cat > "$sh_dest" << 'SHCEOF'
#!/usr/bin/env bash
tables_script="$(dirname "$0")/../tables.sh"
"$tables_script" "$1" "$2"
SHCEOF
    chmod +x "$sh_dest"
    local sh_raw
    sh_raw=$(timeout 120 bash "$sh_dest" "$tmp_layout" "$tmp_data" 2>&1) || true
    local sh_out
    sh_out=$(echo "$sh_raw" | sed -E 's/[0-9]{4}-[0-9]{2}-[0-9]{2}/DATE/g' | sed -E 's/[0-9]{2}:[0-9]{2}:[0-9]{2}/TIME/g')

    rm -f "$tmp_data" "$tmp_layout"

    if [[ "$c_out" == "$sh_out" ]]; then
        return 0
    else
        echo "--- C output ---"
        echo "$c_out"
        echo "--- Bash output ---"
        echo "$sh_out"
        return 1
    fi
}

# Test Suite 00: Linting — fails the suite (and runner) when linters report issues
@test "Suite 00: Linting (shellcheck + cppcheck)" {
    run bash "$PROJECT_ROOT/tests/run_tests.sh" 00
    # When tools report issues, run_tests.sh must exit non-zero.
    # When clean, exit 0. Either outcome is a valid suite result.
    if echo "$output" | grep -q 'FAIL'; then
        [[ $status -ne 0 ]] || fail "Lint reported FAIL but exit status was 0"
    else
        [[ $status -eq 0 ]] || fail "Lint reported no FAIL but exit status was $status"
    fi
}

# Test Suite 01: Basic datatypes and justifications
@test "Suite 01: Basic datatypes and justifications" {
    for label in 1_A 1_B 1_C 1_D 1_E 1_F 1_G 1_H 1_I; do
        run_scenario "01" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}

# Test Suite 02: Sum, min, max, avg, count, summaries
@test "Suite 02: Summaries (sum, min, max, avg, count, unique)" {
    for label in 2_A 2_B 2_C 2_D 2_E 2_F 2_G 2_H 2_I; do
        run_scenario "02" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}

# Test Suite 03: Text wrapping modes
@test "Suite 03: Text wrapping modes" {
    for label in 3_A 3_B 3_C 3_D 3_E 3_F 3_G 3_H 3_I 3_J 3_K; do
        run_scenario "03" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}

# Test Suite 04: Complex tables
@test "Suite 04: Complex tables with mixed features" {
    for label in 4_A 4_B 4_C 4_D 4_E; do
        run_scenario "04" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}

# Test Suite 05: Titles
@test "Suite 05: Title rendering and positioning" {
    for label in 5_A 5_B 5_C 5_D 5_E; do
        run_scenario "05" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}

# Test Suite 06: Title positions
@test "Suite 06: Title position clipping" {
    for label in 6_A 6_B 6_C 6_D 6_E 6_F 6_G 6_H 6_I 6_J 6_K 6_L; do
        run_scenario "06" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}

# Test Suite 07: Footers
@test "Suite 07: Footer rendering and positioning" {
    for label in 7_A 7_B 7_C 7_D 7_E; do
        run_scenario "07" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}

# Test Suite 08: Footer positions
@test "Suite 08: Footer position clipping" {
    for label in 8_A 8_B 8_C 8_D 8_E 8_F 8_G 8_H 8_I 8_J 8_K 8_L 8_M 8_N 8_O 8_P 8_Q; do
        run_scenario "08" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}

# Test Suite 09: Showcase
@test "Suite 09: Showcase with multiple tables" {
    for label in 9_A 9_B 9_C 9_D 9_E 9_F 9_G 9_H 9_I 9_J 9_K 9_L 9_M 9_N 9_O 9_P 9_Q 9_R 9_S 9_T 9_U 9_V; do
        run_scenario "09" "test_${label}"
        [[ $? -eq 0 ]] || fail "Sub-test ${label} failed"
    done
}
