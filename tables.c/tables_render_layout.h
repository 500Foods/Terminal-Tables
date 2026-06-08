/*
 * tables_render_layout.h - Header file for layout calculation functions used in table rendering
 */

#ifndef TABLES_RENDER_LAYOUT_H
#define TABLES_RENDER_LAYOUT_H

#include "tables_config.h"
#include "tables_data.h"

/*
 * Calculate column widths based on content and configuration
 */
void calculate_column_widths(TableConfig *config, TableData *data);

/*
 * Calculate the total width of the table
 */
int calculate_total_width(TableConfig *config);

#endif /* TABLES_RENDER_LAYOUT_H */
