/*
 * tables_render_layout.h - Header file for layout calculation functions used in table rendering
 *
 * Declares functions for calculating column widths and total table width
 * based on content and configuration settings.
 */

#ifndef TABLES_RENDER_LAYOUT_H
#define TABLES_RENDER_LAYOUT_H

#include "tables_config.h"
#include "tables_data.h"

/*
 * Calculate column widths based on content and configuration
 * For each column, determines the minimum width needed to display
 * the header, all data values, and any summary values.
 * Columns with explicitly specified width are skipped.
 */
void calculate_column_widths(TableConfig *config, TableData *data);

/*
 * Calculate the total width of the table
 * Total width includes all visible column widths, inter-column separators,
 * and left/right borders.
 */
int calculate_total_width(TableConfig *config);

#endif /* TABLES_RENDER_LAYOUT_H */
