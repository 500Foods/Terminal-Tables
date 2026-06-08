/*
 * tables_render_output.h - Header file for table rendering output functions
 */

#ifndef TABLES_RENDER_OUTPUT_H
#define TABLES_RENDER_OUTPUT_H

#include "tables_config.h"
#include "tables_data.h"

/*
 * Render the table to the terminal
 */
void render_table(TableConfig *config, TableData *data);

#endif /* TABLES_RENDER_OUTPUT_H */
