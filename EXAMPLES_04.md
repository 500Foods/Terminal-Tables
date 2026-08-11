# Examples 04 — Combined Features

[← Clipping and Wrapping](EXAMPLES_03.md) · [Examples index](EXAMPLES.md) · [Next: Titles →](EXAMPLES_05.md)

The first three suites isolate one behaviour at a time. Suite 04 puts them back together:
five layouts that each combine breaks, summaries, wrapping and mixed datatypes the way a
real report does. These are the examples to copy from when you are building something for
production rather than learning a field.

**Source:** [`tests/scenarios/suite_04/`](tests/scenarios/suite_04) · **Examples:** 5 ·
**Run the suite:** `bash tests/run_tests.sh 04`

---

## The data

All five examples share a four-row dataset with long prose in `description`, a
comma-separated `tags` list, Kubernetes quantities, and a `category` suitable for grouping.

```json
[
  {
    "id": 1,
    "server_name": "web-server-01",
    "category": "Web",
    "cpu_cores": 4,
    "load_avg": 2.45,
    "cpu_usage": "1250m",
    "memory_usage": "2048Mi",
    "status": "Running",
    "description": "Primary web server for frontend applications with a very long description to test wrapping functionality. This description is extended to ensure that it will wrap across multiple lines when a width constraint is applied in the test configuration. Additional text is added here for thorough testing.",
    "tags": "frontend,app,ui,primary,loadbalancer,highavailability"
  },
  {
    "id": 2,
    "server_name": "db-server-01",
    "category": "Database",
    "cpu_cores": 8,
    "load_avg": 5.12,
    "cpu_usage": "3200m",
    "memory_usage": "8192Mi",
    "status": "Running",
    "description": "Main database server handling critical data storage and retrieval operations.",
    "tags": "db,sql,storage,primary,backend"
  },
  {
    "id": 3,
    "server_name": "cache-server",
    "category": "Cache",
    "cpu_cores": 2,
    "load_avg": 0.85,
    "cpu_usage": "500m",
    "memory_usage": "1024Mi",
    "status": "Starting",
    "description": "In-memory cache for speeding up data access in applications.",
    "tags": "cache,redis,fast,memory"
  },
  {
    "id": 4,
    "server_name": "api-gateway",
    "category": "Web",
    "cpu_cores": 6,
    "load_avg": 3.21,
    "cpu_usage": "2100m",
    "memory_usage": "4096Mi",
    "status": "Running",
    "description": "API gateway managing incoming requests and routing to appropriate services with detailed logging and monitoring capabilities enabled for performance tracking.",
    "tags": "api,gateway,routing,web,interface,management"
  }
]
```

Note the row order: `Web`, `Database`, `Cache`, `Web`. The category is **not** sorted, and
suite 04 does not sort it. That turns out to be instructive.

---

## 4-A — Breaks, summaries and a wrapped description
**What it demonstrates.** The canonical combination — group separators, five different
summaries, and a wide wrapped prose column — plus what `break` does to unsorted data.

**Layout** — `tests/scenarios/suite_04/test_4_A_layout.json`

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "Category",
      "key": "category",
      "datatype": "text",
      "justification": "center",
      "break": true,
      "summary": "unique"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "Description",
      "key": "description",
      "datatype": "text",
      "justification": "left",
      "width": 50,
      "wrap_mode": "wrap"
    }
  ]
}
```

**Output**

![4-E output](images/4-E.svg)

**What to look for**

* `Description` at `"width": 60` gives 58 characters of content, which is close to the
  comfortable maximum for prose in a terminal. Rows are two to three lines tall rather than
  the seven of 4-A.
* `Category` breaks the table into groups *and* has no summary, so its cell in the last row
  is blank while `Status` beside it reports `unique` 2. Mixing summarised and unsummarised
  columns in one row is normal.
* `ID` reports `count` 4 rather than a meaningless sum — the right choice for an identifier
  column, and worth contrasting with
  [2-A](EXAMPLES_02.md#2-a--sum-and-count), which sums one on purpose to make the
  arithmetic checkable.
* Every feature in suites 01 to 03 is present here except explicit clipping: datatypes,
  justifications, a fixed width, wrapping, breaks and summaries. If you read one layout in
  this catalogue as a template, read this one.

---

## Takeaways

* `break` compares adjacent rows only. Without a matching `sort`, unsorted input produces
  one group per change, which may be more groups than you expected.
* Separators and multi-line rows belong together; wrapped columns become much easier to
  read once a `break` column is added.
* Several columns may share a key, which is how a statistics strip or a min/max pair is
  built.
* Columns are rendered in layout order regardless of width or datatype.
* A blank summary cell is normal — every column is drawn in the summary row whether or not
  it aggregates anything.

---

[← Clipping and Wrapping](EXAMPLES_03.md) · [Examples index](EXAMPLES.md) · [Next: Titles →](EXAMPLES_05.md)
