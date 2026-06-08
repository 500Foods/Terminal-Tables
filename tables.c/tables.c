/*
 * tables.c - Main entry point for the tables utility in C
 * This program converts JSON data into ANSI-formatted tables for terminal output.
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

#define VERSION "1.0.1"

/* Function prototypes */
void print_help(void);
void print_version(void);

/*
 * Main function
 * Handles command-line arguments and coordinates the execution flow.
 */
int debug_mode = 0;
int debug_layout = 0;

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    
    // Check for help and version flags first, before validating argument count
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
    
    if (argc < 3) {
        fprintf(stderr, "Error: Both layout and data JSON files are required\n");
        print_help();
        return 1;
    }

    const char *layout_file = argv[1];
    const char *data_file = argv[2];

    // Check for debug options
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug_mode = 1;
            fprintf(stderr, "Debug mode enabled\n");
        }
        if (strcmp(argv[i], "--debug_layout") == 0) {
            debug_layout = 1;
            fprintf(stderr, "Debug layout mode enabled\n");
        }
    }

    // Validate input files
    if (validate_input_files(layout_file, data_file) != 0) {
        fprintf(stderr, "Error: Input file validation failed\n");
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Input files validated successfully\n");
    }

    // Parse layout file
    TableConfig config;
    if (parse_layout_file(layout_file, &config) != 0) {
        fprintf(stderr, "Error: Failed to parse layout file %s\n", layout_file);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Layout file parsed successfully, %d columns\n", config.column_count);
    }

    // Set the theme based on configuration
    get_theme(&config);

    // Load and prepare data
    TableData table_data;
    if (prepare_data(data_file, &config, &table_data) != 0) {
        fprintf(stderr, "Error: Failed to load data from %s\n", data_file);
        free_table_config(&config);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Data loaded successfully, %d rows\n", table_data.row_count);
    }

    // Sort data if specified
    sort_data(&config, &table_data);

    // Process data rows and calculate summaries
    process_data_rows(&config, &table_data);

    // Render table
    render_table(&config, &table_data);
    if (debug_mode) {
        fprintf(stderr, "Debug: Table rendering completed\n");
    }

    // Clean up data
    free_table_data(&table_data, config.column_count);
    if (debug_mode) {
        fprintf(stderr, "Debug: Table data freed\n");
    }

    // Clean up
    free_table_config(&config);
    if (debug_mode) {
        fprintf(stderr, "Debug: Table configuration freed\n");
    }

    return 0;
}

/*
 * Print help message
 */
void print_help(void) {
    printf("Usage: tables <layout_json_file> <data_json_file> [OPTIONS]\n");
    printf("Parameters:\n");
    printf("  layout_json_file: JSON file defining table structure and formatting\n");
    printf("  data_json_file: JSON file containing the data to display\n");
    printf("Options:\n");
    printf("  --debug: Enable debug output to stderr for memory issues\n");
    printf("  --debug_layout: Enable debug output for layout issues\n");
    printf("  --version: Display version information\n");
    printf("  --help, -h: Show this help message\n");
}

/*
 * Print version information
 */
void print_version(void) {
    printf("tables version %s\n", VERSION);
}
