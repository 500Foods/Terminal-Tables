# Test Suite

This directory contains the comparison test suite for the Terminal Tables implementations.

## Overview

The test suite compares the output of the C implementation (`tables.c/tables`) against the Bash implementation (`tables.sh/tables.sh`) to ensure identical rendering. Both implementations are run with the same JSON data and layout files, and their outputs are compared including ANSI color sequences (after normalizing dynamic content only).

**Bash is the reference implementation.** The Bash implementation was the original variant created; all other implementations (C, and any future ports) are compared *against* Bash output as the oracle.

## Directory Structure

```
tests/
├── run_tests.sh              # Shell-based test runner
├── comparison.bats           # Bats test file for CI integration
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

Suite 00 (Linting) is handled by `run_tests.sh` directly: shellcheck on `tables.sh/tables.sh` and cppcheck on `tables.c/`. Pass = zero issues; fail = non-zero exit.

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

The performance table includes suite timings, a Total row, and an annotated
**Lines of Code** row (from `cloc`; excluded from any summary math via
`"annotate": true`). Use `--results` (or `-r`) to render that table again
without re-running tests.

### Bats Runner

```bash
bats tests/comparison.bats        # Run all comparison tests (9 suite-level tests)
```

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

**Bash is the reference oracle.** When adding a new language implementation, it must match the Bash output (after normalization) for every scenario. To add a new language:
1. Add the implementation binary/script path to `run_tests.sh`
2. Add a symlink setup block in the `run_scenario()` function
3. Add a new `run` block alongside the C and Bash calls
4. Add the test case to `comparison.bats`

## Normalization

The comparison normalizes only:
- Timestamps (dates → `DATE`, times → `TIME`) — needed because some scenarios use `$(date)` in titles/footers

ANSI color codes and separator geometry are **not** stripped. Both implementations must emit identical theme colors, `{COLOR}` placeholder expansions, color scoping, and border/separator line lengths.

## CLI options tested implicitly

Both implementations support `--mono` (disable all ANSI colors). Comparison runs use the default colored path so ANSI parity is enforced.

## Performance Notes

- The Bash implementation is slower than C (~0.5-2s per table) due to `jq` subprocess calls
- Test suite 08 (17 cases) and 09 (22 cases) can take 30-60 seconds in Bash
- Shell runner uses 120s timeout for Bash, 30s for C

## Requirements

- **C**: Compiled binary at `tables.c/tables` (requires `libjansson-dev`)
- **Bash**: `tables.sh/tables.sh` (requires `jq`)
- **Test runner**: `bats` (for bats-based tests), `jq` (for parsing manifest and dynamic layouts)
