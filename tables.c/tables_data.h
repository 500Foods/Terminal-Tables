/*
 * tables_data.h - Header file for data processing in the tables utility
 * Defines structures and function prototypes for loading, sorting, and summarizing data.
 */

#ifndef TABLES_DATA_H
#define TABLES_DATA_H

#include "tables_config.h"
#include "tables_datatypes.h"

/* Structure to hold a single data row */
typedef struct {
    char **values;          /* Array of string values for each column */
} DataRow;

/* Structure to hold summary statistics for a column */
typedef struct {
    double sum;             /* Sum of values */
    int count;              /* Count of non-null values */
    double min;             /* Minimum value */
    int min_initialized;    /* Flag to indicate if min has been initialized */
    double max;             /* Maximum value */
    int max_initialized;    /* Flag to indicate if max has been initialized */
    char **unique_values;   /* Array of unique values */
    int unique_count;       /* Number of unique values */
    double avg_sum;         /* Sum for calculating average */
    int avg_count;          /* Count for calculating average */
    int max_decimal_places; /* Maximum decimal places found in float data */
    int blanks;             /* Count of blank or zero values */
    int nonblanks;          /* Count of non-blank or non-zero values */
} SummaryStats;

/* Structure to hold table data */
typedef struct {
    DataRow *rows;          /* Array of data rows */
    int row_count;          /* Number of rows */
    SummaryStats *summaries;/* Array of summary stats for each column */
    int max_lines;          /* Maximum number of lines per row after wrapping */
} TableData;

/* Function prototypes */
int prepare_data(const char *data_file, TableConfig *config, TableData *data);
void sort_data(TableConfig *config, TableData *data);
void process_data_rows(TableConfig *config, TableData *data);
void initialize_summaries(TableConfig *config, TableData *data);
void update_summaries(int col_idx, const char *value, DataType data_type, SummaryType summary_type, SummaryStats *stats);
void free_table_data(TableData *data, int column_count);

#endif /* TABLES_DATA_H */
