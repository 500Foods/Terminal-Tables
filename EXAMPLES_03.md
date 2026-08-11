# Examples 03 — Clipping and Wrapping

[← Summaries](EXAMPLES_02.md) · [Examples index](EXAMPLES.md) · [Next: Combined Features →](EXAMPLES_04.md)

Suite 03 is about the moment your content stops fitting. Terminal Tables offers two
strategies — clip the text, or wrap it onto extra lines — and each interacts with the
column's `justification` in a way that is worth internalising. This suite isolates every
combination: no constraint at all, clipping in three alignments, word wrapping, wrapping on
a delimiter in three alignments, and wrapping combined with group separators.

**Source:** [`tests/scenarios/suite_03/`](tests/scenarios/suite_03) · **Examples:** 11 ·
**Run the suite:** `bash tests/run_tests.sh 03`

---

## The data

All eleven examples read the same five rows. Row 2 carries a deliberately enormous
`description` (just over 300 characters) and an unusually long `tags` list; everything else
is short. That contrast is what makes the constraint visible.

```json
[
  {
    "id": 1,
    "category": "Web",
    "description": "Primary",
    "tags": "frontend,app,ui"
  },
  {
    "id": 2,
    "category": "Web",
    "description": "Secondary web server with a very long description to test wrapping functionality. This description is extended to be over 125 characters to ensure that it will wrap when a width of 75 is set in the test. Additional text is added here to meet the length requirement for thorough testing of wrapping behavior.",
    "tags": "frontend,backup,loadbalancer,highavailability,performance"
  },
  {
    "id": 3,
    "category": "Database",
    "description": "Main database server",
    "tags": "db,sql,storage"
  },
  {
    "id": 4,
    "category": "Database",
    "description": "Replica database",
    "tags": "db,replica,read"
  },
  {
    "id": 5,
    "category": "Cache",
    "description": "In-memory cache",
    "tags": "cache,redis,fast"
  }
]
```

The `tags` field is a comma-separated string rather than an array. That is intentional —
it is the shape you get out of most exports, and it is what `wrap_char` is designed to
handle.

## The three controls

| Field | Meaning |
|-------|---------|
| `width` | Total cell width **including** padding. A `width` of 15 with the default padding of 1 leaves 13 characters of content space. `0`, the default, means "measure the content". |
| `wrap_mode` | `clip` (default) truncates the value to fit; `wrap` splits it across as many lines as needed. |
| `wrap_char` | When wrapping, break at this character instead of at spaces. Typically `,`. |

`width` is the trigger for both behaviours. Without it there is nothing to overflow, so
`wrap_mode` and `wrap_char` have no effect.

---

## 3-A — No width means no limit
**What it demonstrates.** The baseline. With no `width`, a column grows to fit its widest
value, however absurd that is.

**Layout** — `tests/scenarios/suite_03/test_3_A_layout.json`

```json
{
  "theme": "Red",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Category",
      "key": "category",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "Description",
      "key": "description",
      "datatype": "text",
      "justification": "left"
    }
  ]
}
```

**Output**

![3-K output](images/3-K.svg)

**What to look for**

* `"width": 75` gives 73 characters of content, and the 300-character description folds
  neatly into five lines. The table is 93 columns wide overall — wide, but printable.
* Compare against 3-A, which rendered the same data at 327 columns. One field turned an
  unusable table into a readable one.
* Setting a `width` on your one prose column, and leaving every other column to
  auto-measure, is the most common real-world layout in this whole catalogue.

---

## Takeaways

* `width` includes padding. Subtract two (with the default padding of 1) to get usable
  content space.
* Without `width` there is no overflow, and `wrap_mode`/`wrap_char` do nothing.
* Clipping preserves the edge the text is justified against: head for `left`, tail for
  `right`, middle for `center`.
* Headers are clipped by the same rule as data.
* `wrap` splits on spaces by default, or on `wrap_char` when set; the delimiter is
  consumed.
* Wrapping runs before clipping — a single token longer than the column is wrapped onto its
  own line and then clipped.
* Continuation lines blank out the other columns, and row height is the tallest cell in the
  row.
* `break` rules are drawn between rows, never inside a multi-line row.

---

[← Summaries](EXAMPLES_02.md) · [Examples index](EXAMPLES.md) · [Next: Combined Features →](EXAMPLES_04.md)
