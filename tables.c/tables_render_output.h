/*
 * tables_render_output.h - Header file for table rendering output functions
 *
 * Declares the main render_table() function that orchestrates the
 * complete table rendering pipeline.
 */

#ifndef TABLES_RENDER_OUTPUT_H
#define TABLES_RENDER_OUTPUT_H

#include "tables_config.h"
#include "tables_data.h"

/*
 * Render the table to the terminal
 * This is the main entry point for table rendering. It coordinates:
 *   - Column width calculation
 *   - Title rendering (if present)
 *   - Top border rendering
 *   - Header and separator rendering
 *   - Data row rendering with wrapping/clipping
 *   - Summary row rendering (if configured)
 *   - Bottom border rendering
 *   - Footer rendering (if present)
 *
 * Parameters:
 *   config: Table configuration (columns, theme, title, footer, etc.)
 *   data:   Table data (rows, summaries, widths)
 */
void render_table(TableConfig *config, TableData *data);

#endif /* TABLES_RENDER_OUTPUT_H */
