/*
 * tables_render.c - Main file for table rendering functionality
 * This file now serves as a placeholder or entry point for rendering functions,
 * which have been split into smaller, more manageable files.
 *
 * The rendering logic has been modularized into:
 *   - tables_render_output.c:  Main render_table() entry point and top/bottom borders
 *   - tables_render_title.c:   Title box rendering and top border integration
 *   - tables_render_headers.c: Column headers and header separator line
 *   - tables_render_rows.c:    Data rows with wrapping, clipping, and break-on-change
 *   - tables_render_summaries.c: Summary (sum/min/max/avg/count) row rendering
 *   - tables_render_footer.c:  Footer box rendering and bottom border integration
 *   - tables_render_layout.c:  Column width and total table width calculations
 *   - tables_render_utils.c:   String utilities (width, clipping, wrapping, colors)
 *
 * This file includes the main render header which re-exports render_table() to
 * maintain backward compatibility with the original API.
 */

#include "tables_render.h"

/* This file is intentionally minimal as the rendering logic has been refactored
 * into separate files for better organization and maintainability. */
