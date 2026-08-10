# Agent Guidelines

## Project Structure

- `tables.c/` — C implementation of the terminal tables library
- `tables.sh/` — Bash implementation of the terminal tables library
- `tests/` — Comparison test suite (runs both implementations and compares output)

## Building

The C binary is at `tables.c/tables` and requires `libjansson-dev`:
```bash
gcc tables.c/tables.c/tables_main.c tables.c/tables.c/tables_config.c \
    tables.c/tables.c/tables_data.c tables.c/tables.c/tables_datatypes.c \
    tables.c/tables.c/tables_render.c tables.c/tables.c/tables_render_title.c \
    tables.c/tables.c/tables_render_footer.c tables.c/tables.c/tables_render_rows.c \
    tables.c/tables.c/tables_render_summaries.c tables.c/tables.c/tables_render_output.c \
    tables.c/tables.c/tables_render_utils.c -ljansson -o tables.c/tables -lm
```

## Running Tests

```bash
# Shell-based test runner
bash tests/run_tests.sh           # Run all suites
bash tests/run_tests.sh 01 05 09  # Run specific suites

# Bats-based tests
bats tests/comparison.bats
```

### Test Scripts

Test scripts live in:
- `tables.c/tst/` — C test scripts (`tables_test_XX_name.sh`, references `../tables.c/tables`)
- `tables.sh/tst/` — Bash test scripts (`tables_test_XX_name.sh`, references `../tables.sh`)

Both suites have identical content except for the `tables_script` path variable.

### Test Suites

| Suite | Description | Sub-tests |
|-------|-------------|-----------|
| 01 | Basic datatypes and justifications | 9 |
| 02 | Sum, min, max, avg, count, summaries | 9 |
| 03 | Text wrapping modes | 11 |
| 04 | Complex tables with mixed features | 5 |
| 05 | Title rendering and positioning | 5 |
| 06 | Title position clipping | 12 |
| 07 | Footer rendering and positioning | 5 |
| 08 | Footer position clipping | 17 |
| 09 | Showcase with multiple tables | 22 |

### Known Performance Notes

- The Bash implementation is significantly slower than C (~0.5-2s per table) due to `jq` subprocess calls
- Test suite 08 (17 sub-tests) and 09 (22 sub-tests) can take 30-60 seconds in Bash
- The shell test runner uses 120s timeout for Bash, 30s for C
