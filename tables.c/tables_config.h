/*
 * tables_config.h - Header file for configuration parsing in the tables utility
 * Defines structures and function prototypes for handling layout JSON configuration.
 */

#ifndef TABLES_CONFIG_H
#define TABLES_CONFIG_H

#include <jansson.h>

/* Constants */
#define DEFAULT_PADDING 1
#define MAX_COLUMNS 100
#define MAX_THEME_NAME 20

/* Enumeration for justification types */
typedef enum {
    JUSTIFY_LEFT,
    JUSTIFY_RIGHT,
    JUSTIFY_CENTER
} Justification;

/* Enumeration for data types */
typedef enum {
    DATA_TEXT,
    DATA_INT,
    DATA_NUM,
    DATA_FLOAT,
    DATA_KCPU,
    DATA_KMEM
} DataType;

/* Enumeration for null/zero value display options */
typedef enum {
    VALUE_BLANK,
    VALUE_ZERO,
    VALUE_MISSING
} ValueDisplay;

/* Enumeration for summary types */
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

/* Enumeration for wrap modes */
typedef enum {
    WRAP_CLIP,
    WRAP_WRAP
} WrapMode;

/* Enumeration for position types (for title/footer) */
typedef enum {
    POSITION_NONE,
    POSITION_LEFT,
    POSITION_RIGHT,
    POSITION_CENTER,
    POSITION_FULL
} Position;

/* Structure for column configuration */
typedef struct {
    char *header;           /* Column header text */
    char *key;              /* JSON field name */
    Justification justify;  /* Text alignment */
    DataType data_type;     /* Data type for validation/formatting */
    ValueDisplay null_val;  /* Display option for null values */
    ValueDisplay zero_val;  /* Display option for zero values */
    char *format;           /* Custom format string */
    SummaryType summary;    /* Summary calculation type */
    int break_on_change;    /* Insert separator on value change */
    int string_limit;       /* Maximum string length */
    WrapMode wrap_mode;     /* Text wrapping behavior */
    char *wrap_char;        /* Character for wrapping */
    int padding;            /* Padding spaces on each side */
    int width;              /* Fixed column width (0 for auto) */
    int width_specified;    /* Flag if width is explicitly set */
    int visible;            /* Flag if column is visible */
} ColumnConfig;

/* Structure for sort configuration */
typedef struct {
    char *key;              /* Column key to sort by */
    int direction;          /* 0 for ascending, 1 for descending */
    int priority;           /* Sort priority (lower number = higher priority) */
} SortConfig;

/* Structure for theme configuration */
typedef struct {
    char *border_color;     /* ANSI color for borders */
    char *caption_color;    /* ANSI color for column headers */
    char *header_color;     /* ANSI color for table title */
    char *footer_color;     /* ANSI color for table footer */
    char *summary_color;    /* ANSI color for summary row */
    char *text_color;       /* ANSI color for regular data */
    char *tl_corner;        /* Top-left corner character */
    char *tr_corner;        /* Top-right corner character */
    char *bl_corner;        /* Bottom-left corner character */
    char *br_corner;        /* Bottom-right corner character */
    char *h_line;           /* Horizontal line character */
    char *v_line;           /* Vertical line character */
    char *t_junct;          /* Top junction character */
    char *b_junct;          /* Bottom junction character */
    char *l_junct;          /* Left junction character */
    char *r_junct;          /* Right junction character */
    char *cross;            /* Cross junction character */
} ThemeConfig;

/* Structure for overall table configuration */
typedef struct {
    char *theme_name;       /* Name of the theme to use */
    char *title;            /* Table title text */
    Position title_pos;     /* Title position */
    char *footer;           /* Table footer text */
    Position footer_pos;    /* Footer position */
    ColumnConfig *columns;  /* Array of column configurations */
    int column_count;       /* Number of columns */
    SortConfig *sorts;      /* Array of sort configurations */
    int sort_count;         /* Number of sort rules */
    ThemeConfig theme;      /* Active theme settings */
} TableConfig;

/* Function prototypes */
int parse_layout_file(const char *filename, TableConfig *config);
void free_table_config(TableConfig *config);
int validate_input_files(const char *layout_file, const char *data_file);

#endif /* TABLES_CONFIG_H */
