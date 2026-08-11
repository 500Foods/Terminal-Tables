# Examples 02 — Summaries

[← Basic Rendering](EXAMPLES_01.md) · [Examples index](EXAMPLES.md) · [Next: Clipping and Wrapping →](EXAMPLES_03.md)

Suite 02 is about the row at the bottom. Setting `summary` on a column adds an
aggregation rule; if any column in the layout has one, a separator and a summary row are
appended to the table. This suite walks every aggregation the library supports, shows how
each behaves against integers, floats, text and Kubernetes quantities, and covers the two
mechanisms for keeping specific values out of the arithmetic.

**Source:** [`tests/scenarios/suite_02/`](tests/scenarios/suite_02) · **Examples:** 10 ·
**Run the suite:** `bash tests/run_tests.sh 02`

---

## The data

Examples 2-A through 2-I share a five-row dataset. It is deliberately awkward: it contains
nulls, numeric zeros, an empty string, and a fifth server whose resource usage is zero.

```json
[
  {
    "id": 1,
    "name": "web-server-01",
    "cpu_cores": 4,
    "load_avg": 2.45,
    "cpu_usage": "1250m",
    "memory_usage": "2048Mi",
    "status": "Running",
    "test_int": 100,
    "test_float": 1.23,
    "test_string": "hello"
  },
  {
    "id": 2,
    "name": "db-server-01",
    "cpu_cores": 8,
    "load_avg": 5.12,
    "cpu_usage": "3200m",
    "memory_usage": "8192Mi",
    "status": "Running",
    "test_int": 0,
    "test_float": 0.0,
    "test_string": ""
  },
  {
    "id": 3,
    "name": "cache-server",
    "cpu_cores": 2,
    "load_avg": 0.85,
    "cpu_usage": "500m",
    "memory_usage": "1024Mi",
    "status": "Starting",
    "test_int": null,
    "test_float": null,
    "test_string": null
  },
  {
    "id": 4,
    "name": "api-gateway",
    "cpu_cores": 6,
    "load_avg": 3.21,
    "cpu_usage": "2100m",
    "memory_usage": "4096Mi",
    "status": "Running",
    "test_int": 200,
    "test_float": 4.56,
    "test_string": "world"
  },
  {
    "id": 5,
    "name": "backup-server",
    "cpu_cores": 4,
    "load_avg": 1.5,
    "cpu_usage": "0m",
    "memory_usage": "0Mi",
    "status": "Idle",
    "test_int": 0,
    "test_float": 0.0,
    "test_string": ""
  }
]
```

Row 3 (`cache-server`) has `null` in all three `test_*` fields. Rows 2 and 5 have `0`,
`0.0` and `""`. Those are the values that separate a `count` from a `nonblanks`, and they
are why this dataset exists.

Example 2-J uses its own data file, shown with that example.

## The summary vocabulary

| Summary | Applies to | Produces |
|---------|-----------|----------|
| `sum` | `int`, `num`, `float`, `kcpu`, `kmem` | Total of all non-annotated values |
| `min` / `max` | numeric types | Smallest / largest value |
| `avg` | numeric types | Mean, formatted using the column's own datatype |
| `count` | any | Number of rows contributing a value |
| `unique` | any | Number of distinct values |
| `blanks` | any | Number of rows rendering as blank |
| `nonblanks` | any | Number of rows rendering as something |
| `none` | any | No summary for this column (the default) |

Summary cells obey the column's `justification`, and numeric summaries are formatted with
the same datatype rules as the body, so a `num` total keeps its thousands separators and a
`kcpu` total keeps its `m` suffix.

---

## 2-A — Sum and count

<!-- screenshot:2-A -->

**What it demonstrates.** The two aggregations you will reach for first, and the fact that
a `count` is perfectly legal on a text column.

**Layout** — `tests/scenarios/suite_02/test_2_A_layout.json`

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Server Name",
      "key": "name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
```

**Output**

```text
╭────┬───────────────┬───────────╮
│ ID │ Server Name   │ CPU Cores │
├────┼───────────────┼───────────┤
│  1 │ web-server-01 │         4 │
│  2 │ db-server-01  │         8 │
│  3 │ cache-server  │         2 │
│  4 │ api-gateway   │         6 │
│  5 │ backup-server │         4 │
├────┼───────────────┼───────────┤
│ 15 │ 5             │        24 │
╰────┴───────────────┴───────────╯
```

**What to look for**

* A separator rule appears above the summary row automatically. You do not add it; it is
  implied by the presence of any `summary` in the layout.
* `ID` totals to 15 (1+2+3+4+5) and `CPU Cores` to 24. Summing an identifier column is
  meaningless in practice but useful here as an unambiguous arithmetic check.
* `Server Name` carries `"summary": "count"` and reports `5`. Counting is the summary that
  makes sense on text.
* The `5` in the `Server Name` column sits at the **left** edge, matching that column's
  `"justification": "left"`. Summary values are not specially aligned — they inherit.

---

## 2-B — Min and max

<!-- screenshot:2-B -->

**What it demonstrates.** Range aggregations, and that different columns in the same table
can use different summaries.

**Layout** — `tests/scenarios/suite_02/test_2_B_layout.json`

```json
{
  "theme": "Blue",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "min"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "max"
    },
    {
      "header": "Load Average",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "min"
    }
  ]
}
```

**Output**

```text
╭────┬───────────┬──────────────╮
│ ID │ CPU Cores │ Load Average │
├────┼───────────┼──────────────┤
│  1 │         4 │         2.45 │
│  2 │         8 │         5.12 │
│  3 │         2 │         0.85 │
│  4 │         6 │         3.21 │
│  5 │         4 │         1.50 │
├────┼───────────┼──────────────┤
│  1 │         8 │         0.85 │
╰────┴───────────┴──────────────╯
```

**What to look for**

* `ID` reports its `min` (1), `CPU Cores` its `max` (8) and `Load Average` its `min`
  (0.85). Three columns, three different rules, one row.
* The `Load Average` minimum keeps two decimal places because the column is `float`. A
  summary is formatted by the column that owns it, not by a generic number formatter.
* Nothing in the summary row identifies *which* aggregation produced each value. If a
  table mixes summary types, say so in the header text — see
  [2-G](#2-g--every-summary-over-an-integer-column) for the pattern.

---

## 2-C — Summing Kubernetes quantities

<!-- screenshot:2-C -->

**What it demonstrates.** `sum` over `kcpu` and `kmem`, and how zero values are presented.

**Layout** — `tests/scenarios/suite_02/test_2_C_layout.json`

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "Server",
      "key": "name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "CPU Usage",
      "key": "cpu_usage",
      "datatype": "kcpu",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Memory Usage",
      "key": "memory_usage",
      "datatype": "kmem",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
```

**Output**

```text
╭───────────────┬───────────┬──────────────╮
│ Server        │ CPU Usage │ Memory Usage │
├───────────────┼───────────┼──────────────┤
│ web-server-01 │    1,250m │       2,048M │
│ db-server-01  │    3,200m │       8,192M │
│ cache-server  │      500m │       1,024M │
│ api-gateway   │    2,100m │       4,096M │
│ backup-server │           │           0M │
├───────────────┼───────────┼──────────────┤
│ 5             │    7,050m │      15,360M │
╰───────────────┴───────────┴──────────────╯
```

**What to look for**

* The CPU total is `7,050m` — the sum of 1250, 3200, 500, 2100 and 0 millicores, re-emitted
  through the `kcpu` formatter. The memory total is `15,360M` on the same principle.
* Look at the `backup-server` row. Its `cpu_usage` of `"0m"` renders **blank**, because the
  default `zero_value` is `blank`, while its `memory_usage` of `"0Mi"` renders as `0M`. If
  you need the two columns to behave identically, set `zero_value` explicitly on both
  rather than relying on the default.
* A blank cell still participates in the sum. Suppressing the display of a zero does not
  remove it from the arithmetic — it contributes nothing because it is zero, not because
  it is hidden.

---

## 2-D — Unique and average

<!-- screenshot:2-D -->

**What it demonstrates.** `unique` as a cardinality check, and `avg` on a float column.

**Layout** — `tests/scenarios/suite_02/test_2_D_layout.json`

```json
{
  "theme": "Blue",
  "columns": [
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center",
      "summary": "unique"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "unique"
    },
    {
      "header": "Load Average",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "avg"
    }
  ]
}
```

**Output**

```text
╭──────────┬───────────┬──────────────╮
│  Status  │ CPU Cores │ Load Average │
├──────────┼───────────┼──────────────┤
│ Running  │         4 │         2.45 │
│ Running  │         8 │         5.12 │
│ Starting │         2 │         0.85 │
│ Running  │         6 │         3.21 │
│   Idle   │         4 │         1.50 │
├──────────┼───────────┼──────────────┤
│    3     │         4 │         2.63 │
╰──────────┴───────────┴──────────────╯
```

**What to look for**

* `Status` reports `3` distinct values across five rows — `Running`, `Starting` and `Idle`.
  This is the summary to use when you want "how many environments / namespaces / regions
  does this report cover".
* `CPU Cores` reports `4` unique values (4, 8, 2, 6) even though there are five rows: the
  value `4` occurs twice. Contrast with a `count`, which would report 5.
* `Load Average` averages 13.13 across five rows and renders `2.63`. The mean is rounded to
  the precision of the column's datatype, not truncated.

---

## 2-E — A summary on every column

<!-- screenshot:2-E -->

**What it demonstrates.** A realistic wide table in which each of the six columns carries
whichever aggregation actually suits it.

**Layout** — `tests/scenarios/suite_02/test_2_E_layout.json`

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Name",
      "key": "name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "max"
    },
    {
      "header": "Load",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "min"
    },
    {
      "header": "CPU",
      "key": "cpu_usage",
      "datatype": "kcpu",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Memory",
      "key": "memory_usage",
      "datatype": "kmem",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
```

**Output**

```text
╭────┬───────────────┬───────┬──────┬────────┬─────────╮
│ ID │ Name          │ Cores │ Load │    CPU │  Memory │
├────┼───────────────┼───────┼──────┼────────┼─────────┤
│  1 │ web-server-01 │     4 │ 2.45 │ 1,250m │  2,048M │
│  2 │ db-server-01  │     8 │ 5.12 │ 3,200m │  8,192M │
│  3 │ cache-server  │     2 │ 0.85 │   500m │  1,024M │
│  4 │ api-gateway   │     6 │ 3.21 │ 2,100m │  4,096M │
│  5 │ backup-server │     4 │ 1.50 │        │      0M │
├────┼───────────────┼───────┼──────┼────────┼─────────┤
│ 15 │ 5             │     8 │ 0.85 │ 7,050m │ 15,360M │
╰────┴───────────────┴───────┴──────┴────────┴─────────╯
```

**What to look for**

* Six columns, five different summary types: `sum`, `count`, `max`, `min`, `sum`, `sum`.
  This is what a production report tends to look like — identifiers counted, resources
  totalled, load bounded.
* The `Memory` column widens from the body's requirement to accommodate `15,360M` in the
  summary row. **Summary values are measured for width along with the data**, so a total
  that is wider than any individual value will widen the column.
* `CPU` shows a blank for `backup-server` yet still contributes to the `7,050m` total, as
  in 2-C.

---

## 2-F — Opting a column out with `none`

<!-- screenshot:2-F -->

**What it demonstrates.** `"summary": "none"` — the explicit form of the default — and how
a partially populated summary row is rendered.

**Layout** — `tests/scenarios/suite_02/test_2_F_layout.json`

```json
{
  "theme": "Blue",
  "columns": [
    {
      "header": "Server Name",
      "key": "name",
      "datatype": "text",
      "justification": "left",
      "summary": "none"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center",
      "summary": "unique"
    },
    {
      "header": "Total CPU",
      "key": "cpu_usage",
      "datatype": "kcpu",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
```

**Output**

```text
╭───────────────┬──────────┬───────────╮
│ Server Name   │  Status  │ Total CPU │
├───────────────┼──────────┼───────────┤
│ web-server-01 │ Running  │    1,250m │
│ db-server-01  │ Running  │    3,200m │
│ cache-server  │ Starting │      500m │
│ api-gateway   │ Running  │    2,100m │
│ backup-server │   Idle   │           │
├───────────────┼──────────┼───────────┤
│               │    3     │    7,050m │
╰───────────────┴──────────┴───────────╯
```

**What to look for**

* `Server Name` is explicitly `"summary": "none"`, so its summary cell is empty — but the
  cell, its padding and its borders are still drawn. The summary row spans the full table
  regardless of how many columns actually contribute.
* Writing `"none"` is identical in effect to omitting `summary` altogether. It is worth
  writing out when a column sits between two summarised columns and you want the intent to
  be obvious to the next reader.
* The summary row exists at all only because *some* column has an aggregation. Remove the
  `unique` and `sum` here and the table would end at the last data row.

---

## 2-G — Every summary over an integer column

<!-- screenshot:2-G -->

**What it demonstrates.** The same `cpu_cores` field rendered six times, once per summary
type, so the aggregations can be compared directly against a single set of inputs.

**Layout** — `tests/scenarios/suite_02/test_2_G_layout.json`

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "Sum",
      "key": "cpu_cores",
      "datatype": "int",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Min",
      "key": "cpu_cores",
      "datatype": "int",
      "justification": "right",
      "summary": "min"
    },
    {
      "header": "Max",
      "key": "cpu_cores",
      "datatype": "int",
      "justification": "right",
      "summary": "max"
    },
    {
      "header": "Avg",
      "key": "cpu_cores",
      "datatype": "int",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "Count",
      "key": "cpu_cores",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Unique",
      "key": "cpu_cores",
      "datatype": "int",
      "justification": "right",
      "summary": "unique"
    }
  ]
}
```

**Output**

```text
╭─────┬─────┬─────┬─────┬───────┬────────╮
│ Sum │ Min │ Max │ Avg │ Count │ Unique │
├─────┼─────┼─────┼─────┼───────┼────────┤
│   4 │   4 │   4 │   4 │     4 │      4 │
│   8 │   8 │   8 │   8 │     8 │      8 │
│   2 │   2 │   2 │   2 │     2 │      2 │
│   6 │   6 │   6 │   6 │     6 │      6 │
│   4 │   4 │   4 │   4 │     4 │      4 │
├─────┼─────┼─────┼─────┼───────┼────────┤
│  24 │   2 │   8 │   5 │     5 │      4 │
╰─────┴─────┴─────┴─────┴───────┴────────╯
```

**What to look for**

* Every body row repeats the same number six times — that is the point. Only the last row
  differs, and it reads: sum 24, min 2, max 8, avg 5, count 5, unique 4.
* **The average is 4.8, and it is displayed as `5`.** The column is `int`, so the mean is
  rounded to the column's precision rather than shown with decimals. If you need `4.8`,
  declare the column as `float` — see the next example.
* `count` is 5 and `unique` is 4 because `4` appears twice in the data. Seeing both side by
  side on identical input is the clearest way to remember which is which.
* Naming each header after its summary is a small but effective documentation habit for
  tables that mix aggregation types.

---

## 2-H — Every summary over a float column

<!-- screenshot:2-H -->

**What it demonstrates.** The same six-way comparison against `load_avg`, showing how the
float datatype changes the presentation of the results.

**Layout** — `tests/scenarios/suite_02/test_2_H_layout.json`

```json
{
  "theme": "Blue",
  "columns": [
    {
      "header": "Sum",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Min",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "min"
    },
    {
      "header": "Max",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "max"
    },
    {
      "header": "Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "Count",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Unique",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "unique"
    }
  ]
}
```

**Output**

```text
╭───────┬──────┬──────┬──────┬───────┬────────╮
│   Sum │  Min │  Max │  Avg │ Count │ Unique │
├───────┼──────┼──────┼──────┼───────┼────────┤
│  2.45 │ 2.45 │ 2.45 │ 2.45 │  2.45 │   2.45 │
│  5.12 │ 5.12 │ 5.12 │ 5.12 │  5.12 │   5.12 │
│  0.85 │ 0.85 │ 0.85 │ 0.85 │  0.85 │   0.85 │
│  3.21 │ 3.21 │ 3.21 │ 3.21 │  3.21 │   3.21 │
│  1.50 │ 1.50 │ 1.50 │ 1.50 │  1.50 │   1.50 │
├───────┼──────┼──────┼──────┼───────┼────────┤
│ 13.13 │ 0.85 │ 5.12 │ 2.63 │     5 │      5 │
╰───────┴──────┴──────┴──────┴───────┴────────╯
```

**What to look for**

* `sum` is `13.13` and `avg` is `2.63` — both carry two decimal places, unlike the integer
  column in 2-G where the mean was rounded to a whole number.
* `count` and `unique` report `5` and `5` **without** decimal places. Cardinality results
  are counts, not measurements, so they are not pushed through the column's float
  formatting.
* `min` (0.85) and `max` (5.12) are real values lifted from the data, so they naturally
  carry the source precision.
* The `Sum` column is one character wider than its neighbours because `13.13` is wider than
  any body value — another instance of the summary row driving column width.

---

## 2-I — Counting blanks and non-blanks

<!-- screenshot:2-I -->

**What it demonstrates.** `blanks` and `nonblanks`, the two summaries that count what the
reader *sees* rather than what the data contains.

**Layout** — `tests/scenarios/suite_02/test_2_I_layout.json`

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "Test Int",
      "key": "test_int",
      "datatype": "int",
      "justification": "right",
      "summary": "blanks"
    },
    {
      "header": "Test Float",
      "key": "test_float",
      "datatype": "float",
      "justification": "right",
      "summary": "nonblanks"
    },
    {
      "header": "Test String",
      "key": "test_string",
      "datatype": "text",
      "justification": "left",
      "summary": "blanks"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "left",
      "summary": "nonblanks"
    }
  ]
}
```

**Output**

```text
╭──────────┬────────────┬─────────────┬──────────╮
│ Test Int │ Test Float │ Test String │ Status   │
├──────────┼────────────┼─────────────┼──────────┤
│      100 │       1.23 │ hello       │ Running  │
│          │            │             │ Running  │
│          │            │             │ Starting │
│      200 │       4.56 │ world       │ Running  │
│          │            │             │ Idle     │
├──────────┼────────────┼─────────────┼──────────┤
│        3 │          2 │ 3           │ 5        │
╰──────────┴────────────┴─────────────┴──────────╯
```

**What to look for**

* `Test Int` holds `100, 0, null, 200, 0` and reports **3 blanks**. Both the nulls and the
  zeros render as blank under the default `null_value` and `zero_value`, and `blanks`
  counts rendered blanks — so a displayed zero would not have been counted.
* `Test Float` holds `1.23, 0.0, null, 4.56, 0.00` and reports **2 non-blanks**, the exact
  complement of the same rule.
* `Test String` holds `"hello", "", null, "world", ""` and reports **3 blanks**: an empty
  string and a null are indistinguishable once rendered.
* `Status` has a value in every row and reports **5 non-blanks**.
* Because these summaries operate on the rendered result, changing `null_value` to `"0"`
  on a column will change its `blanks` count. That coupling is deliberate: these two
  summaries answer "how much of this column is actually populated on screen?"

---

## 2-J — Annotated rows are excluded from summaries

<!-- screenshot:2-J -->

**What it demonstrates.** `"annotate": true`, which lets a row appear in the body while
being ignored by every aggregation — combined with a hidden `break` column to set the
annotation apart visually.

**Data** — `tests/scenarios/suite_02/test_2_J_data.json`

```json
[
  {
    "group": "data",
    "id": 1,
    "name": "web-server-01",
    "cpu_cores": 4
  },
  {
    "group": "data",
    "id": 2,
    "name": "db-server-01",
    "cpu_cores": 8
  },
  {
    "group": "data",
    "id": 3,
    "name": "cache-server",
    "cpu_cores": 2
  },
  {
    "group": "data",
    "id": 4,
    "name": "api-gateway",
    "cpu_cores": 6
  },
  {
    "group": "data",
    "id": 5,
    "name": "backup-server",
    "cpu_cores": 4
  },
  {
    "group": "note",
    "annotate": true,
    "id": 999,
    "name": "Lines of Code",
    "cpu_cores": 1000
  }
]
```

**Layout** — `tests/scenarios/suite_02/test_2_J_layout.json`

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Server Name",
      "key": "name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "group",
      "key": "group",
      "datatype": "text",
      "justification": "left",
      "visible": false,
      "break": true
    }
  ]
}
```

**Output**

```text
╭─────┬───────────────┬───────────╮
│  ID │ Server Name   │ CPU Cores │
├─────┼───────────────┼───────────┤
│   1 │ web-server-01 │         4 │
│   2 │ db-server-01  │         8 │
│   3 │ cache-server  │         2 │
│   4 │ api-gateway   │         6 │
│   5 │ backup-server │         4 │
├─────┼───────────────┼───────────┤
│ 999 │ Lines of Code │     1,000 │
├─────┼───────────────┼───────────┤
│  15 │ 5             │        24 │
╰─────┴───────────────┴───────────╯
```

**What to look for**

* Six rows are rendered, but the totals describe only five. `ID` sums to **15**, not 1014;
  `CPU Cores` sums to **24**, not 1024; `Server Name` counts **5**, not 6. The
  `"annotate": true` row is displayed and then skipped by every aggregation.
* The annotated row *does* affect layout. `ID` is three characters wide to fit `999`, and
  `CPU Cores` is wide enough for `1,000`. Annotated rows participate in width measurement
  and in `break` evaluation; they are excluded from arithmetic only.
* The `group` column is `"visible": false` with `"break": true`. It never appears, but
  because its value changes from `data` to `note` on the last row, a separator is drawn
  above the annotation. This is the idiomatic way to insert a rule at an arbitrary point:
  add a hidden column whose value changes exactly where you want the line.
* The result is a table with two visually distinct trailing rows — an informational line
  and a genuine total — which is exactly how the project's own performance table reports
  lines of code alongside timings.

---

## Takeaways

* Any `summary` anywhere in the layout adds the separator and the summary row; there is no
  separate switch.
* Summary values are formatted by their own column, so `int` rounds, `float` keeps
  decimals, `num` separates thousands and `kcpu`/`kmem` keep their suffixes.
* `count` counts rows, `unique` counts distinct values, `blanks`/`nonblanks` count what is
  visible after `null_value` and `zero_value` have been applied.
* Summary values are measured when computing column widths.
* Use `"annotate": true` for rows that belong in the body but not in the totals, and pair
  it with a hidden `break` column when you want a rule drawn around them.

---

[← Basic Rendering](EXAMPLES_01.md) · [Examples index](EXAMPLES.md) · [Next: Clipping and Wrapping →](EXAMPLES_03.md)
