# Test Suite

This directory contains the comparison test suite for the Terminal Tables implementations.

## Overview

The test suite runs every configured implementation (see `tests/implementations.json`) against the same JSON data and layout files, and compares their output including ANSI color sequences (after normalizing dynamic content only). Implementations are **not** hard-coded into `run_tests.sh` — the runner reads `tests/implementations.json` to discover which languages/binaries to test, run, lint, and count lines of code for, so the performance table and comparisons automatically grow to fit however many implementations are configured.

**Bash is the reference implementation** for correctness. The Bash implementation was the original variant created; all other implementations (C, and any future ports) are compared *against* Bash output as the oracle. Exactly one entry in `implementations.json` is marked `"reference": true`; every other configured implementation's output (colored and `--mono`) is diffed against it.

**C is the performance baseline.** C is expected to remain the fastest implementation, so the performance table reports every other implementation's time as a multiple of C's (`"<Name> / C"` columns), and C is also used to render the performance table itself (rendering it with Bash would make every run needlessly slow). Exactly one entry is marked `"baseline": true`. Reference and baseline are independent settings that happen to point at different implementations today.

## Directory Structure

```
tests/
├── run_tests.sh              # Shell-based test runner (implementation-agnostic)
├── implementations.json      # Registry of implementations under test
├── README.md                 # This documentation
└── scenarios/
    ├── manifest.json         # Master manifest of all comparison test cases
    ├── suite_01/             # Basic datatypes and justifications
    ├── suite_02/             # Summaries (sum, min, max, avg, count, unique)
    ├── suite_03/             # Text wrapping modes
    ├── suite_04/             # Complex tables with mixed features
    ├── suite_05/             # Title rendering and positioning
    ├── suite_06/             # Title position clipping
    ├── suite_07/             # Footer rendering and positioning
    ├── suite_08/             # Footer position clipping
    └── suite_09/             # Showcase with multiple tables
```

Suite 00 (Linting) runs each implementation's configured `lint.cmd` from `implementations.json` (currently shellcheck on `tables.sh/tables.sh` and cppcheck on `tables.c/`). Pass = zero issues; fail = non-zero exit. An implementation without a `lint` entry, or whose lint tool isn't installed, is skipped for linting only.

### Scenario Files

Each test case is defined by a pair of files in the appropriate `suite_XX/` directory:

- `test_X_Y_data.json` — JSON array of data rows
- `test_X_Y_layout.json` — JSON layout definition

For dynamic content (timestamps, computed counts), use `_layout.sh` instead:

- `test_X_Y_layout.sh` — Bash script that outputs JSON (supports `$(date)`, `$(jq)`, etc.)

The `manifest.json` file lists all test cases with their suite, label, and file names.

## Running Tests

### Shell Runner

```bash
bash tests/run_tests.sh           # Run all suites (00–09)
bash tests/run_tests.sh 00        # Linting only (shellcheck + cppcheck)
bash tests/run_tests.sh 01 05 09  # Run specific suites only
bash tests/run_tests.sh --results # Re-show performance table from last run
```

After a run that includes comparison suites, timing data is saved to:

- `tests/performance_data.json`
- `tests/performance_layout.json`

The performance table includes one time column per implementation, one
`"<Name> / C"` ratio column per non-baseline implementation, a Total row,
an annotated **Lines of Code** row (from `cloc`; excluded from any
summary math via `"annotate": true`), and an annotated **Solution Size**
row (total on-disk bytes of each implementation's configured `size.paths`,
also excluded from summary math). Use `--results` (or `-r`) to render
that table again without re-running tests — this renders with the baseline
implementation (C), so it's fast regardless of how slow other
implementations are.

### CI

The GitHub Actions workflow (`.github/workflows/main.yml`) automatically builds the C binary and runs the comparison tests on every push and pull request.

## Adding New Test Cases

1. Create a new data file: `tests/scenarios/suite_XX/test_X_Y_data.json`
2. Create a new layout file: `tests/scenarios/suite_XX/test_X_Y_layout.json`
3. Add an entry to `tests/scenarios/manifest.json`:

   ```json
   {"suite":"01","label":"1-J","data_file":"test_1_J_data.json","layout_file":"test_1_J_layout.json"}
   ```

## Adding New Language Implementations

**Bash is the reference oracle.** When adding a new language implementation, it must match the Bash output (after normalization) for every scenario. `run_tests.sh` has no per-language code — implementations are declared entirely in `tests/implementations.json`. To add a language (e.g. Python, Lua, Rust, Go):

1. Build/implement it wherever makes sense (e.g. `tables.py/tables.py`) so it accepts `layout.json data.json [--mono]` like the existing implementations.
2. Append an entry to `tests/implementations.json`:

   ```json
   {
     "id": "python",
     "name": "Python",
     "run": ["python3", "{PROJECT_ROOT}/tables.py/tables.py", "{LAYOUT}", "{DATA}"],
     "timeout": 30,
     "lint": {"name": "ruff", "cmd": ["ruff", "check", "{PROJECT_ROOT}/tables.py"]},
     "loc": {"path": "tables.py", "cloc_langs": "Python"},
     "size": {"paths": ["tables.py/tables.py"]}
   }
   ```

   `size.paths` is a list because a solution isn't always one file — if it's
   made up of a main script plus helper libraries/modules, list every file
   that's part of the deployed solution so the Solution Size row reflects
   the whole thing, not just the entry point.
3. That's it — `run_tests.sh` will run it against every scenario, diff its output (color and `--mono`) against the reference, lint it (if `lint` is set and the tool is installed), include it in the Lines of Code row (if `loc` is set and `cloc` is installed), include it in the Solution Size row (if `size.paths` is set), and add both a time column and a `"<Name> / C"` ratio column for it to the performance table automatically.

If the new implementation ever benchmarks faster than C on the full suite, that's worth investigating (and possibly moving `"baseline": true` to it) rather than assuming C has become the slow one — see AGENTS.md.

### `implementations.json` schema

Each array entry describes one implementation:

| Field           | Required | Meaning                                                                                          |
|-----------------|----------|---------------------------------------------------------------------------------------------------|
| `id`            | yes      | Short key used internally (perf-table column key, PERF_FILE rows). No spaces.                     |
| `name`          | yes      | Display name (performance table header, scenario summary line).                                   |
| `reference`     | no       | `true` on exactly one entry — the correctness oracle every other implementation is diffed against. |
| `baseline`      | no       | `true` on exactly one entry — the performance baseline (currently C). Every implementation's time is reported as `"<Name> / <Baseline>"`, and the baseline renders the performance table itself. Falls back to the reference if unset. |
| `run`           | yes      | Argv array to execute. Supports `{PROJECT_ROOT}`, `{LAYOUT}`, `{DATA}` placeholders; `--mono` is appended automatically for the mono comparison pass. |
| `timeout`       | no       | Per-invocation timeout in seconds (default 30).                                                    |
| `lint.name`     | no       | Display name for the lint tool (suite 00 output line).                                             |
| `lint.cmd`      | no       | Argv array for the lint command (same placeholders as `run`, minus `{LAYOUT}`/`{DATA}`).            |
| `loc.path`      | no       | Path (relative to the repo root) passed to `cloc` for the Lines of Code row.                       |
| `loc.cloc_langs`| no       | Optional `cloc --include-lang` filter (e.g. `"C,C/C++ Header"`).                                   |
| `size.paths`    | no       | List of paths (relative to the repo root) whose on-disk byte sizes are summed for the Solution Size row. A list because a solution may be more than one file — a main script/binary plus any helper libraries/modules it depends on should all be listed here. |

An implementation whose `run`'s first token isn't an executable file (absolute path) or a command on `PATH` (bare name) is skipped with a warning instead of failing the whole run — useful while a language is only partially wired up.

## Normalization

The comparison normalizes only:

- Timestamps (dates → `DATE`, times → `TIME`) — needed because some scenarios use `$(date)` in titles/footers

ANSI color codes and separator geometry are **not** stripped. Both implementations must emit identical theme colors, `{COLOR}` placeholder expansions, color scoping, and border/separator line lengths.

## CLI options tested implicitly

All implementations are expected to support `--mono` (disable all ANSI colors). Every scenario runs each implementation twice: once with default colors and once with `--mono`, and both must match the reference for the scenario to pass.

## Performance Notes

- Per-implementation timeout comes from `implementations.json` (`timeout`, seconds); currently 120s for Bash and 10s for C.
- The performance table's per-implementation timeout, LOC path, size paths, and lint command all come from `implementations.json` — see that file to tune any of them.

## Requirements

- **C**: Compiled binary at `tables.c/tables` (requires `libjansson-dev`)
- **Bash**: `tables.sh/tables.sh` (requires `jq`)
- **Test runner**: `jq` (for parsing the manifest, dynamic layouts, and `implementations.json`)
- Optional, per implementation: its configured lint tool (e.g. `shellcheck`, `cppcheck`) and `cloc` for the Lines of Code row.
