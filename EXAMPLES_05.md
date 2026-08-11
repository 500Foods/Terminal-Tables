# Examples 05 — Titles

[← Combined Features](EXAMPLES_04.md) · [Examples index](EXAMPLES.md) · [Next: Title Geometry →](EXAMPLES_06.md)

A title is not a line of text above the table — it is a second box welded onto the top
border, and the join is computed so that the two frames share edges cleanly. Suite 05
introduces titles in realistic layouts: centred, right-aligned, full-width, wider than the
table and narrower than it. [Suite 06](EXAMPLES_06.md) then works through the geometry
systematically.

**Source:** [`tests/scenarios/suite_05/`](tests/scenarios/suite_05) · **Examples:** 5 ·
**Run the suite:** `bash tests/run_tests.sh 05`

---

## The data

All five examples share the same four-row inventory.

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
    "description": "Primary web server for frontend applications with a detailed setup.",
    "tags": "frontend,app,ui,primary,loadbalancer"
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
    "description": "Main database server handling critical data operations.",
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
    "description": "In-memory cache for speeding up data access.",
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
    "description": "API gateway managing incoming requests and routing.",
    "tags": "api,gateway,routing,web,interface"
  }
]
```

## How a title is sized and placed

Two fields control everything:

* `title` — the text, which may contain `{COLOR}` placeholders and `$(command)`
  substitutions.
* `title_position` — one of `left`, `center`, `right`, `full`, or omitted.

The title box is the text plus one space of padding on each side plus two border
characters, so a 22-character title produces a 26-character box. What happens when that
box does not match the table width depends on the position:

| Position | Box narrower than table | Box wider than table |
|----------|-------------------------|----------------------|
| omitted | Drawn at the left, table width unchanged | **Box extends past the table**; the title is not clipped |
| `left` | Drawn at the left | Title is **clipped** to the table width |
| `center` | Centred over the table | Title is **clipped** to the table width |
| `right` | Drawn at the right | Title is **clipped** to the table width |
| `full` | Box spans the table width, text centred inside | Title is **clipped** to the table width |

An over-wide title is always clipped from the head — the first *n* characters survive —
whatever position is named. The position governs where a title that *fits* is placed; once
the text has been cut down to exactly the table width there is nothing left to align.
Footers behave differently in this one respect and are covered in
[suite 08](EXAMPLES_08.md#8-i--over-wide-footer-center).

Omitting `title_position` is therefore *not* the same as writing `"left"`. The former lets
a long title overhang the table; the latter cuts it off. Both are useful, and
[6-C versus 6-F](EXAMPLES_06.md#6-c--an-over-wide-title-with-no-position) shows the two
side by side.

---

## 5-A — A centred title narrower than the table
**What it demonstrates.** The common case: a short descriptive title centred over a table
that also carries a summary row.

**Layout** — `tests/scenarios/suite_05/test_5_A_layout.json`

```json
{
  "theme": "Red",
  "title": "Server Overview Report",
  "title_position": "center",
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
      "summary": "unique"
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

![5-E output](images/5-E.svg)

**What to look for**

* `full` makes the title box exactly as wide as the table, here 82 characters, so the top
  border is a single unbroken rule with no junctions at all. This is the tidiest title
  style and the one to reach for when you do not want to think about geometry.
* The title is 102 characters and the box holds 78, so it is clipped:
  `… and Performance Metri`. Nothing marks the truncation. When you use `full`, check your
  title against the narrowest table it will ever render.
* Compare with 5-B, where a long title was allowed to overhang because no position was
  given. Same problem, two different policies, chosen by one field.
* `Description` is right-justified and wrapped at width 25, so each of its lines is flush
  right — the wrapped-and-justified behaviour from
  [3-I](EXAMPLES_03.md#3-i--right-justified-delimiter-wrapping) applied to prose.
* `Load Avg` is centred while every other numeric column is right-justified, and the
  summary `max` of `5.12` is centred with it.

---

## Takeaways

* The title is a box, not a line; its width is the text plus four characters.
* Omitting `title_position` allows an over-wide title to overhang the table. Naming a
  position clips it instead.
* `full` sizes the box to the table and centres the text, giving a clean unbroken top
  border.
* Titles never change column widths. The table is measured first; the title is fitted to
  it afterwards.
* Titles compose freely with breaks, summaries and wrapped columns — there are no
  interactions to work around.

---

[← Combined Features](EXAMPLES_04.md) · [Examples index](EXAMPLES.md) · [Next: Title Geometry →](EXAMPLES_06.md)
