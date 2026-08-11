# Examples 06 — Title Geometry

[← Titles](EXAMPLES_05.md) · [Examples index](EXAMPLES.md) · [Next: Footers →](EXAMPLES_07.md)

Suite 06 is a controlled experiment. It renders the same four rows twelve times, varying
only two things: the `title_position` and the relationship between the title's width and
the table's width. The result is a complete map of how the title box behaves, and it is
the reference to reach for when a title is not landing where you expected.

**Source:** [`tests/scenarios/suite_06/`](tests/scenarios/suite_06) · **Examples:** 12 ·
**Run the suite:** `bash tests/run_tests.sh 06`

---

## The data

Deliberately minimal — four rows, four short columns, no summaries, no wrapping. Nothing
competes with the title for attention.

```json
[
  {
    "id": 1,
    "server_name": "web-server-01",
    "cpu_cores": 4,
    "load_avg": 2.45
  },
  {
    "id": 2,
    "server_name": "db-server-01",
    "cpu_cores": 8,
    "load_avg": 5.12
  },
  {
    "id": 3,
    "server_name": "cache-server",
    "cpu_cores": 2,
    "load_avg": 0.85
  },
  {
    "id": 4,
    "server_name": "api-gateway",
    "cpu_cores": 6,
    "load_avg": 3.21
  }
]
```

## The matrix

| | Title narrower than table | Title exactly table width | Title wider than table |
|--|--|--|--|
| **position omitted** | [6-A](#6-a--short-title-no-position) | [6-B](#6-b--title-exactly-as-wide-as-the-table) | [6-C](#6-c--an-over-wide-title-with-no-position) |
| **`left`** | [6-D](#6-d--short-title-left) | [6-E](#6-e--equal-width-title-left) | [6-F](#6-f--over-wide-title-left) |
| **`center`** | [6-G](#6-g--short-title-center) | [6-H](#6-h--equal-width-title-center) | [6-I](#6-i--over-wide-title-center) |
| **`right`** | [6-J](#6-j--short-title-right) | [6-K](#6-k--equal-width-title-right) | [6-L](#6-l--over-wide-title-right-with-an-auto-width-column) |

Three column configurations are used to produce the three width relationships. Each
example below shows only its `title` and `title_position`; the column sets are listed here
once.

**Set α — auto widths (table renders 45 characters wide).** Used by 6-A, 6-D, 6-G and 6-J.

```json
"columns": [
  { "header": "ID",          "key": "id",         "datatype": "int",   "justification": "right" },
  { "header": "Server Name", "key": "server_name","datatype": "text",  "justification": "left"  },
  { "header": "CPU Cores",   "key": "cpu_cores",  "datatype": "num",   "justification": "right" },
  { "header": "Load Avg",    "key": "load_avg",   "datatype": "float", "justification": "right" }
]
```

**Set β — fixed widths 5 / 15 / 10 / 10 (table renders 45 characters wide).** Used by 6-B,
6-E, 6-H and 6-K, whose 41-character title produces a box of exactly 45.

```json
"columns": [
  { "header": "ID",          "key": "id",          "datatype": "int",   "justification": "right", "width": 5  },
  { "header": "Server Name", "key": "server_name", "datatype": "text",  "justification": "left",  "width": 15 },
  { "header": "CPU Cores",   "key": "cpu_cores",   "datatype": "num",   "justification": "right", "width": 10 },
  { "header": "Load Avg",    "key": "load_avg",    "datatype": "float", "justification": "right", "width": 10 }
]
```

**Set γ — fixed widths 3 / 10 / 5 / 5 (table renders 28 characters wide).** Used by 6-C,
6-F and 6-I. 6-C additionally uses longer header text.

**Set δ — fixed widths 3 / 10 / 5 and one auto column (table renders 33 characters wide).**
Used by 6-L.

---

## 6-A — Short title, no position

<!-- screenshot:6-A -->

**Layout delta** — `tests/scenarios/suite_06/test_6_A_layout.json`, column set α

```json
"title": "Server Report"
```

**Output**

```text
╭───────────────╮
│ Server Report │
├────┬──────────┴────┬───────────┬──────────╮
│ ID │ Server Name   │ CPU Cores │ Load Avg │
├────┼───────────────┼───────────┼──────────┤
│  1 │ web-server-01 │         4 │     2.45 │
│  2 │ db-server-01  │         8 │     5.12 │
│  3 │ cache-server  │         2 │     0.85 │
│  4 │ api-gateway   │         6 │     3.21 │
╰────┴───────────────┴───────────┴──────────╯
```

**What to look for**

* With no `title_position`, a short title is drawn at the **left**. This is the default and
  it matches 6-D exactly.
* The 17-character box sits over a 45-character table, and the table's top border resumes
  immediately to the right of it: `├────┬──────────┴────┬───────────┬──────────╮`. The `┴`
  marks where the title box's right wall lands mid-column.

---

## 6-B — Title exactly as wide as the table

<!-- screenshot:6-B -->

**Layout delta** — `tests/scenarios/suite_06/test_6_B_layout.json`, column set β

```json
"title": "Server Performance Metrics Report Data 23"
```

**Output**

```text
╭───────────────────────────────────────────╮
│ Server Performance Metrics Report Data 23 │
├─────┬───────────────┬──────────┬──────────┤
│  ID │ Server Name   │ PU Cores │ Load Avg │
├─────┼───────────────┼──────────┼──────────┤
│   1 │ web-server-01 │        4 │     2.45 │
│   2 │ db-server-01  │        8 │     5.12 │
│   3 │ cache-server  │        2 │     0.85 │
│   4 │ api-gateway   │        6 │     3.21 │
╰─────┴───────────────┴──────────┴──────────╯
```

**What to look for**

* The title box and the table are both 45 characters, so the top border is a single clean
  rule and the `title_position` becomes irrelevant. **6-B, 6-E, 6-H and 6-K produce
  byte-identical output** — the only difference between their layouts is a position that
  has nothing left to decide.
* This is what `full` achieves deliberately; here it happens by arithmetic.
* Notice the header row: `CPU Cores` has become `PU Cores`. Column set β pins that column
  at `"width": 10`, leaving eight characters of content, and the column is right-justified
  so the clip keeps the tail. Fixing widths to line a title up can quietly cost you a
  header.

---

## 6-C — An over-wide title with no position

<!-- screenshot:6-C -->

**Layout delta** — `tests/scenarios/suite_06/test_6_C_layout.json`, column set γ with long
headers

```json
"title": "Detailed Server Performance and Configuration Report for Q2 2023"
```

**Output**

```text
╭──────────────────────────────────────────────────────────────────╮
│ Detailed Server Performance and Configuration Report for Q2 2023 │
├───┬──────────┬─────┬─────┬───────────────────────────────────────╯
│ r │ Server N │ unt │ lue │
├───┼──────────┼─────┼─────┤
│ 1 │ web-serv │   4 │ .45 │
│ 2 │ db-serve │   8 │ .12 │
│ 3 │ cache-se │   2 │ .85 │
│ 4 │ api-gate │   6 │ .21 │
╰───┴──────────┴─────┴─────╯
```

**What to look for**

* The title is 64 characters, the box is 68, and the table is 28. Because no position was
  given, **the title is not clipped** — the box simply extends past the table and the
  table's top border runs out to meet it, closing with `╯` on the right.
* This is the behaviour to use when the title matters more than the outline: a wide banner
  above a narrow table.
* Compare directly with [6-F](#6-f--over-wide-title-left), which is the same title on the
  same columns with `"title_position": "left"` added. That single field cuts the title to
  24 characters.
* The headers are heavily clipped by set γ's narrow widths: `ID Number` right-justified in
  one character of content becomes `r`; `CPU Cores Count` becomes `unt`; `Load Average
  Value` becomes `lue`. An extreme illustration of the rule from
  [3-C](EXAMPLES_03.md#3-c--right-justified-clipping-keeps-the-tail).

---

## 6-D — Short title, `left`

<!-- screenshot:6-D -->

**Layout delta** — `tests/scenarios/suite_06/test_6_D_layout.json`, column set α

```json
"theme": "Blue",
"title": "Server Report",
"title_position": "left"
```

**Output**

```text
╭───────────────╮
│ Server Report │
├────┬──────────┴────┬───────────┬──────────╮
│ ID │ Server Name   │ CPU Cores │ Load Avg │
├────┼───────────────┼───────────┼──────────┤
│  1 │ web-server-01 │         4 │     2.45 │
│  2 │ db-server-01  │         8 │     5.12 │
│  3 │ cache-server  │         2 │     0.85 │
│  4 │ api-gateway   │         6 │     3.21 │
╰────┴───────────────┴───────────┴──────────╯
```

**What to look for**

* Identical geometry to 6-A. For a title that fits, `left` and the default are the same
  thing.
* The theme is `Blue` here rather than `Red`, which changes nothing in the monochrome
  capture — as it must.

---

## 6-E — Equal-width title, `left`

<!-- screenshot:6-E -->

**Layout delta** — `tests/scenarios/suite_06/test_6_E_layout.json`, column set β

```json
"theme": "Blue",
"title": "Server Performance Metrics Report Data 23",
"title_position": "left"
```

**Output**

```text
╭───────────────────────────────────────────╮
│ Server Performance Metrics Report Data 23 │
├─────┬───────────────┬──────────┬──────────┤
│  ID │ Server Name   │ PU Cores │ Load Avg │
├─────┼───────────────┼──────────┼──────────┤
│   1 │ web-server-01 │        4 │     2.45 │
│   2 │ db-server-01  │        8 │     5.12 │
│   3 │ cache-server  │        2 │     0.85 │
│   4 │ api-gateway   │        6 │     3.21 │
╰─────┴───────────────┴──────────┴──────────╯
```

**What to look for**

* Identical to 6-B, 6-H and 6-K. When the box already fills the width there is nowhere to
  align it.

---

## 6-F — Over-wide title, `left`

<!-- screenshot:6-F -->

**Layout delta** — `tests/scenarios/suite_06/test_6_F_layout.json`, column set γ

```json
"theme": "Blue",
"title": "Detailed Server Performance and Configuration Analysis Report for Q2 2023",
"title_position": "left"
```

**Output**

```text
╭──────────────────────────╮
│ Detailed Server Performa │
├───┬──────────┬─────┬─────┤
│ D │ Server N │ res │ Avg │
├───┼──────────┼─────┼─────┤
│ 1 │ web-serv │   4 │ .45 │
│ 2 │ db-serve │   8 │ .12 │
│ 3 │ cache-se │   2 │ .85 │
│ 4 │ api-gate │   6 │ .21 │
╰───┴──────────┴─────┴─────╯
```

**What to look for**

* The 73-character title is clipped to the 24 characters the 28-wide table can hold:
  `Detailed Server Performa`. The head survives, matching the `left` justification.
* The table's top border is now a plain rule with no overhang. Naming a position is what
  guarantees the title never escapes the table outline.
* This is the direct counterpart to 6-C. Same columns, same intent, opposite policy.

---

## 6-G — Short title, `center`

<!-- screenshot:6-G -->

**Layout delta** — `tests/scenarios/suite_06/test_6_G_layout.json`, column set α

```json
"title": "Server Report",
"title_position": "center"
```

**Output**

```text
              ╭───────────────╮
              │ Server Report │
╭────┬────────┴──────┬────────┴──┬──────────╮
│ ID │ Server Name   │ CPU Cores │ Load Avg │
├────┼───────────────┼───────────┼──────────┤
│  1 │ web-server-01 │         4 │     2.45 │
│  2 │ db-server-01  │         8 │     5.12 │
│  3 │ cache-server  │         2 │     0.85 │
│  4 │ api-gateway   │         6 │     3.21 │
╰────┴───────────────┴───────────┴──────────╯
```

**What to look for**

* The 17-character box is centred over the 45-character table, inset 14 characters from the
  left.
* Both walls of the title box now land inside the table, so the top border shows two `┴`
  junctions: `╭────┬────────┴──────┬────────┴──┬──────────╮`. Neither of them lines up with
  a column separator, and neither is nudged to make it do so — the geometry is exact rather
  than snapped.

---

## 6-H — Equal-width title, `center`

<!-- screenshot:6-H -->

**Layout delta** — `tests/scenarios/suite_06/test_6_H_layout.json`, column set β

```json
"title": "Server Performance Metrics Report Data 23",
"title_position": "center"
```

**Output**

```text
╭───────────────────────────────────────────╮
│ Server Performance Metrics Report Data 23 │
├─────┬───────────────┬──────────┬──────────┤
│  ID │ Server Name   │ PU Cores │ Load Avg │
├─────┼───────────────┼──────────┼──────────┤
│   1 │ web-server-01 │        4 │     2.45 │
│   2 │ db-server-01  │        8 │     5.12 │
│   3 │ cache-server  │        2 │     0.85 │
│   4 │ api-gateway   │        6 │     3.21 │
╰─────┴───────────────┴──────────┴──────────╯
```

**What to look for**

* Identical to 6-B, 6-E and 6-K, completing the equal-width row of the matrix.

---

## 6-I — Over-wide title, `center`

<!-- screenshot:6-I -->

**Layout delta** — `tests/scenarios/suite_06/test_6_I_layout.json`, column set γ

```json
"title": "Detailed Server Performance and Configuration Analysis Report for Q2 2023",
"title_position": "center"
```

**Output**

```text
╭──────────────────────────╮
│ Detailed Server Performa │
├───┬──────────┬─────┬─────┤
│ D │ Server N │ res │ Avg │
├───┼──────────┼─────┼─────┤
│ 1 │ web-serv │   4 │ .45 │
│ 2 │ db-serve │   8 │ .12 │
│ 3 │ cache-se │   2 │ .85 │
│ 4 │ api-gate │   6 │ .21 │
╰───┴──────────┴─────┴─────╯
```

**What to look for**

* Clipped to `Detailed Server Performa` — **the same fragment as 6-F**, not the middle of
  the string. Once a title has been cut down to exactly the table width there is no slack
  left to centre it in, so `center` and `left` converge.
* Contrast with cell clipping, where centring genuinely keeps the middle
  ([3-D](EXAMPLES_03.md#3-d--centred-clipping-keeps-the-middle)). The title's two-step
  fit — clip to width, then place — means the placement step has nothing to do.

---

## 6-J — Short title, `right`

<!-- screenshot:6-J -->

**Layout delta** — `tests/scenarios/suite_06/test_6_J_layout.json`, column set α

```json
"theme": "Blue",
"title": "Server Report FY2025",
"title_position": "right"
```

**Output**

```text
                     ╭──────────────────────╮
                     │ Server Report FY2025 │
╭────┬───────────────┼───────────┬──────────┤
│ ID │ Server Name   │ CPU Cores │ Load Avg │
├────┼───────────────┼───────────┼──────────┤
│  1 │ web-server-01 │         4 │     2.45 │
│  2 │ db-server-01  │         8 │     5.12 │
│  3 │ cache-server  │         2 │     0.85 │
│  4 │ api-gateway   │         6 │     3.21 │
╰────┴───────────────┴───────────┴──────────╯
```

**What to look for**

* The 24-character box is flush with the table's right edge, so the two right walls line
  up and the border closes with `┤` rather than a corner.
* The box's left wall happens to land exactly on a column separator, and the renderer emits
  `┼` for that coincidence rather than the `┴` seen in 6-A and 6-G:
  `╭────┬───────────────┼───────────┬──────────┤`. Getting that character right is a
  genuine test of the junction logic.
* The title text differs from 6-A/6-D/6-G (`FY2025` is appended) purely so the box lands on
  a different column boundary and the join has to be computed rather than inherited.

---

## 6-K — Equal-width title, `right`

<!-- screenshot:6-K -->

**Layout delta** — `tests/scenarios/suite_06/test_6_K_layout.json`, column set β

```json
"theme": "Blue",
"title": "Server Performance Metrics Report Data 23",
"title_position": "right"
```

**Output**

```text
╭───────────────────────────────────────────╮
│ Server Performance Metrics Report Data 23 │
├─────┬───────────────┬──────────┬──────────┤
│  ID │ Server Name   │ PU Cores │ Load Avg │
├─────┼───────────────┼──────────┼──────────┤
│   1 │ web-server-01 │        4 │     2.45 │
│   2 │ db-server-01  │        8 │     5.12 │
│   3 │ cache-server  │        2 │     0.85 │
│   4 │ api-gateway   │        6 │     3.21 │
╰─────┴───────────────┴──────────┴──────────╯
```

**What to look for**

* The fourth and final member of the identical-output group. If a port of this library
  ever renders 6-B, 6-E, 6-H and 6-K differently from one another, its title placement is
  branching where it should not.

---

## 6-L — Over-wide title, `right`, with an auto-width column

<!-- screenshot:6-L -->

**Layout delta** — `tests/scenarios/suite_06/test_6_L_layout.json`, column set δ

```json
"theme": "Blue",
"title": "Detailed Server Performance and Configuration Analysis Report for Q2 2023",
"title_position": "right"
```

**Output**

```text
╭───────────────────────────────╮
│ Detailed Server Performance a │
├───┬──────────┬─────┬──────────┤
│ D │ Server N │ res │ Load Avg │
├───┼──────────┼─────┼──────────┤
│ 1 │ web-serv │   4 │     2.45 │
│ 2 │ db-serve │   8 │     5.12 │
│ 3 │ cache-se │   2 │     0.85 │
│ 4 │ api-gate │   6 │     3.21 │
╰───┴──────────┴─────┴──────────╯
```

**What to look for**

* The final column has no `width`, so it measures itself from `Load Avg` and comes out at
  10 rather than the 5 used in set γ. The table is 33 characters wide instead of 28, and
  the title is therefore clipped to 29 characters — `Detailed Server Performance a` —
  rather than 24.
* **The available title width is a consequence of the column widths.** Change a column,
  change the title. If a title must always show a particular phrase, either pin the column
  widths or omit `title_position` so the box can overhang.
* Like 6-F and 6-I, the clip keeps the head even though the position is `right`, because
  the clip happens before the placement.

---

## Takeaways

* Omitting `title_position` lets an over-wide title overhang the table intact. Naming any
  position clips it to the table width.
* When the title box and the table are the same width, all four positions render
  identically.
* Over-wide titles are clipped head-first regardless of position, because clipping precedes
  placement. Position only matters when there is slack.
* The space available to a title is `table width − 4`, and the table width is decided
  entirely by the columns.
* Pinning column widths to make a title fit can silently clip your headers — check them
  after any width change.

---

[← Titles](EXAMPLES_05.md) · [Examples index](EXAMPLES.md) · [Next: Footers →](EXAMPLES_07.md)
