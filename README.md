# Terminal Tables

This is a tool (with both C and Bash versions) for drawing nice ANSI tables in the terminal. By passing in JSON for the layout, and separately JSON for the data, a table is generated that can include titles, footers, column headers, break lines, summary calculations and more.

<img width="500" alt="example table" src="https://github.com/user-attachments/assets/8aab5c1b-2784-4288-8112-a58c5ee501f8" />  

 *Example demonstrating many of Terminal Tables' core features*

## Additional Notes
While this project is currently under active development, feel free to give it a try and post any issues you encounter.  Or start a discussion if you would like to help steer the project in a particular direction.  Early days yet, so a good time to have your voice heard.  As the project unfolds, additional resources will be made available, including platform binaries, more documentation, demos, and so on.

## Examples
Looking for copy-pasteable layouts and rendered output? See [EXAMPLES.md](EXAMPLES.md), the entry point to a catalogue of worked examples drawn from the test-suite scenarios. It links to one page per test suite ([EXAMPLES_01.md](EXAMPLES_01.md) through [EXAMPLES_09.md](EXAMPLES_09.md)), each pairing the input `layout.json` and `data.json` with the actual table output. Every example is captured with `--mono` so the geometry is easy to read; drop `--mono` to see the colour theme.

## Repository Information 
[![Count Lines of Code](https://github.com/500Foods/Terminal-Tables/actions/workflows/main.yml/badge.svg)](https://github.com/500Foods/Terminal-Tables/actions/workflows/main.yml)
<!--CLOC-START -->
```cloc
Last updated at 2026-08-10 21:26:24 UTC
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
JSON                           109              0              0           4709
C                               14            356           1391           3290
Markdown                         7            668              2           2244
Bourne Shell                     3            172            193           1953
C/C++ Header                    13             81            351            240
YAML                             2              4              0             66
make                             1              6              8             32
-------------------------------------------------------------------------------
SUM:                           149           1287           1945          12534
-------------------------------------------------------------------------------
87 Files were skipped (duplicate, binary, or without source code):
  json: 83
  bats: 1
  gitattributes: 1
  gitignore: 1
  tables: 1
```
<!--CLOC-END-->

## Sponsor / Donate / Support
If you find this work interesting, helpful, or valuable, or that it has saved you time, money, or both, please consider directly supporting these efforts financially via [GitHub Sponsors](https://github.com/sponsors/500Foods) or donating via [Buy Me a Pizza](https://www.buymeacoffee.com/andrewsimard500). Also, check out these other [GitHub Repositories](https://github.com/500Foods?tab=repositories&q=&sort=stargazers) that may interest you.
