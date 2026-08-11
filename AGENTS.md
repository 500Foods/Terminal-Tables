# Agent Guidelines

## Project Structure

- `tables.c/` — C implementation of the terminal tables library
- `tables.sh/` — Bash implementation of the terminal tables library
- `tests/` — Comparison test suite. Implementations under test are declared in `tests/implementations.json`, not hard-coded in the runner — see `tests/README.md` for the schema before adding a new language (Python, Lua, Rust, Go, …).

## Building

The C binary is at `tables.c/tables` and requires `libjansson-dev`:
```bash
make -C tables.c              # optimized + UPX if available
make -C tables.c uncompressed # strip only
```

## Running Tests

```bash
# Shell-based test runner (runs every implementation in tests/implementations.json)
bash tests/run_tests.sh           # Run all suites
bash tests/run_tests.sh 01 05 09  # Run specific suites
bash tests/run_tests.sh --results # Re-show the performance table from the last run
```

### Test Scripts

Test scenarios are defined as JSON data/layout file pairs in `tests/scenarios/`.
The `tests/scenarios/manifest.json` file lists all test cases.

```
tests/
├── run_tests.sh           # Shell runner: iterates scenarios, compares output
├── implementations.json   # Registry of implementations under test (id, run cmd, lint, loc)
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
| 01 | Basic datatypes and justifications | 10 |
| 02 | Sum, min, max, avg, count, summaries | 10 |
| 03 | Text wrapping modes | 11 |
| 04 | Complex tables with mixed features | 5 |
| 05 | Title rendering and positioning | 5 |
| 06 | Title position clipping | 12 |
| 07 | Footer rendering and positioning | 5 |
| 08 | Footer position clipping | 17 |
| 09 | Showcase with multiple tables | 22 |

Total: 98 test cases (97 comparison + 1 lint)

### Normalization

The comparison normalizes only:
- Timestamps (`YYYY-MM-DD` → `DATE`, `HH:MM:SS` → `TIME`) — needed because some scenarios use `$(date)` in titles/footers

ANSI color codes and separator geometry are **not** stripped. Both implementations must emit identical theme colors, placeholder expansions (`{RED}`, `{NC}`, …), color scoping (pad outside color; reset after content), and border/separator line lengths.

**Bash is the reference implementation** for correctness. The Bash implementation (`tables.sh/tables.sh`) was the original variant created. All language implementations (C, future ports) are compared *against* the Bash output as the oracle — marked via `"reference": true` in `tests/implementations.json`. When adding a new language implementation, it should match the Bash output after normalization; the runner diffs every configured implementation against the reference automatically.

**C is the performance baseline.** C must remain the fastest implementation — marked via `"baseline": true` in `tests/implementations.json`. The performance table reports every other implementation's time as a multiple of C's (`"<Name> / C"` columns) and uses C to render the table itself. If any implementation ever benchmarks faster than C on the full suite, that's a regression in C (or a candidate for a new baseline) worth investigating, not something to shrug off.

### CLI options

All implementations are expected to accept:
- `--mono` — disable all ANSI colors (theme colors and `{COLOR}` placeholders expand to empty)
- `--help` / `-h`, `--version`
- C only: `--debug`, `--debug_layout`

**Note on datatypes:** `int` values render without thousands separators (e.g., `1234`); `num` values render with separators (e.g., `1,234`); `float` values render with decimal precision and separators. The summary row respects the same distinction.

### Known Issues

- The Bash implementation is slower than C (~0.5-2s per table) due to `jq` subprocess calls
- Test suite 08 (17 sub-tests) and 09 (22 sub-tests) take 30-60 seconds in Bash
- Per-implementation timeouts are configured in `tests/implementations.json` (currently 120s for Bash, 10s for C)
