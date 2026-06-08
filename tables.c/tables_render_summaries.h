/*
 * tables_render_summaries.h - Header file for summary rendering functions used in table rendering
 */

#ifndef TABLES_RENDER_SUMMARIES_H
#define TABLES_RENDER_SUMMARIES_H

#include "tables_config.h"
#include "tables_data.h"

/*
 * Render the summaries row if any summaries are defined
 */
void render_summaries(TableConfig *config, TableData *data);

#endif /* TABLES_RENDER_SUMMARIES_H */
