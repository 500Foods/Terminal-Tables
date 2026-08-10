/*
 * tables_render.h - Header file for table rendering functionality
 *
 * This header serves as the main entry point for table rendering.
 * It includes all rendering sub-headers and declares the primary
 * render_table() function that orchestrates the full rendering pipeline.
 *
 * The rendering system is modularized into:
 *   - tables_render_output.c:    Main orchestration and border rendering
 *   - tables_render_title.c:     Title box and top border
 *   - tables_render_headers.c:   Column headers and separator
 *   - tables_render_rows.c:      Data rows with wrapping and breaks
 *   - tables_render_summaries.c:  Summary statistics row
 *   - tables_render_footer.c:    Footer box and bottom border
 *   - tables_render_layout.c:    Width calculations
 *   - tables_render_utils.c:     String utilities
 */

#ifndef TABLES_RENDER_H
#define TABLES_RENDER_H

#include "tables_config.h"
#include "tables_data.h"
#include "tables_render_utils.h"
#include "tables_render_layout.h"
#include "tables_render_output.h"
#include "tables_render_title.h"
#include "tables_render_headers.h"
#include "tables_render_rows.h"
#include "tables_render_summaries.h"
#include "tables_render_footer.h"

/* Main rendering function: renders the complete table to stdout
 * Parameters:
 *   config: Table configuration (columns, theme, title, footer, etc.)
 *   data:   Table data (rows, summaries, widths)
 */
void render_table(TableConfig *config, TableData *data);

#endif /* TABLES_RENDER_H */
