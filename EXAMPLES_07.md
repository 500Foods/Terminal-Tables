# Examples 07 — Footers

[← Title Geometry](EXAMPLES_06.md) · [Examples index](EXAMPLES.md) · [Next: Footer Geometry →](EXAMPLES_08.md)

Footers are titles turned upside down. Suite 07 mirrors [suite 05](EXAMPLES_05.md) exactly
— the same five layouts, the same data, with `title`/`title_position` replaced by
`footer`/`footer_position` — so the two pages can be read side by side to confirm that the
behaviour really is symmetrical.

**Source:** [`tests/scenarios/suite_07/`](tests/scenarios/suite_07) · **Examples:** 5 ·
**Run the suite:** `bash tests/run_tests.sh 07`

---

## The data

Identical to suite 05.

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

## How a footer is sized and placed

The rules match titles closely:

* Box width is the footer text plus one space of padding on each side plus two border
  characters.
* `footer_position` accepts `left`, `center`, `right`, `full`, or may be omitted.
* Omitting the position lets an over-wide footer extend past the table; naming a position
  clips it to the table width.
* `full` sizes the box to the table and centres short text inside it.

There is one behavioural difference worth flagging. An over-wide *title* is always clipped
from the head regardless of position, whereas an over-wide *footer* keeps the head for
`left` and `full`, the **middle** for `center`, and the **tail** for `right`. Suite 08
demonstrates all three: [8-F](EXAMPLES_08.md#8-f--over-wide-footer-left),
[8-I](EXAMPLES_08.md#8-i--over-wide-footer-center) and
[8-L](EXAMPLES_08.md#8-l--over-wide-footer-right).

The other visible difference is which characters appear at the join. A title box sits above
the table and pushes `┴` junctions up into the top border; a footer box sits below and
pushes `┬` junctions down into the bottom border.

---

## 7-A — A centred footer under a summary row
**What it demonstrates.** The everyday case: a short attribution line centred beneath a
table that already ends in a summary.

**Layout** — `tests/scenarios/suite_07/test_7_A_layout.json`

```json
{
  "theme": "Red",
  "footer": "Server Overview Report",
  "footer_position": "center",
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

![7-E output](images/7-E.svg)

**What to look for**

* `full` makes the box exactly as wide as the 82-character table, so the join is a single
  unbroken rule with no junction characters at all — the cleanest way to attach a footer.
* The 102-character footer is clipped to the 78 characters the box can hold, ending at
  `… and Performance Metri`. There is no ellipsis; check long footers against your
  narrowest table.
* This is [5-E](EXAMPLES_05.md#5-e--a-full-width-title-that-overflows) inverted, right down
  to the clipped fragment being identical.

---

## Takeaways

* Footers use the same sizing and positioning rules as titles; the clipping rule differs
  only for `center`, which keeps the middle of an over-wide footer.
* The join characters differ — `┬` descending into the bottom border where titles use `┴`
  ascending into the top one — but the geometry is computed the same way.
* Omit `footer_position` to let a long footer overhang; name a position to clip it.
* `full` gives an unbroken join and is the safest choice for generated text of unpredictable
  length.
* A table may carry both a title and a footer; suites [08](EXAMPLES_08.md) and
  [09](EXAMPLES_09.md) exercise that combination extensively.

---

[← Title Geometry](EXAMPLES_06.md) · [Examples index](EXAMPLES.md) · [Next: Footer Geometry →](EXAMPLES_08.md)
