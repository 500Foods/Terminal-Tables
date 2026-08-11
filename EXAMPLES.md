# Terminal Tables — Examples

A guided tour of what Terminal Tables can render, built entirely from the project's own
test suite. Every table on these pages is produced by running one of the scenarios in
[`tests/scenarios/`](tests/scenarios) through the shipped binaries — nothing here is
hand-drawn, so what you read is what you get.

If you are looking for the reference documentation instead of worked examples, see
[`tables.md`](tables.md) for the full layout schema and [`tests/README.md`](tests/README.md)
for how the comparison suite is executed.

---

## Contents

1. [Before you start](#before-you-start)
2. [The two-file model](#the-two-file-model)
3. [Your first table](#your-first-table)
4. [Anatomy of a rendered table](#anatomy-of-a-rendered-table)
5. [Colour and `--mono`](#colour-and---mono)
6. [Running any example yourself](#running-any-example-yourself)
7. [How to read these pages](#how-to-read-these-pages)
8. [The example catalogue](#the-example-catalogue)
9. [Feature index](#feature-index)
10. [Further reading](#further-reading)

---

## Before you start

Terminal Tables ships as two interchangeable implementations that are held to
byte-for-byte identical output by the test suite:

| Implementation | Entry point | Requires | Notes |
|----------------|-------------|----------|-------|
| C | `tables.c/tables` | `libjansson-dev` to build | Fast; adds `--debug` and `--debug_layout` |
| Bash | `tables.sh/tables.sh` | `jq`, `bash` 4+, `awk`, `sed` | Reference implementation; no build step |

Build the C binary once:

```bash
make -C tables.c
```

The Bash version needs no build:

```bash
bash tables.sh/tables.sh --version
```

Because Bash is the reference implementation, any example in this catalogue can be run
through either program and will produce the same table. The outputs shown on these pages
were captured from the C binary; substituting `bash tables.sh/tables.sh` for
`./tables.c/tables` in any command produces the identical result.

## The two-file model

Every invocation takes exactly two JSON documents, in this order:

```bash
./tables.c/tables <layout.json> <data.json> [OPTIONS]
```

* **Layout** — an object describing the table: the `theme`, an optional `title` and
  `footer` with their positions, an optional `sort` specification, and the `columns`
  array. This is the file you tune when you want the table to *look* different.
* **Data** — an array of row objects. Each column pulls its value from the row using the
  column's `key`. Keys that are missing from a row are treated as null. This is the file
  that changes when the underlying facts change.

Keeping presentation and content apart is the central idea: the same data file can be
rendered a dozen different ways, and the same layout can be pointed at a nightly export
without editing a line of configuration. Most of the suites below exploit exactly that —
they hold the data constant and vary only the layout.

## Your first table

**`layout.json`**

```json
{
  "theme": "Red",
  "columns": [
    { "header": "Fruit", "key": "fruit", "datatype": "text", "justification": "left" },
    { "header": "Crates", "key": "crates", "datatype": "num", "justification": "right" }
  ]
}
```

**`data.json`**

```json
[
  { "fruit": "Apples",   "crates": 1200 },
  { "fruit": "Bananas",  "crates": 340 },
  { "fruit": "Cherries", "crates": 87 }
]
```

**Command**

```bash
./tables.c/tables layout.json data.json
```

**Output**

```text
╭──────────┬────────╮
│ Fruit    │ Crates │
├──────────┼────────┤
│ Apples   │  1,200 │
│ Bananas  │    340 │
│ Cherries │     87 │
╰──────────┴────────╯
```

Three things happened without being asked for. Column widths were measured from the
header and the widest cell, one space of padding was added on each side, and the `num`
datatype inserted a thousands separator into `1200`. Alignment came from
`justification`; everything else is a default you can override later.

## Anatomy of a rendered table

Adding a title, a footer, a group separator and a summary row exercises every region of
the frame at once.

**`layout.json`**

```json
{
  "theme": "Red",
  "title": "Warehouse Stock",
  "title_position": "center",
  "footer": "Close of business",
  "footer_position": "center",
  "columns": [
    { "header": "Region", "key": "region", "datatype": "text", "justification": "left",  "break": true },
    { "header": "Fruit",  "key": "fruit",  "datatype": "text", "justification": "left",  "summary": "count" },
    { "header": "Crates", "key": "crates", "datatype": "num",  "justification": "right", "summary": "sum" }
  ]
}
```

**`data.json`**

```json
[
  { "region": "North", "fruit": "Apples",   "crates": 1200 },
  { "region": "North", "fruit": "Bananas",  "crates": 340 },
  { "region": "South", "fruit": "Cherries", "crates": 87 },
  { "region": "South", "fruit": "Dates",    "crates": 1500 }
]
```

**Output**

```text
     ╭─────────────────╮
     │ Warehouse Stock │
╭────┴───┬──────────┬──┴─────╮
│ Region │ Fruit    │ Crates │
├────────┼──────────┼────────┤
│ North  │ Apples   │  1,200 │
│ North  │ Bananas  │    340 │
├────────┼──────────┼────────┤
│ South  │ Cherries │     87 │
│ South  │ Dates    │  1,500 │
├────────┼──────────┼────────┤
│        │ 4        │  3,127 │
╰───┬────┴──────────┴───┬────╯
    │ Close of business │
    ╰───────────────────╯
```

Reading top to bottom:

| Region | Produced by | Notes |
|--------|-------------|-------|
| Title box | `title`, `title_position` | A separate box welded onto the top border; it may be narrower or wider than the table |
| Header row | Column `header` values | Aligned with the column's `justification` |
| Data rows | The data array, in file order unless `sort` is set | |
| Break separator | `"break": true` on a column | Drawn whenever that column's value changes between adjacent rows |
| Summary row | `summary` on one or more columns | Separated from the body by its own rule; columns with `"summary": "none"` stay blank |
| Footer box | `footer`, `footer_position` | The mirror image of the title box |

The junction characters where the title and footer boxes meet the table are computed, not
guessed. When you compare implementations or debug a new port, those junctions are the
first place a width miscalculation shows up.

## Colour and `--mono`

By default the theme colours the border, header, caption, summary and footer separately,
and `{COLOR}` placeholders inside titles, footers and cell values are expanded to ANSI
escapes. Passing `--mono` disables both:

```bash
./tables.c/tables layout.json data.json --mono
```

`--mono` is not merely "strip the escapes afterwards" — the placeholders expand to empty
strings and the theme colours are never emitted, while every width calculation stays
identical. That is why the monochrome output is exactly the same shape as the coloured
output, and why the test suite validates both paths for every scenario.

**All output blocks in this catalogue were originally captured with `--mono`** so that
the geometry was legible in a browser and could be copied into a diff. Pages are being
converted, one at a time, to show the real coloured output instead — starting with
[EXAMPLES_01.md](EXAMPLES_01.md) — using
[**Oh.sh**](https://github.com/500Foods/Oh.sh), a small companion tool that reads raw
ANSI terminal output (colour codes and all, no `--mono`) and renders it as a crisp SVG,
which GitHub then displays inline just like any other image. In practice that means each
example is generated in two passes: the layout and data JSON are unchanged, but the
**Output** section is produced by piping the *coloured* run of `./tables.c/tables`
straight into `Oh`:

```bash
./tables.c/tables layout.json data.json | Oh --width 33 -o images/1-A.svg
```

The `--width` value matches the widest visible line of that particular table (measuring
the rendered text, not the source JSON), so each image is cropped tightly instead of
sitting in a fixed 80-column canvas. The generated SVGs live under [`images/`](images) —
one folder, named after the example label (`1-A.svg`, `1-B.svg`, …) — so every converted
page draws from the same consolidated location. Pages that have not yet been converted
still show a monochrome text block and are unaffected.

## Running any example yourself

Every example is addressed by a label such as `3-F`, which maps directly onto a suite
directory and a file pair:

```
tests/scenarios/suite_03/test_3_F_layout.json
tests/scenarios/suite_03/test_3_F_data.json
```

So to reproduce any table on these pages:

```bash
# C implementation
./tables.c/tables tests/scenarios/suite_03/test_3_F_layout.json \
                  tests/scenarios/suite_03/test_3_F_data.json

# Bash implementation — identical output
bash tables.sh/tables.sh tests/scenarios/suite_03/test_3_F_layout.json \
                         tests/scenarios/suite_03/test_3_F_data.json

# Monochrome, as shown on pages not yet converted to Oh.sh screenshots
./tables.c/tables tests/scenarios/suite_03/test_3_F_layout.json \
                  tests/scenarios/suite_03/test_3_F_data.json --mono

# Coloured SVG, as shown on EXAMPLES_01.md (requires Oh.sh: https://github.com/500Foods/Oh.sh)
./tables.c/tables tests/scenarios/suite_01/test_1_A_layout.json \
                  tests/scenarios/suite_01/test_1_A_data.json | Oh -o images/1-A.svg
```

One scenario (`9-A`) uses a *dynamic layout*: `test_9_A_layout.sh` is a shell script that
prints JSON on stdout and receives the data file as `$1`. Generate the layout first:

```bash
bash tests/scenarios/suite_09/test_9_A_layout.sh \
     tests/scenarios/suite_09/test_9_A_data.json > /tmp/9A_layout.json
./tables.c/tables /tmp/9A_layout.json tests/scenarios/suite_09/test_9_A_data.json
```

To run a whole suite through the comparison harness instead — which renders each scenario
with both implementations and diffs them — use the test runner:

```bash
bash tests/run_tests.sh 03          # one suite
bash tests/run_tests.sh 01 05 09    # several
bash tests/run_tests.sh             # everything, including linting
```

## How to read these pages

Each suite page follows the same shape, and each example within it is presented the same
way:

* **A heading** carrying the scenario label and a short description of the idea it
  isolates, for example `3-C — Right-justified clipping keeps the tail`.
* **What it demonstrates** — one or two sentences on the behaviour being exercised.
* **Layout** — the JSON that produced the table. Where a suite varies only one field
  across many scenarios, the page shows the changed fragment rather than repeating the
  whole file, and names the file so you can open the original.
* **Output** — the exact rendering. On converted pages (see
  [Colour and `--mono`](#colour-and---mono)) this is a coloured SVG captured with
  [Oh.sh](https://github.com/500Foods/Oh.sh); elsewhere it is still a monochrome text
  block.
* **What to look for** — the specific characters, widths or numbers that make the point.
  This is the part worth reading slowly; a table that looks fine at a glance is often
  demonstrating something subtle about padding or rounding.

Where you see an HTML comment of the form `<!-- screenshot:3-C -->` in the page source,
that is a reserved slot awaiting conversion to an Oh.sh screenshot for that example, in
the same way [EXAMPLES_01.md](EXAMPLES_01.md) has already been converted.

## The example catalogue

| Page | Suite | Examples | What you will learn |
|------|-------|---------:|---------------------|
| [Basic Rendering](EXAMPLES_01.md) | 01 | 9 | Themes, the six datatypes, the three justifications, hidden columns, fixed widths and `{COLOR}` placeholders |
| [Summaries](EXAMPLES_02.md) | 02 | 10 | `sum`, `min`, `max`, `avg`, `count`, `unique`, `blanks`, `nonblanks`, how nulls and zeros are counted, and annotated rows |
| [Clipping and Wrapping](EXAMPLES_03.md) | 03 | 11 | What `width` really constrains, how clipping follows justification, word wrapping, and wrapping on a delimiter |
| [Combined Features](EXAMPLES_04.md) | 04 | 5 | Realistic tables that mix breaks, summaries, wrapping and mixed datatypes in one layout |
| [Titles](EXAMPLES_05.md) | 05 | 5 | Adding a title, the four positions, and how a title interacts with the table it sits on |
| [Title Geometry](EXAMPLES_06.md) | 06 | 12 | The full position × title-length matrix, including what happens when a title is narrower, equal to, or wider than the table |
| [Footers](EXAMPLES_07.md) | 07 | 5 | The footer counterparts of suite 05, and how footers attach beneath a summary row |
| [Footer Geometry](EXAMPLES_08.md) | 08 | 17 | The footer position matrix, `full`-width behaviour, command substitution and Unicode width handling |
| [Showcase](EXAMPLES_09.md) | 09 | 22 | Twenty-two complete reports combining titles, footers, breaks, summaries, wrapping, dynamic content and emoji |

Suite 00 has no example page: it is the lint gate (`shellcheck` on the Bash
implementation, `cppcheck` on the C sources) rather than a rendering scenario.

## Feature index

Jump straight to the examples that exercise a particular feature.

| Feature | Layout field | Examples |
|---------|--------------|----------|
| Themes | `theme` | [1-A](EXAMPLES_01.md#1-a--text-and-integer-columns-with-a-hidden-duplicate) (Red), [1-B](EXAMPLES_01.md#1-b--numeric-datatypes-side-by-side) (Blue) |
| Left / centre / right alignment | `justification` | [1-F](EXAMPLES_01.md#1-f--one-value-three-justifications), [1-D](EXAMPLES_01.md#1-d--centring-every-column) |
| Integers and floats | `datatype: int` / `float` | [1-B](EXAMPLES_01.md#1-b--numeric-datatypes-side-by-side), [2-H](EXAMPLES_02.md#2-h--every-summary-over-a-float-column) |
| Thousands separators | `datatype: num` | [1-B](EXAMPLES_01.md#1-b--numeric-datatypes-side-by-side), [1-E](EXAMPLES_01.md#1-e--all-six-datatypes-in-one-table) |
| Kubernetes CPU and memory | `datatype: kcpu` / `kmem` | [1-C](EXAMPLES_01.md#1-c--kubernetes-cpu-and-memory-normalisation), [2-C](EXAMPLES_02.md#2-c--summing-kubernetes-quantities) |
| Hidden columns | `visible: false` | [1-A](EXAMPLES_01.md#1-a--text-and-integer-columns-with-a-hidden-duplicate), [2-J](EXAMPLES_02.md#2-j--annotated-rows-are-excluded-from-summaries) |
| Fixed column width | `width` | [1-G](EXAMPLES_01.md#1-g--colour-placeholders-in-data-and-title), [3-B](EXAMPLES_03.md#3-b--a-fixed-width-clips-from-the-right) |
| Clipping | `width` + `justification` | [1-H](EXAMPLES_01.md#1-h--clipping-coloured-text), [3-B](EXAMPLES_03.md#3-b--a-fixed-width-clips-from-the-right), [3-C](EXAMPLES_03.md#3-c--right-justified-clipping-keeps-the-tail), [3-D](EXAMPLES_03.md#3-d--centred-clipping-keeps-the-middle) |
| Word wrapping | `wrap_mode: wrap` | [3-F](EXAMPLES_03.md#3-f--word-wrapping-with-wrap_mode), [3-K](EXAMPLES_03.md#3-k--wide-column-wrapping), [4-A](EXAMPLES_04.md#4-a--breaks-summaries-and-a-wrapped-description) |
| Delimiter wrapping | `wrap_char` | [3-E](EXAMPLES_03.md#3-e--wrapping-on-a-delimiter-with-wrap_char), [3-H](EXAMPLES_03.md#3-h--breaks-clipping-and-delimiter-wrapping-together) |
| Group separators | `break: true` | [3-G](EXAMPLES_03.md#3-g--break-inserts-a-separator-when-a-value-changes), [4-E](EXAMPLES_04.md#4-e--a-full-width-report-layout), [5-D](EXAMPLES_05.md#5-d--right-aligned-title-over-a-broken-table), [9-I](EXAMPLES_09.md#9-i--grouping-by-location) |
| Summaries | `summary` | [2-A](EXAMPLES_02.md#2-a--sum-and-count) through [2-J](EXAMPLES_02.md#2-j--annotated-rows-are-excluded-from-summaries) |
| Blank / non-blank counts | `summary: blanks` / `nonblanks` | [2-I](EXAMPLES_02.md#2-i--counting-blanks-and-non-blanks) |
| Rows excluded from totals | `"annotate": true` in data | [2-J](EXAMPLES_02.md#2-j--annotated-rows-are-excluded-from-summaries) |
| Titles | `title`, `title_position` | [Suite 05](EXAMPLES_05.md), [Suite 06](EXAMPLES_06.md) |
| Footers | `footer`, `footer_position` | [Suite 07](EXAMPLES_07.md), [Suite 08](EXAMPLES_08.md) |
| Colour placeholders | `{RED}`, `{BOLD}`, `{NC}`, … | [1-G](EXAMPLES_01.md#1-g--colour-placeholders-in-data-and-title), [1-I](EXAMPLES_01.md#1-i--nested-and-multiple-placeholders), [8-M](EXAMPLES_08.md#8-m--full-width-footer-with-a-colour-placeholder) |
| Command substitution | `$(...)` in title/footer | [8-O](EXAMPLES_08.md#8-o--command-substitution-in-a-footer), [8-P](EXAMPLES_08.md#8-p--command-substitution-in-both-title-and-footer), [9-C](EXAMPLES_09.md#9-c--full-width-title-with-a-generated-footer), [9-U](EXAMPLES_09.md#9-u--two-commands-in-one-footer) |
| Unicode and emoji widths | any text | [8-Q](EXAMPLES_08.md#8-q--unicode-in-title-and-footer), [9-V](EXAMPLES_09.md#9-v--an-emoji-heavy-report) |
| Layout generated by a script | `*_layout.sh` | [9-A](EXAMPLES_09.md#9-a--a-layout-generated-by-a-shell-script) |

## Further reading

* [`README.md`](README.md) — project overview and repository statistics
* [`tables.md`](tables.md) — complete layout and data reference
* [`tables_technical.md`](tables_technical.md) — internals and rendering pipeline
* [`tables_developer.md`](tables_developer.md) — contributing and porting notes
* [`tests/README.md`](tests/README.md) — how the comparison suite works and how to add cases
