/*
 * tables.c - Main entry point for the tables utility in C
 * This program converts JSON data into ANSI-formatted tables for terminal output.
 *
 * Usage: tables <layout_json_file> <data_json_file> [OPTIONS]
 * Options:
 *   --mono: Disable all ANSI colors (theme and {COLOR} placeholders)
 *   --debug: Enable debug output to stderr for memory issues
 *   --debug_layout: Enable debug output for layout issues
 *   --version: Display version information
 *   --help, -h: Show this help message
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <locale.h>
#include "tables_config.h"
#include "tables_themes.h"
#include "tables_data.h"
#include "tables_render.h"

/* Version string for the tables utility */
#define VERSION "1.0.1"

/* Function prototypes */
void print_help(void);
void print_version(void);

/* Global flag: enables debug output for memory allocation tracking */
int debug_mode = 0;
/* Global flag: enables debug output for layout dimension calculations */
int debug_layout = 0;
/* Global flag: when set, all ANSI color codes are disabled (monochrome output) */
int mono_mode = 0;

/*
 * Main function
 * Handles command-line arguments and coordinates the execution flow:
 *   1. Process help/version flags
 *   2. Validate input file paths
 *  3. Parse the layout JSON configuration
 *   4. Apply the selected theme
 *   5. Load and prepare data from JSON
 *   6. Sort data if sorting is configured
 *   7. Process data rows (summaries, widths)
 *   8. Render the final table to stdout
 *   9. Clean up all allocated memory
 */
int main(int argc, char *argv[]) {
    /* Set locale for proper UTF-8/wide character handling in terminal output */
    setlocale(LC_ALL, "");

    /* Check for help and version flags first, before validating argument count */
    if (argc >= 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_help();
            return 0;
        }

        if (strcmp(argv[1], "--version") == 0) {
            print_version();
            return 0;
        }
    }

    /* Require both layout and data file arguments */
    if (argc < 3) {
        fprintf(stderr, "Error: Both layout and data JSON files are required\n");
        print_help();
        return 1;
    }

    /* First positional argument: path to layout JSON file */
    const char *layout_file = argv[1];
    /* Second positional argument: path to data JSON file */
    const char *data_file = argv[2];

    /* Parse optional flags starting from argv[3] */
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug_mode = 1;
            fprintf(stderr, "Debug mode enabled\n");
        }
        if (strcmp(argv[i], "--debug_layout") == 0) {
            debug_layout = 1;
            fprintf(stderr, "Debug layout mode enabled\n");
        }
        if (strcmp(argv[i], "--mono") == 0) {
            mono_mode = 1;
        }
    }

    /* Validate that input files exist and can be opened */
    if (validate_input_files(layout_file, data_file) != 0) {
        fprintf(stderr, "Error: Input file validation failed\n");
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Input files validated successfully\n");
    }

    /* Parse layout file into TableConfig structure */
    TableConfig config;
    if (parse_layout_file(layout_file, &config) != 0) {
        fprintf(stderr, "Error: Failed to parse layout file %s\n", layout_file);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Layout file parsed successfully, %d columns\n", config.column_count);
    }

    /* Apply the theme specified in the layout configuration */
    get_theme(&config);

    /* Load data from JSON file and populate TableData structure */
    TableData table_data;
    if (prepare_data(data_file, &config, &table_data) != 0) {
        fprintf(stderr, "Error: Failed to load data from %s\n", data_file);
        free_table_config(&config);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Data loaded successfully, %d rows\n", table_data.row_count);
    }

    /* Sort data rows if sort configuration is present */
    sort_data(&config, &table_data);

    /* Process data rows: update summary stats and calculate column widths */
    process_data_rows(&config, &table_data);

    /* Render the final table to stdout */
    render_table(&config, &table_data);
    if (debug_mode) {
        fprintf(stderr, "Debug: Table rendering completed\n");
    }

    /* Free data structures allocated during processing */
    free_table_data(&table_data, config.column_count);
    if (debug_mode) {
        fprintf(stderr, "Debug: Table data freed\n");
    }

    /* Free configuration structure */
    free_table_config(&config);
    if (debug_mode) {
        fprintf(stderr, "Debug: Table configuration freed\n");
    }

    return 0;
}

/*
 * Print help message
 * Outputs usage information to stdout
 */
void print_help(void) {
    /* Usage line showing required positional arguments */
    printf("Usage: tables <layout_json_file> <data_json_file> [OPTIONS]\n");
    printf("Parameters:\n");
    /* Layout file: defines table structure (columns, themes, title, etc.) */
    printf("  layout_json_file: JSON file defining table structure and formatting\n");
    /* Data file: array of JSON objects, one per row */
    printf("  data_json_file: JSON file containing the data to display\n");
    printf("Options:\n");
    /* --mono disables all color output for use in monochrome environments */
    printf("  --mono: Disable all ANSI colors (theme and {COLOR} placeholders)\n");
    /* --debug enables stderr output for memory debugging */
    printf("  --debug: Enable debug output to stderr for memory issues\n");
    /* --debug_layout enables stderr output for layout debugging */
    printf("  --debug_layout: Enable debug output for layout issues\n");
    printf("  --version: Display version information\n");
    printf("  --help, -h: Show this help message\n");
}

/*
 * Print version information
 * Outputs the current version string to stdout
 */
void print_version(void) {
    printf("tables version %s\n", VERSION);
}
