/*
 * tables_render_rows.h - Header file for row rendering functions used in table rendering
 */

#ifndef TABLES_RENDER_ROWS_H
#define TABLES_RENDER_ROWS_H

#include "tables_config.h"
#include "tables_data.h"

/*
 * Render the data rows of the table with support for wrapping, truncation, and breaking
 */
void render_rows(TableConfig *config, TableData *data);

#endif /* TABLES_RENDER_ROWS_H */
