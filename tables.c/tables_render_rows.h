/*
 * tables_render_rows.h - Header file for row rendering functions used in table rendering
 *
 * Declares the render_rows() function for outputting data rows with
 * multi-line support, text wrapping/truncation, and break-on-change separators.
 */

#ifndef TABLES_RENDER_ROWS_H
#define TABLES_RENDER_ROWS_H

#include "tables_config.h"
#include "tables_data.h"

/*
 * Render the data rows of the table with support for wrapping, truncation, and breaking
 * Each cell is formatted according to its data type, then clipped or wrapped
 * based on column configuration. Rows are rendered line by line to support
 * multi-line cells. Break-on-change separators are inserted when designated
 * column values change between consecutive rows.
 */
void render_rows(TableConfig *config, TableData *data);

#endif /* TABLES_RENDER_ROWS_H */
