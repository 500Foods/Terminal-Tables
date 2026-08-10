# Agent Guidelines

## Project Structure

- `tables.c/` — C implementation of the terminal tables library
- `tables.sh/` — Bash implementation of the terminal tables library
- `tests/` — Comparison test suite (runs both implementations and compares output)

## Building

The C binary is at `tables.c/tables` and requires `libjansson-dev`:
```bash
make -C tables.c              # optimized + UPX if available
make -C tables.c uncompressed # strip only
```

## Running Tests

```bash
# Shell-based test runner (compares C vs Bash output)
bash tests/run_tests.sh           # Run all suites
bash tests/run_tests.sh 01 05 09  # Run specific suites

# Bats-based tests
bats tests/comparison.bats
```

### Test Scripts

Test scenarios are defined as JSON data/layout file pairs in `tests/scenarios/`.
The `tests/scenarios/manifest.json` file lists all test cases.

```
tests/
├── run_tests.sh           # Shell runner: iterates scenarios, compares output
├── comparison.bats        # Bats test file for CI integration
├── README.md              # This documentation
└── scenarios/
    ├── manifest.json      # Master manifest of all test cases
    ├── suite_01/
    │   ├── test_1_A_data.json
    │   ├── test_1_A_layout.json
    │   ├── test_1_B_data.json
    │   └── test_1_B_layout.json
    └── suite_09/
        ├── test_9_A_data.json
        └── test_9_A_layout.sh   # Dynamic layout (uses $(date), $(jq))
```

Each scenario contains:
- `_data.json` — JSON array of data rows
- `_layout.json` (`.sh` if dynamic) — JSON layout definition

To add a new test case, create a new `_data.json` and `_layout.json` pair in the appropriate `suite_XX/` directory and add an entry to the manifest.

### Test Suites

| Suite | Description | Sub-tests |
|-------|-------------|-----------|
| 00 | Linting (shellcheck + cppcheck) | 1 |
| 01 | Basic datatypes and justifications | 9 |
| 02 | Sum, min, max, avg, count, summaries | 10 |
| 03 | Text wrapping modes | 11 |
| 04 | Complex tables with mixed features | 5 |
| 05 | Title rendering and positioning | 5 |
| 06 | Title position clipping | 12 |
| 07 | Footer rendering and positioning | 5 |
| 08 | Footer position clipping | 17 |
| 09 | Showcase with multiple tables | 22 |

Total: 97 test cases (96 comparison + 1 lint)

### Normalization

The comparison normalizes only:
- Timestamps (`YYYY-MM-DD` → `DATE`, `HH:MM:SS` → `TIME`) — needed because some scenarios use `$(date)` in titles/footers

ANSI color codes and separator geometry are **not** stripped. Both implementations must emit identical theme colors, placeholder expansions (`{RED}`, `{NC}`, …), color scoping (pad outside color; reset after content), and border/separator line lengths.

**Bash is the reference implementation.** The Bash implementation (`tables.sh/tables.sh`) was the original variant created. All language implementations (C, future ports) are compared *against* the Bash output as the oracle. When adding a new language implementation, it should match the Bash output after normalization.

### CLI options

Both implementations accept:
- `--mono` — disable all ANSI colors (theme colors and `{COLOR}` placeholders expand to empty)
- `--help` / `-h`, `--version`
- C only: `--debug`, `--debug_layout`

### Known Issues

- The Bash implementation is slower than C (~0.5-2s per table) due to `jq` subprocess calls
- Test suite 08 (17 sub-tests) and 09 (22 sub-tests) take 30-60 seconds in Bash
- The shell test runner uses 120s timeout for Bash, 30s for C

## Bash Bugs Fixed

1. **Trailing newline in TSV parsing** — `jq` output had trailing `\n` in values
2. **Integer avg truncation** — replaced `/` with rounding
3. **Missing `local datatype`** in `render_data_rows`
4. **Float zero-value detection** — `"0.0"` not matched by `"0"` check
5. **Empty field collapsing** in TSV parsing — used `mapfile -d` instead of `read -ra`
6. **Title/footer pre-clipping** — matches C's two-step clip behavior
