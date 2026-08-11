# Examples 08 — Footer Geometry

[← Footers](EXAMPLES_07.md) · [Examples index](EXAMPLES.md) · [Next: Showcase →](EXAMPLES_09.md)

Suite 08 is the largest of the geometry suites. It works through the same position ×
width matrix as [suite 06](EXAMPLES_06.md), adds the two `full` cases that suite 06 leaves
to suite 05, and then moves on to the features that make footers genuinely useful in
production: shell command substitution and Unicode-aware width measurement.

**Source:** [`tests/scenarios/suite_08/`](tests/scenarios/suite_08) · **Examples:** 17 ·
**Run the suite:** `bash tests/run_tests.sh 08`

---

## The data

Examples 8-A through 8-N share a minimal four-row dataset.

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

Examples 8-O, 8-P and 8-Q use wider datasets with `memory_gb`, `status` and `location`
fields; those are shown with their examples.

## The matrix

Unlike suite 06, this suite does not pin any column widths. The three width relationships
are produced by varying the *footer text* against a table that is consistently 45
characters wide, which is the more common real-world situation: your columns are what they
are, and the caption has to cope.

| | Footer narrower than table | Footer exactly table width | Footer wider than table |
|--|--|--|--|
| **position omitted** | [8-A](#8-a--short-footer-no-position) | [8-B](#8-b--footer-exactly-as-wide-as-the-table) | [8-C](#8-c--an-over-wide-footer-with-no-position) |
| **`left`** | [8-D](#8-d--short-footer-left) | [8-E](#8-e--equal-width-footer-left) | [8-F](#8-f--over-wide-footer-left) |
| **`center`** | [8-G](#8-g--short-footer-center) | [8-H](#8-h--equal-width-footer-center) | [8-I](#8-i--over-wide-footer-center) |
| **`right`** | [8-J](#8-j--short-footer-right) | [8-K](#8-k--equal-width-footer-right) | [8-L](#8-l--over-wide-footer-right) |
| **`full`** | [8-M](#8-m--full-width-footer-with-a-colour-placeholder) | — | [8-N](#8-n--full-width-footer-that-overflows) |

**Shared column set** for 8-A through 8-N (8-C substitutes longer header text, which is how
it produces a wider table):

```json
"columns": [
  { "header": "ID",          "key": "id",          "datatype": "int",   "justification": "right" },
  { "header": "Server Name", "key": "server_name", "datatype": "text",  "justification": "left"  },
  { "header": "CPU Cores",   "key": "cpu_cores",   "datatype": "num",   "justification": "right" },
  { "header": "Load Avg",    "key": "load_avg",    "datatype": "float", "justification": "right" }
]
```

Each example below lists only the fields that change.

---

## 8-A — Short footer, no position
**Layout delta** — `tests/scenarios/suite_08/test_8_A_layout.json`

```json
"footer": "Summary Report"
```

**Output**

![8-Q output](images/8-Q.svg)

**What to look for**

* The 😊 emoji counts as **two** display columns, not one and not the four bytes of its
  UTF-8 encoding. The title box is correspondingly two characters wider than 8-P's and the
  right wall still lands exactly on the border.
* The ✓ dingbats around the footer count as **one** column each. Getting emoji and dingbats
  right requires per-codepoint width classification rather than a blanket rule for
  non-ASCII characters.
* The `──` separators are box-drawing characters, single-width, and unaffected.
* If a future implementation measured by bytes, this table would be the first to break: the
  boxes would be drawn too wide and every junction below them would drift.

---

## Takeaways

* Footers follow the title matrix: omitted overhangs, named positions clip, equal widths
  render identically whatever the position.
* An over-wide footer clips head-first for `left` and `full`, tail-first for `right`, and
  genuinely centred for `center`.
* `full` centres short text in a table-width box and is the robust choice for generated
  content.
* `$(...)` in a title or footer is executed by the renderer before measurement. Layout files
  are executable content — treat them accordingly.
* Widths are measured in display columns, so emoji occupy two and dingbats one.

---

[← Footers](EXAMPLES_07.md) · [Examples index](EXAMPLES.md) · [Next: Showcase →](EXAMPLES_09.md)
