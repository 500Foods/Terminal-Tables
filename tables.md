# Tables

A flexible utility for rendering JSON data as ANSI tables in terminal output. Available in two implementations:

- **Bash** (`tables.sh/tables.sh`): The reference implementation, using `jq` for JSON parsing
- **C** (`tables.c/tables`): Performance-optimized binary, compiled from C source with `libjansson`

Both implementations produce identical output (after timestamp normalization) and support the same features.

## Overview

Tables.sh converts JSON data into beautifully formatted ANSI tables with the following features:

- Multiple visual themes with colored borders and distinct element colors
- Support for various data types (text, int, num, float, Kubernetes CPU/memory values)
- Customizable column configurations (headers, alignment, formatting, width, visibility)
- Data processing capabilities (sorting, validation, summaries calculation)
- Text wrapping and custom display options for null/zero values
- Title and footer support with flexible positioning
- Thousands separator formatting for `num` and `float` types (int types are raw)
- `--mono` flag to disable all ANSI colors for monochrome environments

## Usage

Both implementations share the same CLI interface:

```bash
./tables.sh <layout_json_file> <data_json_file> [OPTIONS]
# or
./tables <layout_json_file> <data_json_file> [OPTIONS]
```

### Parameters

- `layout_json_file`: JSON file defining table structure and formatting
- `data_json_file`: JSON file containing the data to display

### Options

- `--mono`: Disable all ANSI colors (theme colors and `{COLOR}` placeholders)
- `--version`: Display version information
- `--help`, `-h`: Show help message
- C only: `--debug`, `--debug_layout`

### Examples

```bash
# Basic table rendering
./tables layout.json data.json

# Monochrome output (no ANSI colors)
./tables layout.json data.json --mono

# Show version
./tables --version

# Show help
./tables --help
```

## Layout JSON Structure

The layout file defines how the table should be structured and formatted:

```json
{
  "theme": "Red",
  "title": "Table Title",
  "title_position": "center",
  "footer": "Table Footer", 
  "footer_position": "center",
  "sort": [
    {"key": "column_key", "direction": "asc", "priority": 1}
  ],
  "columns": [
    {
      "header": "COLUMN NAME",
      "key": "json_field_name",
      "justification": "left",
      "datatype": "text",
      "null_value": "blank",
      "zero_value": "blank",
      "summary": "none",
      "break": false,
      "string_limit": 0,
      "wrap_mode": "clip",
      "wrap_char": "",
      "padding": 1,
      "width": 0,
      "format": "",
      "visible": true
    }
  ]
}
```

### Theme Options

The `theme` field defines the visual appearance of the table:

- `"Red"`: Red borders with distinct colors for different table elements
- `"Blue"`: Blue borders with distinct colors for different table elements

Each theme includes specific colors for:

- **Border**: Table borders and separators
- **Header**: Table title text
- **Caption**: Column header text
- **Summary**: Summary/totals row text
- **Footer**: Footer text
- **Text**: Regular data content

#### Detailed Theme Information

Each theme defines ANSI color codes and Unicode box-drawing characters for all table elements:

##### Key Features

- **Multiple Themes**: Pre-defined Red and Blue themes
- **Color Management**: ANSI color codes for different elements
- **Unicode Characters**: Box-drawing characters for professional appearance
- **Theme Switching**: Dynamic theme changes during runtime

##### Available Themes

- **Red Theme**: Uses red borders (`\033[0;31m`), with cyan captions and footers (`\033[0;36m`), Bright White headers and summaries (`\033[1;37m`), and default text color.
- **Blue Theme**: Uses blue borders (`\033[0;34m`), with blue captions and footers (`\033[0;34m`), Bright White headers and summaries (`\033[1;37m`), and default text color.

##### Theme Structure

Each theme is defined as an associative array with color elements (border, caption, header, footer, summary, text) and border characters (corners, lines, junctions).

##### Functions

- **get_theme**: Updates the active theme based on the theme name ("Red" or "Blue"). Unknown themes fall back to Red.

##### Extending Themes

New themes can be added by defining new associative arrays with custom color schemes and updating the `get_theme` function to recognize the new theme.

### Title and Footer Support

Tables can include optional titles and footers with flexible positioning:

#### Title Configuration

- `title`: The title text to display above the table
- `title_position`: Position of the title relative to the table
  - `"left"`: Left-aligned title
  - `"right"`: Right-aligned title
  - `"center"`: Centered title
  - `"full"`: Title spans the full table width
  - `"none"`: No title (default)

#### Footer Configuration

- `footer`: The footer text to display below the table
- `footer_position`: Position of the footer relative to the table
  - `"left"`: Left-aligned footer
  - `"right"`: Right-aligned footer
  - `"center"`: Centered footer
  - `"full"`: Footer spans the full table width
  - `"none"`: No footer (default)

### Sort Configuration

The `sort` array allows sorting data by one or more columns:

- `key`: The column key to sort by
- `direction`: Either `"asc"` (ascending) or `"desc"` (descending)
- `priority`: Sort priority when multiple sort keys are defined (lower numbers have higher priority)

### Column Configuration Options

Each column in the `columns` array can have the following properties:

| Property | Description | Default | Options |
|----------|-------------|---------|---------|
| `header` | Column header text | (required) | Any string |
| `key` | JSON field name in the data | Derived from header | Any string |
| `justification` | Text alignment | `"left"` | `"left"`, `"right"`, `"center"` |
| `datatype` | Data type for validation and formatting | `"text"` | `"text"`, `"int"`, `"num"`, `"float"`, `"kcpu"`, `"kmem"` |
| `null_value` | How to display null values | `"blank"` | `"blank"`, `"0"`, `"missing"` |
| `zero_value` | How to display zero values | `"blank"` | `"blank"`, `"0"`, `"missing"` |
| `summary` | Type of summary to calculate | `"none"` | See "Summary Types" section |
| `break` | Insert separator when value changes | `false` | `true`, `false` |
| `string_limit` | Maximum string length | `0` (unlimited) | Any integer |
| `wrap_mode` | How to handle text exceeding limit | `"clip"` | `"clip"`, `"wrap"` |
| `wrap_char` | Character to use for wrapping | `""` | Any character |
| `padding` | Padding spaces on each side | `1` | Any integer |
| `width` | Fixed column width | `0` (auto) | Any integer |
| `format` | Custom format string | `""` | Format string |
| `visible` | Whether to display the column | `true` | `true`, `false` |

#### Visibility Option

The `visible` property allows you to hide specific columns from the table output without removing them from the data processing. Setting `"visible": false` in a column's configuration will exclude that column from the rendered table, ensuring that it does not affect the layout or border calculations. This is useful for including data in the JSON that you might need for sorting or other processing but do not wish to display. In particular, this is useful for adding a custom break value, where the break value itself doesn't need to be visible. For example, in a file listing, a folder column could be present, but not shown, such that extra separator lines could be drawn between folders.

### Annotated Rows

Data rows may include an optional `"annotate": true` flag. Annotated rows are rendered like any other row (and still participate in column width and break calculations) but are **excluded from all summary aggregations** (`sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks`).

Use annotated rows for informational lines such as notes, labels, or metrics that should appear in the table body without skewing totals. Combine with a hidden `break` column if you want a separator before/after the annotated row.

**Example data row:**

```json
{
  "annotate": true,
  "name": "Lines of Code",
  "bash": 1384,
  "c": 3467
}
```

## Supported Data Types

Tables.sh supports the following data types:

### text

Text data with optional wrapping and length limits.

- **Validation**: Any non-null text value
- **Formatting**: Raw text with enhanced clipping and wrapping options
- **Summary Types**: `count`, `unique`

### int

Integer numbers rendered as raw digits without thousands separators.

- **Validation**: Any valid number
- **Formatting**: Raw number (e.g., `1234` — no comma separators)
- **Summary Types**: `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks`

### float

Floating-point numbers with decimal precision and thousands separators.

- **Validation**: Any valid number
- **Formatting**: Numbers formatted with thousands separators and decimals (e.g., `1,234.50`)
- **Summary Types**: `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks`

### num

Numeric values with thousands separator formatting.

- **Validation**: Any valid number
- **Formatting**: Numbers formatted with thousands separators (e.g., `1,234`)
- **Summary Types**: `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks`

### kcpu

Kubernetes-style CPU values (e.g., `100m` for 100 millicores).

- **Validation**: Values with `m` suffix or numeric values
- **Formatting**: Thousands separators and `m` suffix (e.g., `1,250m`)
- **Summary Types**: `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks`

### kmem

Kubernetes-style memory values (e.g., `128M`, `1G`, `512Ki`).

- **Validation**: Values with `K`, `M`, `G`, `Ki`, `Mi`, `Gi` suffixes
- **Formatting**: Normalized to `K`/`M`/`G` (or `Ki`/`Mi`/`Gi`) format with thousands separators
- **Summary Types**: `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks`

## Summary Types

Depending on the data type, the following summary calculations are available:

- `sum`: Sum of all values (numeric types, kcpu, kmem)
- `min`: Minimum value (numeric types, kcpu, kmem)
- `max`: Maximum value (numeric types, kcpu, kmem)
- `avg`: Average value (numeric types, kcpu, kmem)
- `count`: Count of non-null values (all types)
- `unique`: Count of unique values (all types)
- `blanks`: Count of blank/null values (all types)
- `nonblanks`: Count of non-blank values (all types)
- `none`: No summary (default)

## Example Tables

### Basic Example

This example renders a table of Kubernetes pod information:

**Layout JSON (layout.json):**

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "POD",
      "key": "pod",
      "justification": "left",
      "datatype": "text"
    },
    {
      "header": "NAMESPACE",
      "key": "namespace",
      "justification": "center",
      "datatype": "text"
    },
    {
      "header": "CPU USE",
      "key": "cpu_use",
      "justification": "right",
      "datatype": "kcpu",
      "summary": "sum"
    }
  ]
}
```

**Data JSON (data.json):**

```json
[
  {
    "pod": "pod-a",
    "namespace": "default",
    "cpu_use": "100m"
  },
  {
    "pod": "pod-b",
    "namespace": "kube-system",
    "cpu_use": "50m"
  }
]
```

**Command:**

```bash
./tables layout.json data.json
```

**Output:**

```table
╭───────────┬────────────┬─────────╮
│POD        │ NAMESPACE  │  CPU USE│
├───────────┼────────────┼─────────┤
│pod-a      │  default   │     100m│
│pod-b      │kube-system │      50m│
├───────────┼────────────┼─────────┤
│           │            │     150m│
╰───────────┴────────────┴─────────╯
```

### Advanced Example with Title and Footer

This example demonstrates more features, including titles, footers, sorting, and summaries:

**Layout JSON:**

```json
{
  "theme": "Blue",
  "title": "Pod Resource Usage Report",
  "title_position": "center",
  "footer": "Generated by tables",
  "footer_position": "right",
  "sort": [
    {"key": "namespace", "direction": "asc", "priority": 1},
    {"key": "pod", "direction": "asc", "priority": 2}
  ],
  "columns": [
    {
      "header": "POD",
      "key": "pod",
      "justification": "left",
      "datatype": "text",
      "summary": "count"
    },
    {
      "header": "NAMESPACE",
      "key": "namespace",
      "justification": "center",
      "datatype": "text",
      "break": true
    },
    {
      "header": "CPU USE",
      "key": "cpu_use",
      "justification": "right",
      "datatype": "kcpu",
      "null_value": "missing",
      "summary": "sum"
    },
    {
      "header": "MEM USE",
      "key": "mem_use",
      "justification": "right",
      "datatype": "kmem",
      "zero_value": "0",
      "summary": "sum"
    },
    {
      "header": "REQUESTS",
      "key": "requests",
      "justification": "right",
      "datatype": "num",
      "summary": "sum"
    }
  ]
}
```

**Data JSON:**

```json
[
  {
    "pod": "pod-a",
    "namespace": "ns1",
    "cpu_use": "100m",
    "mem_use": "128M",
    "requests": 1500
  },
  {
    "pod": "pod-b",
    "namespace": "ns1",
    "cpu_use": "50m",
    "mem_use": "64M",
    "requests": 750
  },
  {
    "pod": "pod-c",
    "namespace": "ns2",
    "cpu_use": null,
    "mem_use": "256M",
    "requests": 2000
  },
  {
    "pod": "pod-d",
    "namespace": "ns2",
    "cpu_use": "200m",
    "mem_use": null,
    "requests": 1200
  }
]
```

### Title and Footer Positioning Examples

**Short Title (shorter than table width):**

```json
{
  "title": "Pod Stats",
  "title_position": "left"
}
```

**Wide Title (wider than table width):**

```json
{
  "title": "Kubernetes Cluster Pod Resource Utilization Report - Production Environment",
  "title_position": "center"
}
```

**Full Width Title:**

```json
{
  "title": "System Report",
  "title_position": "full"
}
```

### Example with Hidden Column

This example shows how to use the `visible: false` property to hide a column from display while still processing its data:

**Layout JSON:**

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "justification": "right",
      "datatype": "int"
    },
    {
      "header": "Hidden Data",
      "key": "hidden_data",
      "justification": "left",
      "datatype": "text",
      "visible": false
    },
    {
      "header": "Server Name",
      "key": "name",
      "justification": "left",
      "datatype": "text"
    },
    {
      "header": "Status",
      "key": "status",
      "justification": "center",
      "datatype": "text"
    }
  ]
}
```

**Data JSON:**

```json
[
  {
    "id": 1,
    "hidden_data": "secret-info-1",
    "name": "web-server-01",
    "status": "Running"
  },
  {
    "id": 2,
    "hidden_data": "secret-info-2",
    "name": "db-server-01",
    "status": "Running"
  }
]
```

**Output:**

```table
╭────┬───────────────┬──────────╮
│ ID │ Server Name   │  Status  │
├────┼───────────────┼──────────┤
│  1 │ web-server-01 │ Running  │
│  2 │ db-server-01  │ Running  │
╰────┴───────────────┴──────────╯
```

In this example, the "Hidden Data" column is not displayed in the table output, even though the data is present in the JSON file.

## Using in Scripts

The Bash implementation can be sourced directly in your own scripts to render tables programmatically:

```bash
#!/usr/bin/env bash

# Source the tables library (Bash implementation)
source ./tables.sh

# Create layout and data files
cat > layout.json << 'EOF'
{
  "theme": "Red",
  "title": "Sample Report",
  "title_position": "center",
  "columns": [
    {
      "header": "NAME",
      "key": "name",
      "datatype": "text"
    },
    {
      "header": "VALUE",
      "key": "value",
      "datatype": "num",
      "summary": "sum"
    }
  ]
}
EOF

cat > data.json << 'EOF'
[
  {"name": "Item A", "value": 1000},
  {"name": "Item B", "value": 2500}
]
EOF

# Draw the table
draw_table layout.json data.json
```

## Tips and Best Practices

1. **Column Width Management**:
   - The script automatically determines column widths based on content
   - Use `width` property to set fixed column widths
   - Use `string_limit` and `wrap_mode` for wide columns

2. **Data Sorting**:
   - Complex sorting can be achieved with multiple sort keys and priorities
   - Use `break: true` to visually group data by important fields

3. **Null and Zero Handling**:
   - Choose appropriate `null_value` and `zero_value` settings for each column
   - Options include showing blank space, "0", or "Missing"

4. **Title and Footer Design**:
   - Use `title_position` and `footer_position` to control alignment
   - Consider table width when designing titles and footers
   - Use `"full"` position for titles/footers that should span the entire table width

5. **Column Visibility**:
   - Use `visible: false` to hide columns that contain data needed for processing (like sorting) but should not be displayed
   - Hidden columns do not affect the table layout or border rendering

6. **Performance**:
   - The Bash implementation is slower than the C implementation due to `jq` subprocess calls
   - For high-throughput use, prefer the C binary

7. **Testing**:
   - The project includes a comprehensive test suite (`tests/run_tests.sh`)
   - Tests compare C and Bash output across 98 test cases

8. **Color Compatibility**:
   - The colored output uses ANSI escape sequences which work in most terminals
   - Use `--mono` to disable all colors for environments without color support

## Dependencies

- `jq` (JSON processor)
- `bash` 4.0+
- `awk`
- `sed`

## Unicode and International Character Support

Tables handles Unicode width detection for international characters and emojis, ensuring accurate column alignment even with double-width characters.

### Supported Unicode Ranges

- **Emojis**: Full support for all emoji ranges (U+1F300-U+1F9FF, U+2600-U+27BF)
- **CJK Characters**: Chinese, Japanese, Korean character sets
- **Mathematical Symbols**: Various mathematical and technical symbols
- **Box Drawing**: Enhanced box-drawing characters for table borders
- **Dingbats**: Checkmarks, arrows, and other symbol characters

## Dynamic Content and Command Execution

Tables.sh supports **dynamic content generation** with command substitution in titles and footers:

### Dynamic Features

- **Command Substitution**: Execute shell commands within titles and footers using `$(command)` syntax
- **Date/Time Integration**: Real-time date and time insertion
- **System Information**: Include system stats, uptime, or any shell command output
- **Color Placeholder Support**: Use `{RED}`, `{BLUE}`, `{GREEN}`, etc. for colored text

### Dynamic Content Examples

```json
{
  "title": "Server Report - Generated $(date '+%Y-%m-%d %H:%M:%S')",
  "footer": "System Load: $(uptime | cut -d':' -f4-) - Total Servers: $(jq 'length' data.json)"
}
```

```json
{
  "title": "{BOLD}Production Status{NC} - {GREEN}$(date +%A){NC}",
  "footer": "{CYAN}Generated by $(whoami) on $(hostname){NC}"
}
```

## Text Wrapping

- **Word Wrapping**: Intelligent word boundary detection
- **Character Wrapping**: Custom character-based wrapping (e.g., comma-separated lists)
- **Width Management**: Automatic and manual column width control
- **Overflow Handling**: Multiple strategies for handling content overflow

## Data Processing Pipeline

- **Validation Layer**: Type-specific data validation for all supported data types
- **Formatting Layer**: Thousands separators, custom formats, and datatype-specific display
- **Aggregation Layer**: Multiple summary types with accurate calculations
- **Sorting Layer**: Multi-key sorting with priority support

## Dependencies

- Bash: `jq`, `bash` 4.0+, `awk`, `sed`
- C: `libjansson-dev`, `gcc`, `make`
- Testing: `shellcheck`, `cppcheck`, `jq`

## Version Information

- **Current Version**: 3.0.0
- **Version History**:
  - **3.0.0** - Added C implementation, `--mono` flag, `blanks`/`nonblanks` summaries, annotated rows, `visible` column property, full kcpu/kmem summary support
    - C implementation for performance (Bash remains the reference)
    - `--mono` flag to disable all ANSI colors
    - New summary types: `blanks`, `nonblanks`
    - `int` datatype renders without thousands separators; `num`/`float` render with separators
    - Annotated rows (`"annotate": true`) for display-only data excluded from summaries
    - 98 test cases

Use `./tables --version` (C) or `./tables.sh --version` (Bash) to see the current version, or `./tables --help` / `./tables.sh --help` for usage information.
