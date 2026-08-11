# Development Guide

This document covers the architecture, internals, and extension points for both the Bash and C implementations of Terminal Tables.

## Architecture

The project has two implementations that share the same CLI interface and output format:

- **Bash** (`tables.sh/tables.sh`): The reference implementation (~2000 lines, single monolithic file). Uses `jq` for JSON parsing. All functions are in one file — there is no `lib/` directory.
- **C** (`tables.c/`): Performance-optimized binary. Modular structure with separate `.c`/`.h` files per component.

Bash is the correctness oracle (marked `"reference": true` in `tests/implementations.json`); C is the performance baseline (marked `"baseline": true`). All test scenarios compare C output against Bash output.

### C Source Structure

```
tables.c/
├── tables.c              # Main entry point, argument parsing
├── tables_config.c/.h    # Layout JSON parsing, column config, sort config
├── tables_data.c/.h      # Data loading from JSON, row processing
├── tables_datatypes.c/.h # Datatype handlers (validate/format/summarize)
├── tables_themes.c/.h    # Theme definitions (Red, Blue)
├── tables_render.c/.h    # Main render dispatch
├── tables_render_layout.c  # Column width calculation, summary values
├── tables_render_output.c  # Table output assembly
├── tables_render_rows.c    # Data row rendering
├── tables_render_headers.c # Header row rendering
├── tables_render_summaries.c # Summary row rendering
├── tables_render_title.c   # Title rendering with positioning
├── tables_render_footer.c  # Footer rendering with positioning
└── tables_render_utils.c   # Unicode width, text wrapping utilities
```

### Bash Source Structure

The Bash implementation is a single file — `tables.sh/tables.sh`. All functions, global variables, and theme definitions live in this one file. There are no subdirectories.

## Execution Flow

Both implementations follow the same pipeline:

1. Parse arguments and validate input files
2. Parse layout JSON: theme, title, footer, columns, sort config
3. Initialize themes and column configuration arrays
4. Load data from JSON file
5. Sort data if specified
6. Process data rows: format values, update column widths, calculate summaries
7. Render the table (title, borders, headers, data rows, summaries, footer)

## Configuration System

### Layout JSON Fields

| Field | Description | Default |
|-------|-------------|---------|
| `theme` | Visual theme | `"Red"` |
| `title` | Title text above table | (none) |
| `title_position` | Title alignment | `"none"` |
| `footer` | Footer text below table | (none) |
| `footer_position` | Footer alignment | `"none"` |
| `sort` | Array of sort keys | (none) |
| `columns` | Array of column definitions | (required) |

### Column Configuration

| Property | Description | Default | Options |
|----------|-------------|---------|---------|
| `header` | Column header text | (required) | Any string |
| `key` | JSON field name for data extraction | Derived from header | Any string |
| `justification` | Text alignment | `"left"` | `"left"`, `"right"`, `"center"` |
| `datatype` | Data type for validation/formatting | `"text"` | `"text"`, `"int"`, `"num"`, `"float"`, `"kcpu"`, `"kmem"` |
| `null_value` | Display for null values | `"blank"` | `"blank"`, `"0"`, `"missing"` |
| `zero_value` | Display for zero values | `"blank"` | `"blank"`, `"0"`, `"missing"` |
| `summary` | Summary calculation type | `"none"` | See Summary Types |
| `break` | Insert separator on value change | `false` | `true`, `false` |
| `string_limit` | Max string length before wrapping | `0` (unlimited) | Any integer |
| `wrap_mode` | How to handle text exceeding limit | `"clip"` | `"clip"`, `"wrap"` |
| `wrap_char` | Character for delimiter-based wrapping | `""` | Any character |
| `padding` | Spaces on each side of content | `1` | Any integer |
| `width` | Fixed column width | `0` (auto) | Any integer |
| `format` | Custom format string (e.g. `"%.2f"`) | `""` | Format string |
| `visible` | Whether to display the column | `true` | `true`, `false` |

### Sort Configuration

Each entry in the `sort` array:

| Property | Description | Options |
|----------|-------------|---------|
| `key` | Column key to sort by | Any column key |
| `direction` | Sort direction | `"asc"`, `"desc"` |
| `priority` | Sort priority (lower = higher) | Any integer |

## Datatype System

### Datatype Registry

Both implementations use a registry pattern mapping datatypes to validate/format/summarize functions:

**Bash** uses `DATATYPE_HANDLERS` associative array:
```bash
declare -A DATATYPE_HANDLERS=(
    [text_validate]="validate_text"
    [text_format]="format_text"
    [text_summary_types]="count unique"
    [int_validate]="validate_number"
    [int_format]="format_number"
    [int_summary_types]="sum min max avg count unique blanks nonblanks"
    [num_validate]="validate_number"
    [num_format]="format_num"
    [num_summary_types]="sum min max avg count unique blanks nonblanks"
    [float_validate]="validate_number"
    [float_format]="format_number"
    [float_summary_types]="sum min max avg count unique blanks nonblanks"
    [kcpu_validate]="validate_kcpu"
    [kcpu_format]="format_kcpu"
    [kcpu_summary_types]="sum min max avg count unique blanks nonblanks"
    [kmem_validate]="validate_kmem"
    [kmem_format]="format_kmem"
    [kmem_summary_types]="sum min max avg count unique blanks nonblanks"
)
```

**C** uses a `handlers[]` array in `tables/datatypes.c`:
```c
{"int", validate_number, format_number, "sum min max avg count unique blanks nonblanks"},
{"num", validate_number, format_num, "sum min max avg count unique blanks nonblanks"},
{"float", validate_number, format_number, "sum min max avg count unique blanks nonblanks"},
{"kcpu", validate_kcpu, format_kcpu, "sum min max avg count unique blanks nonblanks"},
{"kmem", validate_kmem, format_kmem, "sum min max avg count unique blanks nonblanks"},
```

### Datatype Details

| Datatype | Validation | Formatting | Summary Types |
|----------|-----------|------------|---------------|
| `text` | Any non-null text | Raw text (clip/wrap) | `count`, `unique` |
| `int` | Any valid number | Raw digits, **no separators** | `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks` |
| `num` | Any valid number | Thousands separators (e.g., `1,234`) | `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks` |
| `float` | Any valid number | Decimals + separators (e.g., `1,234.50`) | `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks` |
| `kcpu` | `100m` format or number | `m` suffix + separators (e.g., `1,250m`) | `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks` |
| `kmem` | `128M`, `1G`, `512Ki` | Normalized + separators (e.g., `1,024M`) | `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks` |

### Formatting Functions (Bash)

- `format_text()`: Raw text with optional clipping/wrapping
- `format_number()`: Int values — raw digits, no comma separators
- `format_num()`: Num values — with thousands separators
- `format_kcpu()`: Kubernetes CPU values with `m` suffix
- `format_kmem()`: Kubernetes memory values with unit normalization

### Formatting Functions (C)

- `format_text()` in `tables_datatypes.c`
- `format_number()` — returns raw value (used for `int`)
- `format_num()` — uses `format_with_commas()` (used for `num`)
- `format_kcpu()` / `format_kmem()` — Kubernetes value formatting
- `format_display_value_with_precision()` — dispatches to handler, with special float precision path

## Theme System

### Available Themes

- **Red Theme**: Red borders (`\033[0;31m`), cyan captions (`\033[0;36m`), bright white headers/summaries (`\033[1;37m`)
- **Blue Theme**: Blue borders (`\033[0;34m`), blue captions (`\033[0;34m`), bright white headers/summaries (`\033[1;37m`)

### Theme Structure

Each theme defines these elements:

| Element | Description |
|---------|-------------|
| `border_color` | Table borders and separators |
| `caption_color` | Column header text |
| `header_color` | Table title text |
| `footer_color` | Table footer text |
| `summary_color` | Summary/totals row text |
| `text_color` | Regular data content |
| `tl_corner` / `tr_corner` / `bl_corner` / `br_corner` | Corner characters |
| `h_line` / `v_line` | Horizontal/vertical line characters |
| `t_junct` / `b_junct` / `l_junct` / `r_junct` / `cross` | Junction characters |

### Adding a New Theme

1. Define a new associative array (Bash) or `Theme` struct (C) with custom colors and border characters
2. Add a case in `get_theme()` (Bash) or `get_theme()` (C `tables_themes.c`) to handle the new theme name
3. Unknown themes fall back to Red with a warning to stderr

## Annotated Rows

Data rows may include `"annotate": true`. Annotated rows are rendered normally (participating in column width and break calculations) but are **excluded from all summary aggregations**.

- **Bash**: Stored per-row in `ROW_ANNOTATE[]`
- **C**: Stored per-row in `DataRow.annotate` field (`tables_data.h`)

Use annotated rows for informational lines (e.g., "Lines of Code" totals) that should not skew numeric summaries.

## Column Visibility

The `visible` property (`true`/`false`) controls whether a column appears in the rendered table. Hidden columns still participate in data processing (sorting, break calculations) and width calculations — they are simply not included in the output.

- **Bash**: Stored in `VISIBLES[]` array
- **C**: Stored in `Column.visible` field (`tables_config.h`)

## Summary Types

| Type | Description | Applicable Datatypes |
|------|-------------|---------------------|
| `sum` | Arithmetic sum | int, num, float, kcpu, kmem |
| `min` | Minimum value | int, num, float, kcpu, kmem |
| `max` | Maximum value | int, num, float, kcpu, kmem |
| `avg` | Average value | int, num, float, kcpu, kmem |
| `count` | Count of non-null values | all types |
| `unique` | Count of unique values | all types |
| `blanks` | Count of blank/null values | all types |
| `nonblanks` | Count of non-blank values | all types |
| `none` | No summary (default) | all types |

## Text Wrapping

- `clip`: Truncates text to `string_limit` (default)
- `wrap`: Wraps text at word boundaries, using `wrap_char` as delimiter if specified

## CLI Options

Both implementations accept:

| Flag | Description |
|------|-------------|
| `--mono` | Disable all ANSI colors (theme colors and `{COLOR}` placeholders expand to empty) |
| `--help`, `-h` | Show help message |
| `--version` | Display version information |
| `--debug` (C only) | Enable detailed stderr logging |
| `--debug_layout` (C only) | Print column width and layout debug info |

## Test Suite

The test suite (`tests/run_tests.sh`) compares C output against Bash output across JSON-defined scenarios.

### Test Scenarios

Scenarios are defined as `_data.json` / `_layout.json` pairs in `tests/scenarios/suite_XX/` directories, registered in `tests/scenarios/manifest.json`.

To add a new test case:
1. Create `_data.json` and `_layout.json` files in the appropriate `suite_XX/` directory
2. Add an entry to `tests/scenarios/manifest.json`

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

The comparison normalizes only timestamps (`YYYY-MM-DD` → `DATE`, `HH:MM:SS` → `TIME`). ANSI color codes and separator geometry are **not** stripped — both implementations must emit identical theme colors and border characters.

### Implementations

Declared in `tests/implementations.json` (not hardcoded in the runner). Each entry specifies the run command, lint command, timeout, and line count. Bash is `"reference": true`; C is `"baseline": true`.

## Building

```bash
# C binary (requires libjansson-dev)
make -C tables.c              # optimized + UPX if available
make -C tables.c uncompressed # strip only

# Shellcheck
shellcheck tables.sh/tables.sh

# C static analysis
cppcheck tables.c/
```

## Unicode Width Calculation

The C implementation (`tables_render_utils.c`) calculates display width with a multi-tier approach:
1. Strip ANSI escape codes
2. Pure ASCII: simple `strlen`
3. Extended/Unicode: manual UTF-8 byte parsing with width lookup for double-width characters

The Bash implementation (`tables.sh/tables.sh`) uses a similar approach via `get_display_length()`:
1. Strip ANSI codes with `sed`
2. Pure ASCII: `${#string}`
3. Unicode: byte-level UTF-8 decoding with code point range checks

Both produce identical width calculations for all supported character ranges.

## Version Information

- **Current Version**: 3.0.0
- **Version History**:
  - **3.0.0** — Added C implementation (performance baseline), `--mono` flag, `blanks`/`nonblanks` summary types, `annotate` rows, `visible` column property, full kcpu/kmem summary support, separated int (raw) vs num (comma) formatting
  - **1.0.2** — Added help functionality and version history
  - **1.0.1** — Fixed shellcheck issues (SC2004, SC2155)
  - **1.0.0** — Initial release
