#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

SCENARIOS_DIR="$SCRIPT_DIR/scenarios"
MANIFEST="$SCENARIOS_DIR/manifest.json"
PERF_DATA_JSON="$SCRIPT_DIR/performance_data.json"
PERF_LAYOUT_JSON="$SCRIPT_DIR/performance_layout.json"
IMPLEMENTATIONS_JSON="$SCRIPT_DIR/implementations.json"

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

# =============================================================================
# Implementation registry
# -----------------------------------------------------------------------------
# Every language/binary under test is described once in implementations.json
# (id, display name, run command template, lint tool, loc path) instead of
# being hard-coded here. Adding a new language later — Python, Lua, Rust,
# Go, ... — is just a new entry in that file; nothing below needs to change.
# One of the entries must be marked "reference": true (currently Bash, per
# AGENTS.md: it is the oracle every other implementation is compared
# against for correctness). One entry should be marked "baseline": true
# (currently C: it's expected to remain the fastest implementation, so
# every implementation's performance is reported relative to it, and it's
# used to render the performance table itself). Reference and baseline are
# independent — they happen to be different implementations today. See
# tests/README.md for the full schema.
# =============================================================================

declare -a IMPL_ID IMPL_NAME IMPL_REFERENCE IMPL_BASELINE IMPL_RUN_TOKENS IMPL_TIMEOUT
declare -a IMPL_LINT_NAME IMPL_LINT_TOKENS IMPL_LOC_PATH IMPL_LOC_LANGS
REFERENCE_IDX=-1
BASELINE_IDX=-1

# First run token, with {PROJECT_ROOT} expanded, decides how an
# implementation's availability is probed: an absolute path must be an
# executable file; a bare command name is looked up on PATH.
impl_available() {
    local tokens_joined="$1"
    local first="${tokens_joined%%$'\x1f'*}"
    first="${first//\{PROJECT_ROOT\}/$PROJECT_ROOT}"
    if [[ "${first}" == /* ]]; then
        [[ -x "${first}" ]]
    else
        command -v "${first}" >/dev/null 2>&1
    fi
}

# Expand a joined (unit-separator delimited) token list into CMD_ARR,
# substituting {PROJECT_ROOT}/{LAYOUT}/{DATA} and optionally appending
# --mono. Pure bash (no forks) since tokens are pre-split once at load time.
CMD_ARR=()
expand_cmd() {
    local tokens_joined="$1" layout="$2" data="$3" mono="$4"
    CMD_ARR=()
    if [[ -n "${tokens_joined}" ]]; then
        local -a raw=()
        IFS=$'\x1f' read -ra raw <<< "${tokens_joined}"
        local tok
        for tok in "${raw[@]}"; do
            tok="${tok//\{PROJECT_ROOT\}/$PROJECT_ROOT}"
            tok="${tok//\{LAYOUT\}/$layout}"
            tok="${tok//\{DATA\}/$data}"
            CMD_ARR+=("${tok}")
        done
    fi
    [[ "${mono}" == "1" ]] && CMD_ARR+=("--mono")
}

# Single jq invocation for the whole registry (one @tsv line per
# implementation, with array fields join(0x1f)-encoded) instead of one jq
# fork per field per implementation.
load_implementations() {
    if [[ ! -f "${IMPLEMENTATIONS_JSON}" ]]; then
        echo "ERROR: implementations config not found at ${IMPLEMENTATIONS_JSON}" >&2
        exit 1
    fi

    local -a lines=()
    mapfile -t lines < <(jq -r '
        .[] | [
            .id,
            .name,
            ((.reference // false) | tostring),
            ((.baseline // false) | tostring),
            (.run | join("\u001f")),
            ((.timeout // 30) | tostring),
            (.lint.name // ""),
            ((.lint.cmd // []) | join("\u001f")),
            (.loc.path // ""),
            (.loc.cloc_langs // "")
        ] | @tsv
    ' "${IMPLEMENTATIONS_JSON}")

    local idx=0 line
    for line in "${lines[@]}"; do
        local -a f=()
        mapfile -t -d $'\t' f <<< "${line}"
        f[9]="${f[9]%$'\n'}"

        if ! impl_available "${f[4]}"; then
            echo -e "${YELLOW}Warning: implementation '${f[1]}' unavailable, skipping${NC}" >&2
            continue
        fi

        IMPL_ID[idx]="${f[0]}"
        IMPL_NAME[idx]="${f[1]}"
        IMPL_REFERENCE[idx]="${f[2]}"
        IMPL_BASELINE[idx]="${f[3]}"
        IMPL_RUN_TOKENS[idx]="${f[4]}"
        IMPL_TIMEOUT[idx]="${f[5]}"
        IMPL_LINT_NAME[idx]="${f[6]}"
        IMPL_LINT_TOKENS[idx]="${f[7]}"
        IMPL_LOC_PATH[idx]="${f[8]}"
        IMPL_LOC_LANGS[idx]="${f[9]}"
        [[ "${IMPL_REFERENCE[idx]}" == "true" ]] && REFERENCE_IDX=${idx}
        [[ "${IMPL_BASELINE[idx]}" == "true" ]] && BASELINE_IDX=${idx}
        idx=$((idx + 1))
    done

    if [[ ${REFERENCE_IDX} -lt 0 ]]; then
        echo "ERROR: no available implementation is marked \"reference\": true in ${IMPLEMENTATIONS_JSON}" >&2
        exit 1
    fi
    if [[ ${BASELINE_IDX} -lt 0 ]]; then
        # No implementation is unavailable/unmarked as baseline: fall back to
        # the reference so the table/rendering still work, just without a
        # meaningful performance baseline.
        BASELINE_IDX=${REFERENCE_IDX}
    fi
}

load_implementations
IMPL_COUNT=${#IMPL_ID[@]}

# Render the performance table itself with the fastest implementation
# (baseline, i.e. C) rather than the reference (Bash), which is orders of
# magnitude slower and would make `--results` (and the final table at the
# end of every run) needlessly slow.
render_table() {
    local layout="$1" data="$2"
    expand_cmd "${IMPL_RUN_TOKENS[${BASELINE_IDX}]}" "${layout}" "${data}" 0
    if "${CMD_ARR[@]}"; then
        return 0
    fi
    # Baseline failed/unavailable at runtime: fall back to any other
    # available implementation so results are still shown.
    local i
    for ((i=0; i<IMPL_COUNT; i++)); do
        [[ $i -eq ${BASELINE_IDX} ]] && continue
        expand_cmd "${IMPL_RUN_TOKENS[$i]}" "${layout}" "${data}" 0
        "${CMD_ARR[@]}" && return 0
    done
    return 1
}

show_saved_results() {
    if [[ ! -f "${PERF_LAYOUT_JSON}" || ! -f "${PERF_DATA_JSON}" ]]; then
        echo "No saved performance results found."
        echo "Run tests first (e.g. bash tests/run_tests.sh 01), then:"
        echo "  bash tests/run_tests.sh --results"
        exit 1
    fi
    render_table "${PERF_LAYOUT_JSON}" "${PERF_DATA_JSON}"
    exit $?
}

# Parse flags; remaining args are suite filters (or test labels, e.g. "9-V")
SUITES=()
SHOW_LABELS=()
SHOW_MONO=0
if [[ -z "${JOB_WORKER_MODE}" ]]; then
    for arg in "$@"; do
        case "${arg}" in
            --results|-r)
                show_saved_results
                ;;
            --mono)
                SHOW_MONO=1
                ;;
            --help|-h)
                cat <<'USAGE'
Usage: run_tests.sh [OPTIONS] [SUITE ... | LABEL ...]

  Run comparison tests for the given suites (default: 00–09).

  Implementations under test are defined in tests/implementations.json
  (id, display name, run command, lint tool, lines-of-code path). Add a
  new language by adding an entry there; this script does not hard-code
  any particular language.

  Passing one or more specific test labels (containing a dash, e.g.
  "9-V") instead of suite numbers switches to "show" mode: instead of
  the normal pass/fail comparison and performance table, it prints that
  scenario's actual rendered output from every configured implementation
  side by side, so you can eyeball what "9-V" looks like in C vs. Bash.

Options:
  --results, -r   Re-display the performance table from the last run
  --mono          In show mode, render with --mono instead of colors
  --help, -h      Show this help

Examples:
  bash tests/run_tests.sh
  bash tests/run_tests.sh 01 04
  bash tests/run_tests.sh --results
  bash tests/run_tests.sh 9-V
  bash tests/run_tests.sh 9-V 5-A --mono
USAGE
                exit 0
                ;;
            -*)
                echo "Unknown option: ${arg} (try --help)"
                exit 1
                ;;
            *)
                if [[ "${arg}" == *-* ]]; then
                    SHOW_LABELS+=("${arg}")
                else
                    SUITES+=("${arg}")
                fi
                ;;
        esac
    done
fi

pass_count=0
fail_count=0
total_count=0

if [[ -z "${JOB_WORKER_MODE}" ]]; then
    RUN_ID="$$_${RANDOM}${RANDOM}"
    TEST_TMPDIR=$(mktemp -d)
    PERF_FILE="$TEST_TMPDIR/perf.tsv"
    export PERF_FILE
    touch "${PERF_FILE}"
    SUITE_NAMES_FILE="$TEST_TMPDIR/suite_names.tsv"
    touch "${SUITE_NAMES_FILE}"

    cleanup() { rm -rf "${TEST_TMPDIR}"; }
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

# "N.Nx" ratio of one value against the baseline implementation's value
# (used for the performance table's "<Name> / <Baseline>" columns and the
# LOC row). C is the baseline: it's expected to remain the fastest
# implementation, so every other implementation is reported as a multiple
# of it.
ratio_vs_baseline() {
    local value="$1" baseline="$2"
    if [[ ${baseline} -gt 0 ]]; then
        awk "BEGIN {printf \"%.1f x\", $value / $baseline}"
    else
        echo "—"
    fi
}

# cloc code-line count for one implementation's configured loc.path
# (optionally filtered to loc.cloc_langs). Returns 0 if cloc/path/config
# is unavailable so the LOC row degrades gracefully.
count_loc_for_impl() {
    local idx="$1"
    local path="${IMPL_LOC_PATH[$idx]}"
    [[ -z "${path}" ]] && { echo 0; return; }
    command -v cloc >/dev/null 2>&1 || { echo 0; return; }
    local langs="${IMPL_LOC_LANGS[$idx]}" loc
    if [[ -n "${langs}" ]]; then
        loc=$(cloc --csv --quiet --include-lang="${langs}" "${PROJECT_ROOT}/${path}" 2>/dev/null \
            | awk -F, '$2=="SUM"{print $5; exit}')
    else
        loc=$(cloc --csv --quiet "${PROJECT_ROOT}/${path}" 2>/dev/null \
            | awk -F, '$2=="SUM"{print $5; exit}')
    fi
    echo "${loc:-0}"
}

# Color a list of ms values: fastest = {GREEN}, slowest = {RED}. Takes any
# number of values; prints one colored "N ms" string per input on its own
# line, in input order. Equal values all get green.
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
            # Middle values (3+ implementations) stay uncolored
            echo "$label"
        fi
    done
}

# Same coloring rule as colorize_timings but for plain integers (LOC counts:
# fewest = green, most = red) rather than "N ms" labels.
colorize_counts() {
    local -a vals=("$@")
    local n=${#vals[@]}
    [[ $n -eq 0 ]] && return
    local min="${vals[0]}" max="${vals[0]}" v
    for v in "${vals[@]}"; do
        [[ $v -lt $min ]] && min=$v
        [[ $v -gt $max ]] && max=$v
    done
    for v in "${vals[@]}"; do
        local label
        label=$(format_int "$v")
        if [[ $min -eq $max ]]; then
            echo "{GREEN}${label}{NC}"
        elif [[ $v -eq $min ]]; then
            echo "{GREEN}${label}{NC}"
        elif [[ $v -eq $max ]]; then
            echo "{RED}${label}{NC}"
        else
            echo "$label"
        fi
    done
}

run_lint_suite() {
    local issues=0
    local -a ms=()
    local i

    for ((i=0; i<IMPL_COUNT; i++)); do
        ms[i]=0
        [[ -z "${IMPL_LINT_NAME[$i]}" ]] && continue

        echo -n "  ${IMPL_LINT_NAME[$i]} (${IMPL_NAME[$i]}): "
        expand_cmd "${IMPL_LINT_TOKENS[$i]}" "" "" 0
        local tool="${CMD_ARR[0]:-}"
        if [[ -z "${tool}" ]] || ! command -v "${tool}" >/dev/null 2>&1; then
            echo -e "${YELLOW}SKIP${NC} (${tool:-lint tool} not installed)"
            continue
        fi

        local out rc start end
        rc=0
        start=$(date +%s%N)
        out=$("${CMD_ARR[@]}" 2>&1) || rc=$?
        end=$(date +%s%N)
        ms[i]=$(( (end - start) / 1000000 ))

        if [[ $rc -eq 0 ]]; then
            echo -e "${GREEN}PASS${NC}  (${ms[$i]}ms)"
        else
            local count
            count=$(echo "$out" | grep -c . || true)
            echo -e "${RED}FAIL${NC}  (${ms[$i]}ms, $count issue(s))"
            echo "$out" | head -20 | sed 's/^/    /'
            issues=$((issues + 1))
        fi
    done

    # Record lint timings for the performance table, one line per implementation
    for ((i=0; i<IMPL_COUNT; i++)); do
        echo -e "00\t${IMPL_ID[$i]}\t${ms[$i]:-0}" >> "${PERF_FILE}"
    done
    if ! grep -q $'^00\t' "${SUITE_NAMES_FILE}" 2>/dev/null; then
        echo -e "00\tLinting" >> "${SUITE_NAMES_FILE}"
    fi

    [[ ${issues} -eq 0 ]]
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

    # Run every configured implementation directly (no intermediate wrapper
    # script) so timing reflects the implementation itself, not extra forks.
    local -a ms=() out_color=() out_mono=()
    local i
    for ((i=0; i<IMPL_COUNT; i++)); do
        expand_cmd "${IMPL_RUN_TOKENS[$i]}" "${tmp_layout}" "${tmp_data}" 0
        local start end raw
        start=$(date +%s%N)
        raw=$(timeout "${IMPL_TIMEOUT[$i]}" "${CMD_ARR[@]}" 2>&1) || true
        end=$(date +%s%N)
        ms[i]=$(( (end - start) / 1000000 ))
        out_color[i]=$(normalize_output "${raw}")

        # --mono variant: not timed for the performance table, only
        # validated for correctness against the reference implementation.
        expand_cmd "${IMPL_RUN_TOKENS[$i]}" "${tmp_layout}" "${tmp_data}" 1
        local mraw
        mraw=$(timeout "${IMPL_TIMEOUT[$i]}" "${CMD_ARR[@]}" 2>&1) || true
        out_mono[i]=$(normalize_output "${mraw}")

        echo -e "${suite}\t${IMPL_ID[$i]}\t${ms[$i]}" >> "${PERF_FILE}"
    done

    rm -f "$tmp_data" "$tmp_layout"

    # Every non-reference implementation is compared against the reference
    # (Bash — see AGENTS.md) for both the colored and --mono output.
    local ref=${REFERENCE_IDX}
    local -a fail_ids=()
    for ((i=0; i<IMPL_COUNT; i++)); do
        [[ $i -eq ${ref} ]] && continue
        if [[ "${out_color[$i]}" != "${out_color[$ref]}" || "${out_mono[$i]}" != "${out_mono[$ref]}" ]]; then
            fail_ids+=("$i")
        fi
    done

    local summary=""
    for ((i=0; i<IMPL_COUNT; i++)); do
        [[ -n "${summary}" ]] && summary+=", "
        summary+="${IMPL_NAME[$i]}: ${ms[$i]}ms"
    done

    if [[ ${#fail_ids[@]} -eq 0 ]]; then
        echo -e "${GREEN}PASS${NC}  (${summary}) [mono: ${GREEN}PASS${NC}]"
        return 0
    fi

    local names=""
    for i in "${fail_ids[@]}"; do
        [[ -n "${names}" ]] && names+=", "
        names+="${IMPL_NAME[$i]}"
    done
    echo -e "${RED}DIFF${NC}  (${summary}) — diverges from ${IMPL_NAME[$ref]}: ${names}"
    for i in "${fail_ids[@]}"; do
        if [[ "${out_color[$i]}" != "${out_color[$ref]}" ]]; then
            echo "  ${IMPL_NAME[$i]} vs ${IMPL_NAME[$ref]} (color):"
            diff <(echo "${out_color[$ref]}") <(echo "${out_color[$i]}") | head -20 | sed 's/^/    /'
        fi
        if [[ "${out_mono[$i]}" != "${out_mono[$ref]}" ]]; then
            echo "  ${IMPL_NAME[$i]} vs ${IMPL_NAME[$ref]} (mono):"
            diff <(echo "${out_mono[$ref]}") <(echo "${out_mono[$i]}") | head -20 | sed 's/^/    /'
        fi
    done
    return 1
}

# Show mode: given a specific test label (e.g. "9-V", detected by the
# presence of a dash in a positional arg), print that scenario's actual
# rendered output from every configured implementation, one after another,
# instead of running the normal pass/fail comparison + performance table.
# Useful for eyeballing "what does 9-V look like in C vs. Bash?".
show_single_label() {
    local label="$1"
    local match suite
    match=$(jq -r --arg label "${label}" '.[] | select(.label == $label) | .suite' "${MANIFEST}")
    if [[ -z "${match}" ]]; then
        echo -e "${RED}No scenario found for test '${label}' (check tests/scenarios/manifest.json)${NC}" >&2
        return 1
    fi
    suite=$(echo "${match}" | head -1)

    local suite_dir="$SCENARIOS_DIR/suite_${suite}"
    local scenario_name="test_$(echo "${label}" | sed 's/-/_/')"
    local data_file layout_file
    data_file=$(ls "$suite_dir/${scenario_name}_data".* 2>/dev/null | head -1)
    layout_file=$(ls "$suite_dir/${scenario_name}_layout".* 2>/dev/null | head -1)

    if [[ -z "${data_file}" || -z "${layout_file}" ]]; then
        echo -e "${RED}Scenario files not found for '${label}' in ${suite_dir}${NC}" >&2
        return 1
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

    echo -e "${YELLOW}=== Test ${label} (suite ${suite}) ===${NC}"
    local i
    for ((i=0; i<IMPL_COUNT; i++)); do
        echo ""
        echo -e "${YELLOW}--- ${IMPL_NAME[$i]} ---${NC}"
        expand_cmd "${IMPL_RUN_TOKENS[$i]}" "${tmp_layout}" "${tmp_data}" "${SHOW_MONO}"
        timeout "${IMPL_TIMEOUT[$i]}" "${CMD_ARR[@]}"
    done
    echo ""

    rm -f "$tmp_data" "$tmp_layout"
    return 0
}

if [[ -z "${JOB_WORKER_MODE}" && ${#SHOW_LABELS[@]} -gt 0 ]]; then
    show_rc=0
    for label in "${SHOW_LABELS[@]}"; do
        show_single_label "${label}" || show_rc=1
    done
    exit ${show_rc}
fi

# Worker entry point: invoked as a fresh `bash` process (one per xargs slot)
# so scenarios run concurrently. Each worker gets its own tmp files under a
# job-specific subdirectory of the parent's TEST_TMPDIR, so parallel jobs
# (and separate concurrent `run_tests.sh` invocations, which each get their
# own TEST_TMPDIR) never share mutable state.
if [[ -n "${JOB_WORKER_MODE}" ]]; then
    job_line=$(sed -n "$((JOB_IDX + 1))p" "${JOBS_TSV}")
    IFS=$'\t' read -r suite _label scenario_name <<< "${job_line}"

    job_root="$JOB_TMPDIR/job_${JOB_IDX}"
    mkdir -p "${job_root}"
    export PERF_FILE="$JOB_TMPDIR/perf.tsv"

    scenario_output=$(run_scenario "${suite}" "${scenario_name}")
    rc=$?
    printf '%s\n' "${scenario_output}" > "$JOB_TMPDIR/out_${JOB_IDX}.txt"
    echo "${rc}" > "$JOB_TMPDIR/rc_${JOB_IDX}.txt"
    rm -rf "${job_root}"
    exit 0
fi

SUITE_START_NS=$(date +%s%N)

echo "=== Terminal Tables Test Suite ==="
echo ""
echo "Implementations: $(IFS=', '; echo "${IMPL_NAME[*]}") (reference: ${IMPL_NAME[$REFERENCE_IDX]})"
echo ""

if [[ ${#SUITES[@]} -eq 0 ]]; then
    SUITES=("00" "01" "02" "03" "04" "05" "06" "07" "08" "09")
fi

if [[ ! -f "${MANIFEST}" ]]; then
    echo "ERROR: manifest.json not found at ${MANIFEST}"
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

suite_count=$(jq -r 'length' "${MANIFEST}")

# --- Phase 1: build the job list (preserves manifest/suite order) ---------
build_current_suite=""
job_count=0
declare -a JOB_SUITE JOB_LABEL JOB_SUITE_NAME
JOBS_TSV="$TEST_TMPDIR/jobs.tsv"
: > "${JOBS_TSV}"

for ((i=0; i<suite_count; i++)); do
    suite=$(jq -r ".[$i].suite" "${MANIFEST}")
    label=$(jq -r ".[$i].label" "${MANIFEST}")

    skip=true
    for s in "${SUITES[@]}"; do
        if [[ "${suite}" == "$s" ]]; then
            skip=false
            break
        fi
    done

    [[ "${skip}" == "true" ]] && continue

    scenario_name="test_$(echo "${label}" | sed 's/-/_/')"

    if [[ "${suite}" != "${build_current_suite}" ]]; then
        build_current_suite="${suite}"
        current_suite_name=$(jq -r ".[$i].suite_name" "${MANIFEST}")
        current_suite_name="$(format_suite_name "${current_suite_name}")"
        # Record suite display name once for the performance table
        if ! grep -q "^${suite}"$'\t' "${SUITE_NAMES_FILE}" 2>/dev/null; then
            echo -e "${suite}\t${current_suite_name}" >> "${SUITE_NAMES_FILE}"
        fi
        SUITE_FAIL_COUNT["${suite}"]=${SUITE_FAIL_COUNT["${suite}"]:-0}
    fi

    printf '%s\t%s\t%s\n' "${suite}" "${label}" "${scenario_name}" >> "${JOBS_TSV}"
    JOB_SUITE[$job_count]="${suite}"
    JOB_LABEL[$job_count]="${label}"
    JOB_SUITE_NAME[$job_count]="${current_suite_name}"
    job_count=$((job_count + 1))
done

# --- Phase 2: run all jobs in parallel via xargs ---------------------------
if [[ $job_count -gt 0 ]]; then
    NPROC=$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
    [[ "${NPROC}" -lt 1 ]] && NPROC=1

    seq 0 $((job_count - 1)) | xargs -P "${NPROC}" -I{} \
        bash "${SCRIPT_PATH}" --job-worker {} "${JOBS_TSV}" "${TEST_TMPDIR}"
fi

# --- Phase 3: replay results in original manifest/suite order -------------
current_suite=""
for ((idx=0; idx<job_count; idx++)); do
    suite="${JOB_SUITE[$idx]}"
    label="${JOB_LABEL[$idx]}"

    if [[ "${suite}" != "${current_suite}" ]]; then
        if [[ -n "${current_suite}" ]]; then
            echo ""
        fi
        current_suite="${suite}"
        echo "--- Suite ${suite}: ${JOB_SUITE_NAME[$idx]} ---"
    fi

    scenario_output=$(cat "$TEST_TMPDIR/out_${idx}.txt" 2>/dev/null)
    rc=$(cat "$TEST_TMPDIR/rc_${idx}.txt" 2>/dev/null || echo 2)
    echo "  $label: ${scenario_output}"

    if [[ "${rc}" -eq 0 ]]; then
        pass_count=$((pass_count + 1))
    else
        fail_count=$((fail_count + 1))
        SUITE_FAIL_COUNT["${suite}"]=$((${SUITE_FAIL_COUNT["${suite}"]:-0} + 1))
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

# Performance comparison table (dogfood the tables library). One time
# column per configured+available implementation, built dynamically so
# adding a language to implementations.json needs no changes here.
if [[ -s "${PERF_FILE}" ]]; then
    # SUITE_IMPL_MS["<suite>|<impl_id>"] = summed ms across that suite's scenarios
    declare -A SUITE_IMPL_MS=()
    while IFS=$'\t' read -r s impl ms_val; do
        key="${s}|${impl}"
        SUITE_IMPL_MS[$key]=$(( ${SUITE_IMPL_MS[$key]:-0} + ms_val ))
    done < "${PERF_FILE}"

    declare -A SUITE_LABEL=()
    while IFS=$'\t' read -r s name; do
        SUITE_LABEL[$s]="$name"
    done < "${SUITE_NAMES_FILE}"

    declare -a TOTAL_IMPL_MS=()
    for ((i=0; i<IMPL_COUNT; i++)); do TOTAL_IMPL_MS[i]=0; done

    # P/F glyphs: single-width dingbats (no emoji background)
    PF_PASS="{GREEN}✓{NC}"
    PF_FAIL="{RED}✗{NC}"

    # Build one jq row object per suite from arbitrary (key, value) pairs —
    # constructed dynamically since the column count depends on how many
    # implementations are configured/available.
    build_row() {
        local group="$1" suite="$2" pf="$3"; shift 3
        local -a jq_args=(--arg group "$group" --arg suite "$suite" --arg pf "$pf")
        local filter='{group:$group,suite:$suite,pf:$pf'
        while [[ $# -gt 0 ]]; do
            local key="$1" val="$2"; shift 2
            jq_args+=(--arg "$key" "$val")
            filter+=",${key}:\$${key}"
        done
        filter+='}'
        jq -nc "${jq_args[@]}" "$filter"
    }

    # Append one r_<id> ratio-vs-baseline pair per non-baseline implementation
    # to the "pairs" array (a nameref target), given a parallel array of ms
    # (or LOC) values indexed the same as IMPL_*.
    append_ratio_pairs() {
        local -n _pairs="$1"; shift
        local -a vals=("$@")
        local baseline_val="${vals[${BASELINE_IDX}]}"
        local i
        for ((i=0; i<IMPL_COUNT; i++)); do
            [[ $i -eq ${BASELINE_IDX} ]] && continue
            _pairs+=("r_${IMPL_ID[$i]}" "$(ratio_vs_baseline "${vals[$i]}" "${baseline_val}")")
        done
    }

    data_rows="["
    first=true
    while IFS=$'\t' read -r s name; do
        row_ms=(); pairs=()
        for ((i=0; i<IMPL_COUNT; i++)); do
            v="${SUITE_IMPL_MS["${s}|${IMPL_ID[$i]}"]:-0}"
            row_ms[i]="$v"
            TOTAL_IMPL_MS[i]=$(( TOTAL_IMPL_MS[i] + v ))
        done

        mapfile -t row_colored < <(colorize_timings "${row_ms[@]}")
        for ((i=0; i<IMPL_COUNT; i++)); do
            pairs+=("t_${IMPL_ID[$i]}" "${row_colored[$i]}")
        done
        append_ratio_pairs pairs "${row_ms[@]}"

        if [[ ${SUITE_FAIL_COUNT[$s]:-0} -eq 0 ]]; then pf="$PF_PASS"; else pf="$PF_FAIL"; fi

        [[ "${first}" != "true" ]] && data_rows+=","
        first=false
        data_rows+=$(build_row "suite" "$s $name" "$pf" "${pairs[@]}")
    done < "${SUITE_NAMES_FILE}"

    mapfile -t total_colored < <(colorize_timings "${TOTAL_IMPL_MS[@]}")
    total_pairs=()
    for ((i=0; i<IMPL_COUNT; i++)); do
        total_pairs+=("t_${IMPL_ID[$i]}" "${total_colored[$i]}")
    done
    append_ratio_pairs total_pairs "${TOTAL_IMPL_MS[@]}"
    if [[ $fail_count -eq 0 ]]; then total_pf="$PF_PASS"; else total_pf="$PF_FAIL"; fi
    data_rows+=","
    data_rows+=$(build_row "total" "Total" "$total_pf" "${total_pairs[@]}")

    # Informational annotated LOC row (excluded from any future summary math)
    declare -a loc_vals=()
    loc_any=0
    for ((i=0; i<IMPL_COUNT; i++)); do
        loc_vals[i]=$(count_loc_for_impl "$i")
        [[ ${loc_vals[$i]} -gt 0 ]] && loc_any=1
    done
    if [[ ${loc_any} -eq 1 ]]; then
        mapfile -t loc_colored < <(colorize_counts "${loc_vals[@]}")
        loc_pairs=()
        for ((i=0; i<IMPL_COUNT; i++)); do
            loc_pairs+=("t_${IMPL_ID[$i]}" "${loc_colored[$i]}")
        done
        append_ratio_pairs loc_pairs "${loc_vals[@]}"
        data_rows+=","
        data_rows+=$(build_row "loc" "Lines of Code" "" "${loc_pairs[@]}")
    fi

    data_rows+="]"
    # Write to a per-run temp file first, then rename into place atomically so
    # concurrent `run_tests.sh` invocations never leave a partially-written or
    # torn performance_data.json/performance_layout.json for each other (or
    # for `--results`) to read.
    perf_data_tmp="${PERF_DATA_JSON}.${RUN_ID}.tmp"
    perf_layout_tmp="${PERF_LAYOUT_JSON}.${RUN_ID}.tmp"
    echo "$data_rows" | jq '.' > "${perf_data_tmp}"
    mv -f "${perf_data_tmp}" "${PERF_DATA_JSON}"

    wall_fmt=$(format_ms "$SUITE_WALL_MS")

    # Build the columns array dynamically: Suite, P/F, one time column per
    # implementation (in registry order), one "<Name> / <Baseline>" ratio
    # column per non-baseline implementation, then a hidden break column.
    baseline_name="${IMPL_NAME[${BASELINE_IDX}]}"
    impl_columns=""
    for ((i=0; i<IMPL_COUNT; i++)); do
        impl_columns+="    {\"header\": \"${IMPL_NAME[$i]}\", \"key\": \"t_${IMPL_ID[$i]}\", \"datatype\": \"text\", \"justification\": \"right\"},"$'\n'
    done
    ratio_columns=""
    for ((i=0; i<IMPL_COUNT; i++)); do
        [[ $i -eq ${BASELINE_IDX} ]] && continue
        ratio_columns+="    {\"header\": \"${IMPL_NAME[$i]} / ${baseline_name}\", \"key\": \"r_${IMPL_ID[$i]}\", \"datatype\": \"text\", \"justification\": \"right\"},"$'\n'
    done

    cat > "${perf_layout_tmp}" << LAYOUT
{
  "theme": "Red",
  "title": "Performance Comparison",
  "title_position": "center",
  "columns": [
    {"header": "Suite", "key": "suite", "datatype": "text", "justification": "left"},
    {"header": "P/F", "key": "pf", "datatype": "text", "justification": "center"},
${impl_columns}${ratio_columns}    {"header": "group", "key": "group", "datatype": "text", "justification": "left", "visible": false, "break": true}
  ],
  "footer": "Test Suite Execution Time: {BOLD}{WHITE}${wall_fmt}{NC}",
  "footer_position": "center"
}
LAYOUT
    mv -f "${perf_layout_tmp}" "${PERF_LAYOUT_JSON}"

    echo ""
    render_table "${PERF_LAYOUT_JSON}" "${PERF_DATA_JSON}" 2>/dev/null || true
fi

exit $fail_count
