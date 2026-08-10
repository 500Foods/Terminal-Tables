# Test Suite

This directory contains the comparison test suite for the Terminal Tables implementations.

## Overview

The test suite compares the output of the C implementation (`tables.c/tables`) against the Bash implementation (`tables.sh/tables.sh`) to ensure identical rendering. Both implementations are run with the same test scripts and their outputs are compared byte-for-byte (after color stripping and normalization).

## Directory Structure

- `tests/run_tests.sh` — Shell-based test runner that runs all suites and compares C vs Bash output
- `tests/comparison.bats` — Bats test files for CI integration
- `tests/data/` — Shared JSON data/Layout files (populated as needed)

## Running Tests

### Shell Runner

```bash
bash tests/run_tests.sh           # Run all suites
bash tests/run_tests.sh 01 02     # Run specific suites
```

### Bats Runner

```bash
bats tests/comparison.bats        # Run all comparison tests
bats tests/comparison.bats -fn "Suite 01"  # Run specific test
```

## Test Suites

| Suite | File | Description | Sub-tests |
|-------|------|-------------|-----------|
| 01 | tables_test_01_basic | Basic datatypes and justifications (text, int, num, float, kcpu, kmem) | 9 |
| 02 | tables_test_02_summary | Sum, min, max, avg, count, unique summaries | 9 |
| 03 | tables_test_03_wrapping | Text wrapping modes (wrap, clip, break) | 11 |
| 04 | tables_test_04_complex | Complex tables with mixed features | 5 |
| 05 | tables_test_05_titles | Title rendering and positioning | 5 |
| 06 | tables_test_06_title_positions | Title position clipping (left/center/right) | 12 |
| 07 | tables_test_07_footers | Footer rendering and positioning | 5 |
| 08 | tables_test_08_footer_positions | Footer position clipping (left/center/right) | 17 |
| 09 | tables_test_09_showcase | Showcase with multiple tables | 22 |

Total: 7 tests × ~93 sub-tests (counting all sub-tests across all suites)

## Normalization

The comparison normalizes the following differences:
- ANSI color codes are stripped
- Timestamps (dates and times) are replaced with `DATE` and `TIME` placeholders
- Test labels `TestC X-Y` (C version) → `Test X-Y` (Bash version)
- Separator lines (dashes after test labels) are normalized for length

## Adding New Implementations

To add a new language implementation to the comparison suite:
1. Add the implementation path as a new variable in `run_tests.sh`
2. Add a new symlink setup block in the `run_test_case()` function
3. Add a new comparison call alongside the C and Bash calls
4. Add the new test case to `comparison.bats`

## Requirements

- **C**: Compiled binary at `tables.c/tables` (requires `libjansson-dev`)
- **Bash**: `tables.sh/tables.sh` (requires `jq`)
- **Both**: `bats` (for bats-based tests)
