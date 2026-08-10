/*
 * tables_render_summaries.h - Header file for summary rendering functions used in table rendering
 *
 * Declares the render_summaries() function for outputting an aggregate
 * statistics row below the data rows.
 */

#ifndef TABLES_RENDER_SUMMARIES_H
#define TABLES_RENDER_SUMMARIES_H

#include "tables_config.h"
#include "tables_data.h"

/*
 * Render the summaries row if any summaries are defined
 * Outputs a separator line followed by a row containing summary values
 * (sum, min, max, avg, count, unique, blanks, nonblanks) for each column
 * that has a summary type configured.
 */
void render_summaries(TableConfig *config, TableData *data);

#endif /* TABLES_RENDER_SUMMARIES_H */
