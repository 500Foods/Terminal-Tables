#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

C_BIN="$PROJECT_ROOT/tables.c/tables"
SH_SCRIPT="$PROJECT_ROOT/tables.sh/tables.sh"
SCENARIOS_DIR="$SCRIPT_DIR/scenarios"
MANIFEST="$SCENARIOS_DIR/manifest.json"
PERF_DATA_JSON="$SCRIPT_DIR/performance_data.json"
PERF_LAYOUT_JSON="$SCRIPT_DIR/performance_layout.json"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

SCRIPT_PATH="$SCRIPT_DIR/$(basename "${BASH_SOURCE[0]}")"

# Internal worker mode: `run_tests.sh --job-worker <idx> <jobs_tsv> <job_tmpdir>`
# is re-invoked (via xargs, one process per parallel slot) to run a single
# scenario out-of-process so scenarios can execute concurrently.
JOB_WORKER_MODE=""
if [[ "${1:-}" == "--job-worker" ]]; then
    JOB_WORKER_MODE=1
    JOB_IDX="$2"
    JOBS_TSV="$3"
    JOB_TMPDIR="$4"
fi

show_saved_results() {
    if [[ ! -f "$PERF_LAYOUT_JSON" || ! -f "$PERF_DATA_JSON" ]]; then
        echo "No saved performance results found."
        echo "Run tests first (e.g. bash tests/run_tests.sh 01), then:"
        echo "  bash tests/run_tests.sh --results"
        exit 1
    fi
    if [[ ! -x "$C_BIN" ]]; then
        echo "ERROR: C binary not found or not executable at $C_BIN"
        exit 1
    fi
    "$C_BIN" "$PERF_LAYOUT_JSON" "$PERF_DATA_JSON"
    exit 0
}

# Parse flags; remaining args are suite filters
SUITES=()
if [[ -z "$JOB_WORKER_MODE" ]]; then
    for arg in "$@"; do
        case "$arg" in
            --results|-r)
                show_saved_results
                ;;
            --help|-h)
                cat <<'USAGE'
Usage: run_tests.sh [OPTIONS] [SUITE ...]

  Run comparison tests for the given suites (default: 00–09).

Options:
  --results, -r   Re-display the performance table from the last run
  --help, -h      Show this help

Examples:
  bash tests/run_tests.sh
  bash tests/run_tests.sh 01 04
  bash tests/run_tests.sh --results
USAGE
                exit 0
                ;;
            -*)
                echo "Unknown option: $arg (try --help)"
                exit 1
                ;;
            *)
                SUITES+=("$arg")
                ;;
        esac
    done
fi

pass_count=0
fail_count=0
total_count=0

if [[ -z "$JOB_WORKER_MODE" ]]; then
    RUN_ID="$$_${RANDOM}${RANDOM}"
    TEST_TMPDIR=$(mktemp -d)
    PERF_FILE="$TEST_TMPDIR/perf.tsv"
    export PERF_FILE
    touch "$PERF_FILE"
    SUITE_NAMES_FILE="$TEST_TMPDIR/suite_names.tsv"
    touch "$SUITE_NAMES_FILE"

    cleanup() { rm -rf "$TEST_TMPDIR"; }
    trap cleanup EXIT
fi

normalize_output() {
    local output="$1"
    # Keep ANSI and separator geometry — only dynamic timestamps are normalized
    output=$(echo "$output" | sed -E 's/[0-9]{4}-[0-9]{2}-[0-9]{2}/DATE/g')
    output=$(echo "$output" | sed -E 's/[0-9]{2}:[0-9]{2}:[0-9]{2}/TIME/g')
    echo "$output"
}

resolve_file() {
    local file="$1"
    local datafile="${2:-}"
    if [[ "$file" == *.sh ]]; then
        bash "$file" "$datafile"
    else
        cat "$file"
    fi
}

# Title-case suite_name from manifest (title_positions -> Title Positions)
format_suite_name() {
    local name="$1"
    echo "$name" | tr '_' ' ' | awk '{
        for (i = 1; i <= NF; i++) {
            $i = toupper(substr($i, 1, 1)) tolower(substr($i, 2))
        }
        print
    }'
}

# Thousands separators for integer display
format_int() {
    local n="$1"
    echo "$n" | sed ':a;s/\B[0-9]\{3\}\>/,&/;ta'
}

# Format milliseconds with thousands separators and a trailing " ms"
format_ms() {
    local ms="$1"
    echo "$(format_int "$ms") ms"
}

# cloc code-line counts: Bash = tables.sh; C = tables.c sources + headers
# CSV columns: files,language,blank,comment,code  (SUM row has language=SUM)
count_loc() {
    local bash_loc=0 c_loc=0
    if command -v cloc >/dev/null 2>&1; then
        bash_loc=$(cloc --csv --quiet "$PROJECT_ROOT/tables.sh" 2>/dev/null | awk -F, '$2=="SUM"{print $5; exit}')
        c_loc=$(cloc --csv --quiet --include-lang=C,"C/C++ Header" "$PROJECT_ROOT/tables.c" 2>/dev/null | awk -F, '$2=="SUM"{print $5; exit}')
        bash_loc=${bash_loc:-0}
        c_loc=${c_loc:-0}
    fi
    echo "$bash_loc" "$c_loc"
}

# Color language timings: lowest = {GREEN}, highest = {RED}.
# Takes a list of integer ms values; prints one colored "N ms" string per input
# on its own line. Equal values share the same color (green if all equal).
colorize_timings() {
    local -a vals=("$@")
    local n=${#vals[@]}
    [[ $n -eq 0 ]] && return

    local min="${vals[0]}"
    local max="${vals[0]}"
    local v
    for v in "${vals[@]}"; do
        [[ $v -lt $min ]] && min=$v
        [[ $v -gt $max ]] && max=$v
    done

    for v in "${vals[@]}"; do
        local label
        label=$(format_ms "$v")
        if [[ $min -eq $max ]]; then
            echo "{GREEN}${label}{NC}"
        elif [[ $v -eq $min ]]; then
            echo "{GREEN}${label}{NC}"
        elif [[ $v -eq $max ]]; then
            echo "{RED}${label}{NC}"
        else
            # Middle values (future languages) stay uncolored
            echo "$label"
        fi
    done
}

run_lint_suite() {
    local issues=0
    local sh_ms=0
    local c_ms=0

    echo -n "  shellcheck (tables.sh): "
    if ! command -v shellcheck >/dev/null 2>&1; then
        echo -e "${YELLOW}SKIP${NC} (shellcheck not installed)"
    else
        local sc_out sc_rc sc_start sc_end
        sc_rc=0
        sc_start=$(date +%s%N)
        sc_out=$(shellcheck "$SH_SCRIPT" 2>&1) || sc_rc=$?
        sc_end=$(date +%s%N)
        sh_ms=$(( (sc_end - sc_start) / 1000000 ))
        if [[ $sc_rc -eq 0 ]]; then
            echo -e "${GREEN}PASS${NC}  (${sh_ms}ms)"
        else
            local sc_count
            sc_count=$(echo "$sc_out" | grep -c . || true)
            echo -e "${RED}FAIL${NC}  (${sh_ms}ms, $sc_count issue(s))"
            echo "$sc_out" | head -20 | sed 's/^/    /'
            issues=$((issues + 1))
        fi
    fi

    echo -n "  cppcheck (tables.c): "
    if ! command -v cppcheck >/dev/null 2>&1; then
        echo -e "${YELLOW}SKIP${NC} (cppcheck not installed)"
    else
        local cc_out cc_rc cc_start cc_end
        cc_rc=0
        cc_start=$(date +%s%N)
        # Defaults only; --error-exitcode so any finding fails the suite
        cc_out=$(cppcheck --error-exitcode=1 --quiet \
            "$PROJECT_ROOT/tables.c" 2>&1) || cc_rc=$?
        cc_end=$(date +%s%N)
        c_ms=$(( (cc_end - cc_start) / 1000000 ))
        if [[ $cc_rc -eq 0 ]]; then
            echo -e "${GREEN}PASS${NC}  (${c_ms}ms)"
        else
            local cc_count
            cc_count=$(echo "$cc_out" | grep -c . || true)
            echo -e "${RED}FAIL${NC}  (${c_ms}ms, $cc_count issue(s))"
            echo "$cc_out" | head -20 | sed 's/^/    /'
            issues=$((issues + 1))
        fi
    fi

    # Record lint timings for the performance table (Bash=shellcheck, C=cppcheck)
    echo -e "00\t${c_ms}\t${sh_ms}" >> "$PERF_FILE"
    if ! grep -q $'^00\t' "$SUITE_NAMES_FILE" 2>/dev/null; then
        echo -e "00\tLinting" >> "$SUITE_NAMES_FILE"
    fi

    if [[ $issues -eq 0 ]]; then
        return 0
    fi
    return 1
}

run_scenario() {
    local suite="$1" scenario="$2"
    local suite_dir="$SCENARIOS_DIR/suite_${suite}"

    local data_file layout_file
    data_file=$(ls "$suite_dir/${scenario}_data".* 2>/dev/null | head -1)
    layout_file=$(ls "$suite_dir/${scenario}_layout".* 2>/dev/null | head -1)

    if [[ -z "$data_file" || -z "$layout_file" ]]; then
        echo -e "${YELLOW}SKIP: scenario files not found${NC}"
        return 2
    fi

    local tmp_data tmp_layout
    tmp_data=$(mktemp)
    tmp_layout=$(mktemp)

    resolve_file "$data_file" > "$tmp_data"
    if [[ "$layout_file" == *.sh ]]; then
        resolve_file "$layout_file" "$tmp_data" > "$tmp_layout"
    else
        resolve_file "$layout_file" > "$tmp_layout"
    fi

    # Invoke each implementation's binary/script directly (no intermediate
    # wrapper script) to avoid an extra bash fork+exec skewing the timing.
    local c_start c_end c_raw c_out
    c_start=$(date +%s%N)
    c_raw=$(timeout 10 "$C_BIN" "$tmp_layout" "$tmp_data" 2>&1) || true
    c_end=$(date +%s%N)
    local c_ms=$(( (c_end - c_start) / 1000000 ))
    c_out=$(normalize_output "$c_raw")

    local sh_start sh_end sh_raw sh_out
    sh_start=$(date +%s%N)
    sh_raw=$(timeout 120 bash "$SH_SCRIPT" "$tmp_layout" "$tmp_data" 2>&1) || true
    sh_end=$(date +%s%N)
    local sh_ms=$(( (sh_end - sh_start) / 1000000 ))
    sh_out=$(normalize_output "$sh_raw")

    # --mono variant: same inputs, both implementations invoked with --mono
    # appended. Not timed for the performance table; only validated for
    # correctness.
    local c_mono_raw c_mono_out
    c_mono_raw=$(timeout 10 "$C_BIN" "$tmp_layout" "$tmp_data" --mono 2>&1) || true
    c_mono_out=$(normalize_output "$c_mono_raw")

    local sh_mono_raw sh_mono_out
    sh_mono_raw=$(timeout 120 bash "$SH_SCRIPT" "$tmp_layout" "$tmp_data" --mono 2>&1) || true
    sh_mono_out=$(normalize_output "$sh_mono_raw")

    rm -f "$tmp_data" "$tmp_layout"

    echo -e "${suite}\t${c_ms}\t${sh_ms}" >> "$PERF_FILE"

    local color_ok=1 mono_ok=1
    [[ "$c_out" == "$sh_out" ]] && color_ok=0
    [[ "$c_mono_out" == "$sh_mono_out" ]] && mono_ok=0

    if [[ $color_ok -eq 0 && $mono_ok -eq 0 ]]; then
        echo -e "${GREEN}PASS${NC}  (C: ${c_ms}ms, Bash: ${sh_ms}ms) [mono: ${GREEN}PASS${NC}]"
        return 0
    fi

    if [[ $color_ok -ne 0 ]]; then
        echo -e "${RED}DIFF${NC}  (C: ${c_ms}ms, Bash: ${sh_ms}ms) [mono: $([[ $mono_ok -eq 0 ]] && echo -e "${GREEN}PASS${NC}" || echo -e "${RED}FAIL${NC}")]"
        diff <(echo "$c_out") <(echo "$sh_out") | head -20
    else
        echo -e "${GREEN}PASS${NC}  (C: ${c_ms}ms, Bash: ${sh_ms}ms) [mono: ${RED}FAIL${NC}]"
    fi
    if [[ $mono_ok -ne 0 ]]; then
        echo "  mono diff:"
        diff <(echo "$c_mono_out") <(echo "$sh_mono_out") | head -20 | sed 's/^/  /'
    fi
    return 1
}

# Worker entry point: invoked as a fresh `bash` process (one per xargs slot)
# so scenarios run concurrently. Each worker gets its own tmp files under a
# job-specific subdirectory of the parent's TEST_TMPDIR, so parallel jobs
# (and separate concurrent `run_tests.sh` invocations, which each get their
# own TEST_TMPDIR) never share mutable state.
if [[ -n "$JOB_WORKER_MODE" ]]; then
    job_line=$(sed -n "$((JOB_IDX + 1))p" "$JOBS_TSV")
    IFS=$'\t' read -r suite _label scenario_name <<< "$job_line"

    job_root="$JOB_TMPDIR/job_${JOB_IDX}"
    mkdir -p "$job_root"
    export PERF_FILE="$JOB_TMPDIR/perf.tsv"

    scenario_output=$(run_scenario "$suite" "$scenario_name")
    rc=$?
    printf '%s\n' "$scenario_output" > "$JOB_TMPDIR/out_${JOB_IDX}.txt"
    echo "$rc" > "$JOB_TMPDIR/rc_${JOB_IDX}.txt"
    rm -rf "$job_root"
    exit 0
fi

SUITE_START_NS=$(date +%s%N)

echo "=== Terminal Tables Test Suite ==="
echo ""

if [[ ${#SUITES[@]} -eq 0 ]]; then
    SUITES=("00" "01" "02" "03" "04" "05" "06" "07" "08" "09")
fi

if [[ ! -f "$MANIFEST" ]]; then
    echo "ERROR: manifest.json not found at $MANIFEST"
    exit 1
fi

# Per-suite fail counts for the performance P/F column
declare -A SUITE_FAIL_COUNT=()

# Suite 00: Linting (special-cased; not in manifest)
for s in "${SUITES[@]}"; do
    if [[ "$s" == "00" ]]; then
        echo "--- Suite 00: Linting ---"
        if run_lint_suite; then
            pass_count=$((pass_count + 1))
            SUITE_FAIL_COUNT["00"]=0
        else
            fail_count=$((fail_count + 1))
            SUITE_FAIL_COUNT["00"]=1
        fi
        total_count=$((total_count + 1))
        echo ""
        break
    fi
done

suite_count=$(jq -r 'length' "$MANIFEST")

# --- Phase 1: build the job list (preserves manifest/suite order) ---------
build_current_suite=""
job_count=0
declare -a JOB_SUITE JOB_LABEL JOB_SUITE_NAME
JOBS_TSV="$TEST_TMPDIR/jobs.tsv"
: > "$JOBS_TSV"

for ((i=0; i<suite_count; i++)); do
    suite=$(jq -r ".[$i].suite" "$MANIFEST")
    label=$(jq -r ".[$i].label" "$MANIFEST")

    skip=true
    for s in "${SUITES[@]}"; do
        if [[ "$suite" == "$s" ]]; then
            skip=false
            break
        fi
    done

    [[ "$skip" == "true" ]] && continue

    scenario_name="test_$(echo "$label" | sed 's/-/_/')"

    if [[ "$suite" != "$build_current_suite" ]]; then
        build_current_suite="$suite"
        current_suite_name=$(jq -r ".[$i].suite_name" "$MANIFEST")
        current_suite_name="$(format_suite_name "$current_suite_name")"
        # Record suite display name once for the performance table
        if ! grep -q "^${suite}"$'\t' "$SUITE_NAMES_FILE" 2>/dev/null; then
            echo -e "${suite}\t${current_suite_name}" >> "$SUITE_NAMES_FILE"
        fi
        SUITE_FAIL_COUNT["$suite"]=${SUITE_FAIL_COUNT["$suite"]:-0}
    fi

    printf '%s\t%s\t%s\n' "$suite" "$label" "$scenario_name" >> "$JOBS_TSV"
    JOB_SUITE[$job_count]="$suite"
    JOB_LABEL[$job_count]="$label"
    JOB_SUITE_NAME[$job_count]="$current_suite_name"
    job_count=$((job_count + 1))
done

# --- Phase 2: run all jobs in parallel via xargs ---------------------------
if [[ $job_count -gt 0 ]]; then
    NPROC=$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
    [[ "$NPROC" -lt 1 ]] && NPROC=1

    seq 0 $((job_count - 1)) | xargs -P "$NPROC" -I{} \
        bash "$SCRIPT_PATH" --job-worker {} "$JOBS_TSV" "$TEST_TMPDIR"
fi

# --- Phase 3: replay results in original manifest/suite order -------------
current_suite=""
for ((idx=0; idx<job_count; idx++)); do
    suite="${JOB_SUITE[$idx]}"
    label="${JOB_LABEL[$idx]}"

    if [[ "$suite" != "$current_suite" ]]; then
        if [[ -n "$current_suite" ]]; then
            echo ""
        fi
        current_suite="$suite"
        echo "--- Suite ${suite}: ${JOB_SUITE_NAME[$idx]} ---"
    fi

    scenario_output=$(cat "$TEST_TMPDIR/out_${idx}.txt" 2>/dev/null)
    rc=$(cat "$TEST_TMPDIR/rc_${idx}.txt" 2>/dev/null || echo 2)
    echo "  $label: $scenario_output"

    if [[ "$rc" -eq 0 ]]; then
        pass_count=$((pass_count + 1))
    else
        fail_count=$((fail_count + 1))
        SUITE_FAIL_COUNT["$suite"]=$((${SUITE_FAIL_COUNT["$suite"]:-0} + 1))
    fi
    total_count=$((total_count + 1))
done

SUITE_END_NS=$(date +%s%N)
SUITE_WALL_MS=$(( (SUITE_END_NS - SUITE_START_NS) / 1000000 ))

echo ""
echo "=== Summary ==="
echo -e "${GREEN}Passed: $pass_count${NC}"
echo -e "${RED}Failed: $fail_count${NC}"
echo "Total: $total_count"

# Performance comparison table (dogfood the tables library)
if [[ -s "$PERF_FILE" && -x "$C_BIN" ]]; then
    declare -A SUITE_C_MS SUITE_SH_MS SUITE_LABEL
    while IFS=$'\t' read -r s c_ms sh_ms; do
        SUITE_C_MS[$s]=$((${SUITE_C_MS[$s]:-0} + c_ms))
        SUITE_SH_MS[$s]=$((${SUITE_SH_MS[$s]:-0} + sh_ms))
    done < "$PERF_FILE"

    while IFS=$'\t' read -r s name; do
        SUITE_LABEL[$s]="$name"
    done < "$SUITE_NAMES_FILE"

    total_c=0
    total_sh=0
    for s in "${!SUITE_C_MS[@]}"; do
        total_c=$((total_c + SUITE_C_MS[$s]))
        total_sh=$((total_sh + SUITE_SH_MS[$s]))
    done

    # P/F glyphs: single-width dingbats (no emoji background)
    PF_PASS="{GREEN}✓{NC}"
    PF_FAIL="{RED}✗{NC}"

    # Build ordered suite list from names file (run order)
    data_rows="["
    first=true
    while IFS=$'\t' read -r s name; do
        c_ms="${SUITE_C_MS[$s]:-0}"
        sh_ms="${SUITE_SH_MS[$s]:-0}"
        if [[ $c_ms -gt 0 ]]; then
            ratio=$(awk "BEGIN {printf \"%.1f x\", $sh_ms / $c_ms}")
        else
            ratio="—"
        fi

        # Color per-row: fastest language green, slowest red (scales to N langs later)
        local_colored=()
        mapfile -t local_colored < <(colorize_timings "$sh_ms" "$c_ms")
        sh_fmt="${local_colored[0]}"
        c_fmt="${local_colored[1]}"

        if [[ ${SUITE_FAIL_COUNT[$s]:-0} -eq 0 ]]; then
            pf="$PF_PASS"
        else
            pf="$PF_FAIL"
        fi

        if [[ "$first" != "true" ]]; then
            data_rows+=","
        fi
        first=false
        data_rows+=$(jq -nc \
            --arg suite "$s $name" \
            --arg pf "$pf" \
            --arg bash_time "$sh_fmt" \
            --arg c_time "$c_fmt" \
            --arg ratio "$ratio" \
            '{group:"suite",suite:$suite,pf:$pf,bash_time:$bash_time,c_time:$c_time,ratio:$ratio}')
    done < "$SUITE_NAMES_FILE"

    if [[ $total_c -gt 0 ]]; then
        total_ratio=$(awk "BEGIN {printf \"%.1f x\", $total_sh / $total_c}")
    else
        total_ratio="—"
    fi
    mapfile -t total_colored < <(colorize_timings "$total_sh" "$total_c")
    total_sh_fmt="${total_colored[0]}"
    total_c_fmt="${total_colored[1]}"
    if [[ $fail_count -eq 0 ]]; then
        total_pf="$PF_PASS"
    else
        total_pf="$PF_FAIL"
    fi
    data_rows+=","
    data_rows+=$(jq -nc \
        --arg pf "$total_pf" \
        --arg bash_time "$total_sh_fmt" \
        --arg c_time "$total_c_fmt" \
        --arg ratio "$total_ratio" \
        '{group:"total",suite:"Total",pf:$pf,bash_time:$bash_time,c_time:$c_time,ratio:$ratio}')

    # Informational annotated row (excluded from any future summary math)
    read -r bash_loc c_loc < <(count_loc)
    if [[ ${bash_loc:-0} -gt 0 || ${c_loc:-0} -gt 0 ]]; then
        if [[ $c_loc -gt 0 ]]; then
            loc_ratio=$(awk "BEGIN {printf \"%.1f x\", $bash_loc / $c_loc}")
        else
            loc_ratio="—"
        fi
        # Fewer lines = green, more = red
        bash_loc_num=$(format_int "$bash_loc")
        c_loc_num=$(format_int "$c_loc")
        if [[ $bash_loc -lt $c_loc ]]; then
            bash_loc_fmt="{GREEN}${bash_loc_num}{NC}"
            c_loc_fmt="{RED}${c_loc_num}{NC}"
        elif [[ $c_loc -lt $bash_loc ]]; then
            bash_loc_fmt="{RED}${bash_loc_num}{NC}"
            c_loc_fmt="{GREEN}${c_loc_num}{NC}"
        else
            bash_loc_fmt="{GREEN}${bash_loc_num}{NC}"
            c_loc_fmt="{GREEN}${c_loc_num}{NC}"
        fi
        data_rows+=","
        data_rows+=$(jq -nc \
            --arg bash_time "$bash_loc_fmt" \
            --arg c_time "$c_loc_fmt" \
            --arg ratio "$loc_ratio" \
            '{group:"loc",suite:"Lines of Code",pf:"",bash_time:$bash_time,c_time:$c_time,ratio:$ratio,annotate:true}')
    fi

    data_rows+="]"
    # Write to a per-run temp file first, then rename into place atomically so
    # concurrent `run_tests.sh` invocations never leave a partially-written or
    # torn performance_data.json/performance_layout.json for each other (or
    # for `--results`) to read.
    perf_data_tmp="${PERF_DATA_JSON}.${RUN_ID}.tmp"
    perf_layout_tmp="${PERF_LAYOUT_JSON}.${RUN_ID}.tmp"
    echo "$data_rows" | jq '.' > "$perf_data_tmp"
    mv -f "$perf_data_tmp" "$PERF_DATA_JSON"

    wall_fmt=$(format_ms "$SUITE_WALL_MS")
    cat > "$perf_layout_tmp" << LAYOUT
{
  "theme": "Red",
  "title": "Performance Comparison",
  "title_position": "center",
  "columns": [
    {"header": "Suite", "key": "suite", "datatype": "text", "justification": "left"},
    {"header": "P/F", "key": "pf", "datatype": "text", "justification": "center"},
    {"header": "Bash", "key": "bash_time", "datatype": "text", "justification": "right"},
    {"header": "C", "key": "c_time", "datatype": "text", "justification": "right"},
    {"header": "Bash / C", "key": "ratio", "datatype": "text", "justification": "right"},
    {"header": "group", "key": "group", "datatype": "text", "justification": "left", "visible": false, "break": true}
  ],
  "footer": "Test Suite Execution Time: {BOLD}{WHITE}${wall_fmt}{NC}",
  "footer_position": "center"
}
LAYOUT
    mv -f "$perf_layout_tmp" "$PERF_LAYOUT_JSON"

    echo ""
    "$C_BIN" "$PERF_LAYOUT_JSON" "$PERF_DATA_JSON" 2>/dev/null || true
fi

exit $fail_count
