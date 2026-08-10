/*
 * tables_config.h - Header file for configuration parsing in the tables utility
 * Defines structures and function prototypes for handling layout JSON configuration.
 *
 * This header defines:
 *   - Enumerations for justification, data types, value display, summary types,
 *     wrap modes, and positioning
 *   - ColumnConfig: per-column configuration (header, key, width, alignment, etc.)
 *   - SortConfig: sort rule configuration (key, direction, priority)
 *   - ThemeConfig: visual theme settings (colors and border characters)
 *   - TableConfig: overall table configuration (title, footer, columns, sorts, theme)
 *   - Function prototypes for layout parsing and memory management
 */

#ifndef TABLES_CONFIG_H
#define TABLES_CONFIG_H

#include <jansson.h>

/* Constants
 * DEFAULT_PADDING: Default number of spaces on each side of cell content
 * MAX_COLUMNS: Maximum number of columns supported
 * MAX_THEME_NAME: Maximum length of theme name strings
 */
#define DEFAULT_PADDING 1
#define MAX_COLUMNS 100
#define MAX_THEME_NAME 20

/* Enumeration for justification types
 * Controls horizontal text alignment within a cell:
 *   JUSTIFY_LEFT: Text aligned to the left edge
 *   JUSTIFY_RIGHT: Text aligned to the right edge
 *   JUSTIFY_CENTER: Text centered within the cell
 */
typedef enum {
    JUSTIFY_LEFT,
    JUSTIFY_RIGHT,
    JUSTIFY_CENTER
} Justification;

/* Enumeration for data types
 * Controls how values are validated and formatted:
 *   DATA_TEXT:  Plain text (no validation, no special formatting)
 *   DATA_INT:   Integer numbers (comma formatting, sum/min/max/avg)
 *   DATA_NUM:   Plain numbers (comma formatting, sum/min/max/avg)
 *   DATA_FLOAT: Floating point (precision-aware, sum/min/max/avg)
 *   DATA_KCPU:  Kubernetes CPU (millicores like 100m, sum/min/max/avg)
 *   DATA_KMEM:  Kubernetes memory (units like Mi, Gi, Ki, M, G, K)
 */
typedef enum {
    DATA_TEXT,
    DATA_INT,
    DATA_NUM,
    DATA_FLOAT,
    DATA_KCPU,
    DATA_KMEM
} DataType;

/* Enumeration for null/zero value display options
 * Controls how null and zero values are rendered:
 *   VALUE_BLANK: Display empty string (default)
 *   VALUE_ZERO:  Display "0"
 *   VALUE_MISSING: Display "Missing"
 */
typedef enum {
    VALUE_BLANK,
    VALUE_ZERO,
    VALUE_MISSING
} ValueDisplay;

/* Enumeration for summary types
 * Controls what aggregate statistic to calculate and display for a column:
 *   SUMMARY_NONE:      No summary for this column
 *   SUMMARY_SUM:       Sum of all values
 *   SUMMARY_MIN:       Minimum value
 *   SUMMARY_MAX:       Maximum value
 *   SUMMARY_AVG:       Average of all values
 *   SUMMARY_COUNT:     Count of non-null values
 *   SUMMARY_UNIQUE:    Count of unique values
 *   SUMMARY_BLANKS:    Count of blank/zero values
 *   SUMMARY_NONBLANKS: Count of non-blank/non-zero values
 */
typedef enum {
    SUMMARY_NONE,
    SUMMARY_SUM,
    SUMMARY_MIN,
    SUMMARY_MAX,
    SUMMARY_AVG,
    SUMMARY_COUNT,
    SUMMARY_UNIQUE,
    SUMMARY_BLANKS,
    SUMMARY_NONBLANKS
} SummaryType;

/* Enumeration for wrap modes
 * Controls how text is handled when it exceeds column width:
 *   WRAP_CLIP: Truncate text that exceeds the width
 *   WRAP_WRAP: Wrap text to multiple lines within the column
 */
typedef enum {
    WRAP_CLIP,
    WRAP_WRAP
} WrapMode;

/* Enumeration for position types (for title/footer)
 * Controls positioning of title and footer boxes:
 *   POSITION_NONE:    No positioning (can extend beyond table width)
 *   POSITION_LEFT:    Left-aligned within table width
 *   POSITION_RIGHT:   Right-aligned within table width
 *   POSITION_CENTER:  Centered within table width
 *   POSITION_FULL:    Span full table width
 */
typedef enum {
    POSITION_NONE,
    POSITION_LEFT,
    POSITION_RIGHT,
    POSITION_CENTER,
    POSITION_FULL
} Position;

/* Structure for column configuration
 * Holds all settings for a single table column, parsed from layout JSON
 */
typedef struct {
    char *header;           /* Column header text displayed in the header row */
    char *key;              /* JSON field name used to extract values from data rows */
    Justification justify;  /* Text alignment within the column (left, right, center) */
    DataType data_type;     /* Data type for validation and formatting (text, int, float, etc.) */
    ValueDisplay null_val;  /* Display option for null values (blank, zero, or "Missing") */
    ValueDisplay zero_val;  /* Display option for zero values (blank, zero, or "Missing") */
    char *format;           /* Custom format string (e.g., "%.2f") for value display */
    SummaryType summary;    /* Summary calculation type (sum, min, max, avg, count, etc.) */
    int break_on_change;    /* If set, insert a separator line when this column's value changes */
    int string_limit;       /* Maximum string length before truncation/clipping */
    WrapMode wrap_mode;     /* Text wrapping behavior (clip or wrap) */
    char *wrap_char;        /* Character for delimiter-based wrapping (optional) */
    int padding;            /* Padding spaces on each side of cell content */
    int width;              /* Fixed column width (0 for auto-calculate from content) */
    int width_specified;    /* Flag if width is explicitly set in config (vs auto-calculated) */
    int visible;            /* Flag if column is visible in output (can be hidden) */
} ColumnConfig;

/* Structure for sort configuration
 * Defines a single sort rule for ordering data rows
 */
typedef struct {
    char *key;              /* Column key to sort by */
    int direction;          /* 0 for ascending, 1 for descending */
    int priority;           /* Sort priority (lower number = higher priority for multi-key sort) */
} SortConfig;

/* Structure for theme configuration
 * Holds all visual settings for table rendering: colors and border characters
 */
typedef struct {
    char *border_color;     /* ANSI color for borders, separators, and corners */
    char *caption_color;    /* ANSI color for column header text */
    char *header_color;     /* ANSI color for table title text */
    char *footer_color;     /* ANSI color for table footer text */
    char *summary_color;    /* ANSI color for summary row text */
    char *text_color;       /* ANSI color for regular data cell text (typically reset) */
    char *tl_corner;        /* Top-left corner character */
    char *tr_corner;        /* Top-right corner character */
    char *bl_corner;        /* Bottom-left corner character */
    char *br_corner;        /* Bottom-right corner character */
    char *h_line;           /* Horizontal line character */
    char *v_line;           /* Vertical line character */
    char *t_junct;          /* Top junction character (for column separators) */
    char *b_junct;          /* Bottom junction character (for column separators) */
    char *l_junct;          /* Left junction character (for column separators) */
    char *r_junct;          /* Right junction character (for column separators) */
    char *cross;            /* Cross junction character (for column separators) */
} ThemeConfig;

/* Structure for overall table configuration
 * The top-level configuration that ties together all table settings
 */
typedef struct {
    char *theme_name;       /* Name of the theme to use (e.g., "Red", "Blue") */
    char *title;            /* Table title text (optional, can include $(date) etc.) */
    Position title_pos;     /* Title position (left, center, right, full, none) */
    char *footer;           /* Table footer text (optional, can include $(date) etc.) */
    Position footer_pos;    /* Footer position (left, center, right, full, none) */
    ColumnConfig *columns;  /* Array of column configurations */
    int column_count;       /* Number of columns */
    SortConfig *sorts;      /* Array of sort configurations */
    int sort_count;         /* Number of sort rules */
    ThemeConfig theme;      /* Active theme settings (populated by get_theme) */
} TableConfig;

/* Function prototypes
 * Layout file parsing and validation */
int parse_layout_file(const char *filename, TableConfig *config);
void free_table_config(TableConfig *config);
int validate_input_files(const char *layout_file, const char *data_file);

#endif /* TABLES_CONFIG_H */
