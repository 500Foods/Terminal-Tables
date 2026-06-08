#!/usr/bin/env bash
# tables.sh - Library for JSON to ANSI tables
declare -g COLUMN_COUNT=0 MAX_LINES=1 THEME_NAME="Red" DEFAULT_PADDING=1
declare -A DATATYPE_HANDLERS=([text_validate]="validate_text" [text_format]="format_text" [text_summary_types]="count unique blanks nonblanks" [int_validate]="validate_number" [int_format]="format_number" [int_summary_types]="sum min max avg count unique blanks nonblanks" [num_validate]="validate_number" [num_format]="format_num" [num_summary_types]="sum min max avg count unique blanks nonblanks" [float_validate]="validate_number" [float_format]="format_float" [float_summary_types]="sum min max avg count unique blanks nonblanks" [kcpu_validate]="validate_kcpu" [kcpu_format]="format_kcpu" [kcpu_summary_types]="sum min max avg count unique blanks nonblanks" [kmem_validate]="validate_kmem" [kmem_format]="format_kmem" [kmem_summary_types]="sum min max avg count unique blanks nonblanks")
declare -A THEME
# Only declare color variables if not already set (prevents readonly variable errors)
if [[ -z "${RED:-}" ]]; then
    declare -r RED=$'\033[0;31m' BLUE=$'\033[0;34m' GREEN=$'\033[0;32m' YELLOW=$'\033[0;33m' CYAN=$'\033[0;36m' MAGENTA=$'\033[0;35m' WHITE=$'\033[1;37m' BOLD=$'\033[1m' DIM=$'\033[2m' UNDERLINE=$'\033[4m' NC=$'\033[0m'
fi
replace_color_placeholders() { local text="$1"; text="${text//\{RED\}/$RED}"; text="${text//\{BLUE\}/$BLUE}"; text="${text//\{GREEN\}/$GREEN}"; text="${text//\{YELLOW\}/$YELLOW}"; text="${text//\{CYAN\}/$CYAN}"; text="${text//\{MAGENTA\}/$MAGENTA}"; text="${text//\{WHITE\}/$WHITE}"; text="${text//\{BOLD\}/$BOLD}"; text="${text//\{DIM\}/$DIM}"; text="${text//\{UNDERLINE\}/$UNDERLINE}"; text="${text//\{NC\}/$NC}"; text="${text//\{RESET\}/$NC}"; echo "$text"; }
validate_data() { local value="$1" type="$2"; case "$type" in text) [[ "$value" != "null" ]] && echo "$value" || echo "";; number|int|float|num) [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ || "$value" == "0" || "$value" == "null" ]] && echo "$value" || echo "";; kcpu) [[ "$value" =~ ^[0-9]+m$ || "$value" == "0" || "$value" == "0m" || "$value" == "null" || "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]] && echo "$value" || echo "$value";; kmem) [[ "$value" =~ ^[0-9]+[KMG]$ || "$value" =~ ^[0-9]+Mi$ || "$value" =~ ^[0-9]+Gi$ || "$value" =~ ^[0-9]+Ki$ || "$value" == "0" || "$value" == "null" ]] && echo "$value" || echo "$value";; *) echo "$value";; esac; }
validate_text() { validate_data "$1" "text"; }
validate_number() { validate_data "$1" "number"; }
validate_kcpu() { validate_data "$1" "kcpu"; }
validate_kmem() { validate_data "$1" "kmem"; }
get_theme() { local theme_name="$1"; unset THEME; declare -g -A THEME; local border_color caption_color; case "${theme_name,,}" in red) border_color='\033[0;31m'; caption_color='\033[0;32m';; blue) border_color='\033[0;34m'; caption_color='\033[0;34m';; *) border_color='\033[0;31m'; caption_color='\033[0;32m'; echo -e "${border_color}Warning: Unknown theme '$theme_name', using Red\033[0m" >&2;; esac; THEME=([border_color]="$border_color" [caption_color]="$caption_color" [header_color]='\033[1;37m' [footer_color]='\033[0;36m' [summary_color]='\033[1;37m' [text_color]='\033[0m' [tl_corner]='╭' [tr_corner]='╮' [bl_corner]='╰' [br_corner]='╯' [h_line]='─' [v_line]='│' [t_junct]='┬' [b_junct]='┴' [l_junct]='├' [r_junct]='┤' [cross]='┼'); }
get_display_length() {
    local text="$1"
    local clean_text
    clean_text=$(echo -n "$text" | sed -E 's/\x1B\[[0-9;]*[a-zA-Z]//g')
    if [[ "$clean_text" =~ ^[[:ascii:]]*$ ]]; then
        echo "${#clean_text}"
        return
    fi
    if [[ ! "$clean_text" =~ [^\x00-\x7F] ]]; then
        echo "${#clean_text}"
        return
    fi
    local codepoints
    codepoints=$(echo -n "$clean_text" | od -An -tx1 | tr -d ' \n')
    local width=0 i=0 len=${#codepoints}
    while [[ $i -lt $len ]]; do
        local byte1_hex="${codepoints:$i:2}"
        local byte1=$((0x$byte1_hex))
        if [[ $byte1 -lt 128 ]]; then
            ((width++)); ((i += 2))
        elif [[ $byte1 -lt 224 ]]; then
            if [[ $((i + 4)) -le $len ]]; then
                local byte2_hex="${codepoints:$((i+2)):2}"
                local codepoint=$(( (byte1 & 0x1F) << 6 | (0x$byte2_hex & 0x3F) ))
                if [[ $codepoint -ge 4352 && $codepoint -le 55215 ]]; then ((width += 2)); else ((width++)); fi
                ((i += 4))
            else ((width++)); ((i += 2)); fi
        elif [[ $byte1 -lt 240 ]]; then
            if [[ $((i + 6)) -le $len ]]; then
                local byte2_hex="${codepoints:$((i+2)):2}" byte3_hex="${codepoints:$((i+4)):2}"
                local codepoint=$(( (byte1 & 0x0F) << 12 | (0x$byte2_hex & 0x3F) << 6 | (0x$byte3_hex & 0x3F) ))
                if [[ $codepoint -ge 127744 && $codepoint -le 129535 ]] || [[ $codepoint -ge 9728 && $codepoint -le 9983 ]]; then ((width += 2)); else ((width++)); fi
                ((i += 6))
            else ((width++)); ((i += 2)); fi
        else
            if [[ $((i + 8)) -le $len ]]; then ((width += 2)); ((i += 8)); else ((width++)); ((i += 2)); fi
        fi
    done
    echo "$width"
}
format_with_commas() {
    local num="$1"
    
    # Handle decimal numbers by separating integer and decimal parts
    if [[ "$num" =~ ^([0-9]+)\.([0-9]+)$ ]]; then
        local integer_part="${BASH_REMATCH[1]}"
        local decimal_part="${BASH_REMATCH[2]}"
        
        # Add commas to integer part
        local result="$integer_part"
        while [[ $result =~ ^([0-9]+)([0-9]{3}.*) ]]; do
            result="${BASH_REMATCH[1]},${BASH_REMATCH[2]}"
        done
        
        echo "${result}.${decimal_part}"
    else
        # Handle integer numbers
        local result="$num"
        while [[ $result =~ ^([0-9]+)([0-9]{3}.*) ]]; do
            result="${BASH_REMATCH[1]},${BASH_REMATCH[2]}"
        done
        echo "$result"
    fi
}
format_text() {
    local value="$1" format="$2" string_limit="$3" wrap_mode="$4" wrap_char="$5" justification="$6"
    [[ -z "$value" || "$value" == "null" ]] && { echo ""; return; }
    if [[ "$string_limit" -gt 0 && ${#value} -gt $string_limit ]]; then
        if [[ "$wrap_mode" == "wrap" && -n "$wrap_char" ]]; then
            local wrapped="" IFS="$wrap_char"; read -ra parts <<< "$value"
            for part in "${parts[@]}"; do wrapped+="$part\n"; done
            echo -e "$wrapped" | head -n $((string_limit / ${#wrap_char}))
        elif [[ "$wrap_mode" == "wrap" ]]; then echo "${value:0:$string_limit}"
        else
            case "$justification" in
                "right") echo "${value: -${string_limit}}";;
                "center") local start=$(( (${#value} - string_limit) / 2 )); echo "${value:${start}:${string_limit}}";;
                *) echo "${value:0:$string_limit}";;
            esac
        fi
    else echo "$value"; fi
}
format_numeric() { local value="$1" format="$2" use_commas="$3"; [[ -z "$value" || "$value" == "null" || "$value" == "0" ]] && { echo ""; return; }; if [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then if [[ -n "$format" ]]; then printf '%s' "$value"; elif [[ "$use_commas" == "true" ]]; then format_with_commas "$value"; else echo "$value"; fi; else echo "$value"; fi; }
format_number() { format_numeric "$1" "$2" "true"; }
format_num() { format_numeric "$1" "$2" "true"; }
format_float() {
    local value="$1" format="$2" column_index="$3"
    [[ -z "$value" || "$value" == "null" || "$value" == "0" ]] && { echo ""; return; }
    
    if [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
        # Use the maximum decimal places for this column if available
        local max_decimals="${MAX_DECIMAL_PLACES[$column_index]:-2}"
        
        # Format with consistent decimal places
        local formatted_value
        formatted_value=$(printf "%.${max_decimals}f" "$value")
        
        # Apply thousands separators
        formatted_value=$(format_with_commas "$formatted_value")
        echo "$formatted_value"
    else
        echo "$value"
    fi
}
format_k_unit() {
    local value="$1" format="$2" unit_type="$3"
    [[ -z "$value" || "$value" == "null" ]] && { echo ""; return; }
    if [[ "$unit_type" == "cpu" ]]; then
        [[ "$value" == "0" || "$value" == "0m" ]] && { echo "0m"; return; }
        if [[ "$value" =~ ^[0-9]+m$ ]]; then
            echo "$(format_with_commas "${value%m}")m"
        elif [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
            printf "%sm" "$(format_with_commas "$((${value%.*} * 1000))")"
        else echo "$value"; fi
    else
        [[ "$value" =~ ^0[MKG]$ || "$value" == "0Mi" || "$value" == "0Gi" || "$value" == "0Ki" ]] && { echo "0M"; return; }
        if [[ "$value" =~ ^[0-9]+[KMG]$ ]]; then
            echo "$(format_with_commas "${value%[KMG]}")${value: -1}"
        elif [[ "$value" =~ ^[0-9]+[MGK]i$ ]]; then
            echo "$(format_with_commas "${value%?i}")${value: -2:1}"
        else echo "$value"; fi
    fi
}
format_kcpu() { format_k_unit "$1" "$2" "cpu"; }
format_kmem() { format_k_unit "$1" "$2" "mem"; }
format_display_value() {
    local value="$1" null_value="$2" zero_value="$3" datatype="$4" format="$5" string_limit="$6" wrap_mode="$7" wrap_char="$8" justification="$9"
    local validate_fn="${DATATYPE_HANDLERS[${datatype}_validate]}" format_fn="${DATATYPE_HANDLERS[${datatype}_format]}"
    value=$("$validate_fn" "$value")
    local display_value
    display_value=$("$format_fn" "$value" "$format" "$string_limit" "$wrap_mode" "$wrap_char" "$justification")
    if [[ "$value" == "null" ]]; then
        case "$null_value" in
            0) display_value="0" ;;
            missing) display_value="Missing" ;;
            *) display_value="" ;;
        esac
    elif [[ "$value" == "0" || "$value" == "0m" || "$value" == "0M" || "$value" == "0G" || "$value" == "0K" ]]; then
        case "$zero_value" in
            0) display_value="0" ;;
            missing) display_value="Missing" ;;
            *) display_value="" ;;
        esac
    fi
    echo "$display_value"
}
declare -gx TABLE_TITLE="" TITLE_WIDTH=0 TITLE_POSITION="none" TABLE_FOOTER="" FOOTER_WIDTH=0 FOOTER_POSITION="none"
declare -ax HEADERS=() KEYS=() JUSTIFICATIONS=() DATATYPES=() NULL_VALUES=() ZERO_VALUES=() FORMATS=() SUMMARIES=() BREAKS=() STRING_LIMITS=() WRAP_MODES=() WRAP_CHARS=() PADDINGS=() WIDTHS=() SORT_KEYS=() SORT_DIRECTIONS=() SORT_PRIORITIES=() IS_WIDTH_SPECIFIED=() VISIBLES=()
validate_input_files() {
    local layout_file="$1" data_file="$2"
    [[ ! -s "$layout_file" || ! -s "$data_file" ]] && echo -e "${THEME[border_color]}Error: Layout or data JSON file empty/missing${THEME[text_color]}" >&2 && return 1
    return 0
}
parse_layout_file() {
    local layout_file="$1" columns_json sort_json
    THEME_NAME=$(jq -r '.theme // "Red"' "$layout_file")
    TABLE_TITLE=$(jq -r '.title // ""' "$layout_file")
    TITLE_POSITION=$(jq -r '.title_position // "none"' "$layout_file" | tr '[:upper:]' '[:lower:]')
    TABLE_FOOTER=$(jq -r '.footer // ""' "$layout_file")
    FOOTER_POSITION=$(jq -r '.footer_position // "none"' "$layout_file" | tr '[:upper:]' '[:lower:]')
    columns_json=$(jq -c '.columns // []' "$layout_file")
    sort_json=$(jq -c '.sort // []' "$layout_file")
    case "$TITLE_POSITION" in left|right|center|full|none) ;; *) echo -e "${THEME[border_color]}Warning: Invalid title position '$TITLE_POSITION', using 'none'${THEME[text_color]}" >&2; TITLE_POSITION="none";; esac
    case "$FOOTER_POSITION" in left|right|center|full|none) ;; *) echo -e "${THEME[border_color]}Warning: Invalid footer position '$FOOTER_POSITION', using 'none'${THEME[text_color]}" >&2; FOOTER_POSITION="none";; esac
    [[ -z "$columns_json" || "$columns_json" == "[]" ]] && echo -e "${THEME[border_color]}Error: No columns defined in layout JSON${THEME[text_color]}" >&2 && return 1
    parse_column_config "$columns_json"; parse_sort_config "$sort_json"
}
parse_column_config() {
    local columns_json="$1"
    HEADERS=(); KEYS=(); JUSTIFICATIONS=(); DATATYPES=(); NULL_VALUES=(); ZERO_VALUES=()
    FORMATS=(); SUMMARIES=(); BREAKS=(); STRING_LIMITS=(); WRAP_MODES=(); WRAP_CHARS=()
    PADDINGS=(); WIDTHS=(); IS_WIDTH_SPECIFIED=(); VISIBLES=()
    local column_count
    column_count=$(jq '. | length' <<<"$columns_json")
    COLUMN_COUNT=$column_count
    for ((i=0; i<column_count; i++)); do
        local col_json
        col_json=$(jq -c ".[$i]" <<<"$columns_json")
        HEADERS[i]=$(jq -r '.header // ""' <<<"$col_json")
        KEYS[i]=$(jq -r '.key // (.header | ascii_downcase | gsub("[^a-z0-9]"; "_"))' <<<"$col_json")
        JUSTIFICATIONS[i]=$(jq -r '.justification // "left"' <<<"$col_json" | tr '[:upper:]' '[:lower:]')
        DATATYPES[i]=$(jq -r '.datatype // "text"' <<<"$col_json" | tr '[:upper:]' '[:lower:]')
        NULL_VALUES[i]=$(jq -r '.null_value // "blank"' <<<"$col_json" | tr '[:upper:]' '[:lower:]')
        ZERO_VALUES[i]=$(jq -r '.zero_value // "blank"' <<<"$col_json" | tr '[:upper:]' '[:lower:]')
        FORMATS[i]=$(jq -r '.format // ""' <<<"$col_json")
        SUMMARIES[i]=$(jq -r '.summary // "none"' <<<"$col_json" | tr '[:upper:]' '[:lower:]')
        BREAKS[i]=$(jq -r '.break // false' <<<"$col_json")
        STRING_LIMITS[i]=$(jq -r '.string_limit // 0' <<<"$col_json")
        WRAP_MODES[i]=$(jq -r '.wrap_mode // "clip"' <<<"$col_json" | tr '[:upper:]' '[:lower:]')
        WRAP_CHARS[i]=$(jq -r '.wrap_char // ""' <<<"$col_json")
        PADDINGS[i]=$(jq -r '.padding // '"$DEFAULT_PADDING" <<<"$col_json")
        local visible_raw visible_key_check
        visible_raw=$(jq -r '.visible // true' <<<"$col_json")
        visible_key_check=$(jq -r 'has("visible")' <<<"$col_json")
        VISIBLES[i]=$(if [[ "$visible_key_check" == "true" ]]; then jq -r '.visible' <<<"$col_json"; else echo "$visible_raw"; fi)
        local specified_width
        specified_width=$(jq -r '.width // 0' <<<"$col_json")
        if [[ $specified_width -gt 0 ]]; then
            WIDTHS[i]=$specified_width; IS_WIDTH_SPECIFIED[i]="true"
        else
            WIDTHS[i]=$((${#HEADERS[i]} + (2 * PADDINGS[i]))); IS_WIDTH_SPECIFIED[i]="false"
        fi
        validate_column_config "$i" "${HEADERS[$i]}" "${JUSTIFICATIONS[$i]}" "${DATATYPES[$i]}" "${SUMMARIES[$i]}"
    done
}
validate_column_config() {
    local i="$1" header="$2" justification="$3" datatype="$4" summary="$5"
    [[ -z "$header" ]] && echo -e "${THEME[border_color]}Error: Column $i has no header${THEME[text_color]}" >&2 && return 1
    [[ "$justification" != "left" && "$justification" != "right" && "$justification" != "center" ]] && echo -e "${THEME[border_color]}Warning: Invalid justification '$justification' for column $header, using 'left'${THEME[text_color]}" >&2 && JUSTIFICATIONS[i]="left"
    [[ -z "${DATATYPE_HANDLERS[${datatype}_validate]}" ]] && echo -e "${THEME[border_color]}Warning: Invalid datatype '$datatype' for column $header, using 'text'${THEME[text_color]}" >&2 && DATATYPES[i]="text"
    local valid_summaries="${DATATYPE_HANDLERS[${DATATYPES[$i]}_summary_types]}"
    [[ "$summary" != "none" && ! " $valid_summaries " =~ $summary ]] && echo -e "${THEME[border_color]}Warning: Summary '$summary' not supported for datatype '${DATATYPES[$i]}' in column $header, using 'none'${THEME[text_color]}" >&2 && SUMMARIES[i]="none"
}
parse_sort_config() {
    local sort_json="$1"
    SORT_KEYS=(); SORT_DIRECTIONS=(); SORT_PRIORITIES=()
    local sort_count
    sort_count=$(jq '. | length' <<<"$sort_json")
    for ((i=0; i<sort_count; i++)); do
        local sort_item
        sort_item=$(jq -c ".[$i]" <<<"$sort_json")
        SORT_KEYS[i]=$(jq -r '.key // ""' <<<"$sort_item")
        SORT_DIRECTIONS[i]=$(jq -r '.direction // "asc"' <<<"$sort_item" | tr '[:upper:]' '[:lower:]')
        SORT_PRIORITIES[i]=$(jq -r '.priority // 0' <<<"$sort_item")
        [[ -z "${SORT_KEYS[$i]}" ]] && echo -e "${THEME[border_color]}Warning: Sort item $i has no key, ignoring${THEME[text_color]}" >&2 && continue
        [[ "${SORT_DIRECTIONS[$i]}" != "asc" && "${SORT_DIRECTIONS[$i]}" != "desc" ]] && echo -e "${THEME[border_color]}Warning: Invalid sort direction '${SORT_DIRECTIONS[$i]}' for key ${SORT_KEYS[$i]}, using 'asc'${THEME[text_color]}" >&2 && SORT_DIRECTIONS[i]="asc"
    done
}
declare -a ROW_JSONS=() DATA_ROWS=()
declare -A SUM_SUMMARIES=() COUNT_SUMMARIES=() MIN_SUMMARIES=() MAX_SUMMARIES=() UNIQUE_VALUES=() AVG_SUMMARIES=() AVG_COUNTS=() MAX_DECIMAL_PLACES=()
initialize_summaries() {
    SUM_SUMMARIES=(); COUNT_SUMMARIES=(); MIN_SUMMARIES=(); MAX_SUMMARIES=()
    UNIQUE_VALUES=(); AVG_SUMMARIES=(); AVG_COUNTS=(); MAX_DECIMAL_PLACES=()
    for ((i=0; i<COLUMN_COUNT; i++)); do
        SUM_SUMMARIES[$i]=0; COUNT_SUMMARIES[$i]=0; MIN_SUMMARIES[$i]=""; MAX_SUMMARIES[$i]=""
        UNIQUE_VALUES[$i]=""; AVG_SUMMARIES[$i]=0; AVG_COUNTS[$i]=0; MAX_DECIMAL_PLACES[$i]=0
        blanks_count[$i]=0; nonblanks_count[$i]=0
    done
}
prepare_data() {
    local data_file="$1"
    DATA_ROWS=()
    local data_json
    data_json=$(jq -c '. // []' "$data_file")
    local row_count
    row_count=$(jq '. | length' <<<"$data_json")
    [[ $row_count -eq 0 ]] && return
        local jq_expr=".[] | ["
        for key in "${KEYS[@]}"; do jq_expr+=".${key} // null,"; done
        jq_expr="${jq_expr%,}] | join(\"\t\")"
        local all_data
        all_data=$(jq -r "$jq_expr" "$data_file")
    IFS=$'\n' read -d '' -r -a rows <<< "$all_data"
    for ((i=0; i<row_count; i++)); do
        IFS=$'\t' read -r -a values <<< "${rows[$i]}"
        declare -A row_data
        for ((j=0; j<${#KEYS[@]}; j++)); do
            local key="${KEYS[$j]}" value="${values[$j]}"
            [[ "$value" == "null" ]] && value="null" || value="${value:-null}"
            row_data["$key"]="$value"
        done
        local row_data_str
        row_data_str=$(declare -p row_data)
        DATA_ROWS[i]="$row_data_str"
    done
}
sort_data() {
    [[ ${#SORT_KEYS[@]} -eq 0 ]] && return
    local indices=(); for ((i=0; i<${#DATA_ROWS[@]}; i++)); do indices+=("$i"); done
    get_sort_value() {
        local idx="$1" key="$2"
        declare -A row_data
        if ! eval "${DATA_ROWS[$idx]}"; then echo ""; return; fi
        if [[ -v "row_data[$key]" ]]; then echo "${row_data[$key]}"; else echo ""; fi
    }
    local primary_key="${SORT_KEYS[0]}" primary_dir="${SORT_DIRECTIONS[0]}"
    local sorted_indices=()
    IFS=$'\n' read -d '' -r -a sorted_indices < <(for idx in "${indices[@]}"; do
        value=$(get_sort_value "$idx" "$primary_key"); printf "%s\t%s\n" "$value" "$idx"
    done | sort -k1,1"${primary_dir:0:1}" | cut -f2)
    local temp_rows=("${DATA_ROWS[@]}"); DATA_ROWS=()
    for idx in "${sorted_indices[@]}"; do DATA_ROWS+=("${temp_rows[$idx]}"); done
}
process_data_rows() {
    local row_count; MAX_LINES=1; row_count=${#DATA_ROWS[@]}
    [[ $row_count -eq 0 ]] && return
    # First pass: update summaries and max decimal places
    for ((i=0; i<row_count; i++)); do
        declare -A row_data
        if ! eval "${DATA_ROWS[$i]}"; then continue; fi
        for ((j=0; j<COLUMN_COUNT; j++)); do
            local key="${KEYS[$j]}" value="null"
            if [[ -v "row_data[$key]" ]]; then value="${row_data[$key]}"; fi
            local validated_value=$("${DATATYPE_HANDLERS[${DATATYPES[$j]}_validate]}" "$value")
            update_summaries "$j" "$validated_value" "${DATATYPES[$j]}" "${SUMMARIES[$j]}"
        done
    done
    # Second pass: calculate display values, widths, and max lines
    ROW_JSONS=()
    for ((i=0; i<row_count; i++)); do
        local row_json line_count=1
        row_json="{\"row\":$i}"; ROW_JSONS+=("$row_json")
        declare -A row_data
        if ! eval "${DATA_ROWS[$i]}"; then continue; fi
        for ((j=0; j<COLUMN_COUNT; j++)); do
            local key="${KEYS[$j]}" datatype="${DATATYPES[$j]}" format="${FORMATS[$j]}" string_limit="${STRING_LIMITS[$j]}" wrap_mode="${WRAP_MODES[$j]}" wrap_char="${WRAP_CHARS[$j]}"
            local value="null"
            if [[ -v "row_data[$key]" ]]; then value="${row_data[$key]}"; fi
            local validated_value=$("${DATATYPE_HANDLERS[${datatype}_validate]}" "$value")
            local display_value=$("${DATATYPE_HANDLERS[${datatype}_format]}" "$validated_value" "$format" "$string_limit" "$wrap_mode" "$wrap_char" "$j")
            if [[ "$validated_value" == "null" ]]; then
                case "${NULL_VALUES[$j]}" in 0) display_value="0";; missing) display_value="Missing";; *) display_value="";; esac
            elif [[ "$datatype" == "int" || "$datatype" == "num" || "$datatype" == "float" || "$datatype" == "kcpu" || "$datatype" == "kmem" ]]; then
                local is_zero=0 num_val=$(echo "$validated_value" | awk '{print ($1 + 0)}')
                if [[ $(echo "if ($num_val == 0) 1 else 0" | bc) -eq 1 ]]; then is_zero=1; fi
                if [[ $is_zero -eq 1 ]]; then
                    case "${ZERO_VALUES[$j]}" in 0) display_value="0";; missing) display_value="Missing";; *) display_value="";; esac
                fi
            fi
            if [[ "${IS_WIDTH_SPECIFIED[j]}" != "true" && "${VISIBLES[j]}" == "true" ]]; then
                if [[ -n "$wrap_char" && "$wrap_mode" == "wrap" && -n "$display_value" && "$validated_value" != "null" ]]; then
                    local max_len=0 IFS="$wrap_char"; read -ra parts <<<"$display_value"
                    for part in "${parts[@]}"; do
                        local len=$(get_display_length "$part")
                        [[ $len -gt $max_len ]] && max_len=$len
                    done
                    local padded_width=$((max_len + (2 * PADDINGS[j]))); [[ $padded_width -gt ${WIDTHS[j]} ]] && WIDTHS[j]=$padded_width
                    [[ ${#parts[@]} -gt $line_count ]] && line_count=${#parts[@]}
                else
                    local len=$(get_display_length "$display_value")
                    local padded_width=$((len + (2 * PADDINGS[j]))); [[ $padded_width -gt ${WIDTHS[j]} ]] && WIDTHS[j]=$padded_width
                fi
            fi
        done
        [[ $line_count -gt $MAX_LINES ]] && MAX_LINES=$line_count
    done
    # Update widths for summaries
    for ((j=0; j<COLUMN_COUNT; j++)); do
        if [[ "${SUMMARIES[$j]}" != "none" ]]; then
            local summary_value=$(format_summary_value "$j" "${SUMMARIES[$j]}" "${DATATYPES[$j]}" "${FORMATS[$j]}")
            if [[ -n "$summary_value" && "${IS_WIDTH_SPECIFIED[j]}" != "true" && "${VISIBLES[j]}" == "true" ]]; then
                local summary_len=$(get_display_length "$summary_value")
                local summary_padded_width=$((summary_len + (2 * PADDINGS[j])))
                [[ $summary_padded_width -gt ${WIDTHS[j]} ]] && WIDTHS[j]=$summary_padded_width
            fi
        fi
    done
}
update_summaries() {
    local j="$1" value="$2" datatype="$3" summary_type="$4"
    # Track maximum decimal places for float data type
    if [[ "$datatype" == "float" && "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
        local decimal_part="${value#*.}"
        local decimal_places=0
        if [[ -n "$decimal_part" && "$decimal_part" != "$value" ]]; then
            decimal_places=${#decimal_part}
        fi
        if [[ $decimal_places -gt ${MAX_DECIMAL_PLACES[$j]:-0} ]]; then
            MAX_DECIMAL_PLACES[$j]=$decimal_places
        fi
    fi
    # Compute is_blank matching C logic
    local is_blank=0
    if [[ "$value" == "null" || -z "$value" ]]; then
        is_blank=1
    else
        local num_val=0
        case "$datatype" in
            int|num|float)
                num_val=$(echo "$value" | awk '{print $1 + 0}')
                if [[ $(echo "if ($num_val == 0) 1 else 0" | bc) -eq 1 ]]; then is_blank=1; fi
                ;;
            kcpu)
                local num_part="$value"
                if [[ "$value" == *m ]]; then num_part="${value%m}"; fi
                num_val=$(echo "$num_part" | awk '{print $1 + 0}')
                if [[ $(echo "if ($num_val == 0) 1 else 0" | bc) -eq 1 ]]; then is_blank=1; fi
                ;;
            kmem)
                local unit="" num_part="$value"
                if [[ "$value" =~ ([0-9.]+)([KMG]i?)$ ]]; then
                    num_part=${BASH_REMATCH[1]}
                    unit=${BASH_REMATCH[2]}
                fi
                num_val=$(echo "$num_part" | awk '{print $1 + 0}')
                case "$unit" in
                    K|Ki) num_val=$(echo "$num_val / 1000" | bc -l) ;;
                    G|Gi) num_val=$(echo "$num_val * 1000" | bc -l) ;;
                    M|Mi) ;;
                esac
                if [[ $(echo "if ($num_val == 0) 1 else 0" | bc) -eq 1 ]]; then is_blank=1; fi
                ;;
        esac
    fi
    (( blanks_count[$j] += is_blank ))
    (( nonblanks_count[$j] += (1 - is_blank) ))
    [[ $is_blank -eq 1 ]] && return
    case "$summary_type" in
        sum)
            if [[ "$datatype" == "kcpu" && "$value" =~ ^[0-9]+m$ ]]; then SUM_SUMMARIES[$j]=$(( ${SUM_SUMMARIES[$j]:-0} + ${value%m} ))
            elif [[ "$datatype" == "kmem" ]]; then
                if [[ "$value" =~ ^[0-9]+M$ ]]; then SUM_SUMMARIES[$j]=$(( ${SUM_SUMMARIES[$j]:-0} + ${value%M} ))
                elif [[ "$value" =~ ^[0-9]+G$ ]]; then SUM_SUMMARIES[$j]=$(( ${SUM_SUMMARIES[$j]:-0} + ${value%G} * 1000 ))
                elif [[ "$value" =~ ^[0-9]+K$ ]]; then SUM_SUMMARIES[$j]=$(( ${SUM_SUMMARIES[$j]:-0} + ${value%K} / 1000 ))
                elif [[ "$value" =~ ^[0-9]+Mi$ ]]; then SUM_SUMMARIES[$j]=$(( ${SUM_SUMMARIES[$j]:-0} + ${value%Mi} ))
                elif [[ "$value" =~ ^[0-9]+Gi$ ]]; then SUM_SUMMARIES[$j]=$(( ${SUM_SUMMARIES[$j]:-0} + ${value%Gi} * 1000 )); fi
            elif [[ "$datatype" == "int" || "$datatype" == "num" ]]; then
                if [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then local int_value=${value%.*}; [[ "$int_value" == "$value" ]] && int_value=$value; SUM_SUMMARIES[$j]=$((${SUM_SUMMARIES[$j]:-0} + int_value)); fi
            elif [[ "$datatype" == "float" ]]; then
                if [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then SUM_SUMMARIES[$j]=$(echo "${SUM_SUMMARIES[$j]:-0} + $value" | bc); fi
            fi;;
        min)
            if [[ "$datatype" == "kcpu" && "$value" =~ ^[0-9]+m$ ]]; then
                local num_val="${value%m}"
                if [[ -z "${MIN_SUMMARIES[$j]}" ]] || (( num_val < ${MIN_SUMMARIES[$j]:-999999} )); then MIN_SUMMARIES[$j]="$num_val"; fi
            elif [[ "$datatype" == "kmem" && "$value" =~ ^[0-9]+[KMG]$ ]]; then
                local num_val="${value%[KMG]}"
                local unit="${value: -1}"
                if [[ "$unit" == "G" ]]; then num_val=$((num_val * 1000)); elif [[ "$unit" == "K" ]]; then num_val=$((num_val / 1000)); fi
                if [[ -z "${MIN_SUMMARIES[$j]}" ]] || (( num_val < ${MIN_SUMMARIES[$j]:-999999} )); then MIN_SUMMARIES[$j]="$num_val"; fi
            elif [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                if [[ -z "${MIN_SUMMARIES[$j]}" ]] || (( $(printf "%.0f" "$value") < $(printf "%.0f" "${MIN_SUMMARIES[$j]:-999999}") )); then MIN_SUMMARIES[$j]="$value"; fi
            fi;;
        max)
            if [[ "$datatype" == "kcpu" && "$value" =~ ^[0-9]+m$ ]]; then
                local num_val="${value%m}"
                if [[ -z "${MAX_SUMMARIES[$j]}" ]] || (( num_val > ${MAX_SUMMARIES[$j]:-0} )); then MAX_SUMMARIES[$j]="$num_val"; fi
            elif [[ "$datatype" == "kmem" && "$value" =~ ^[0-9]+[KMG]$ ]]; then
                local num_val="${value%[KMG]}"
                local unit="${value: -1}"
                if [[ "$unit" == "G" ]]; then num_val=$((num_val * 1000)); elif [[ "$unit" == "K" ]]; then num_val=$((num_val / 1000)); fi
                if [[ -z "${MAX_SUMMARIES[$j]}" ]] || (( num_val > ${MAX_SUMMARIES[$j]:-0} )); then MAX_SUMMARIES[$j]="$num_val"; fi
            elif [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                if [[ -z "${MAX_SUMMARIES[$j]}" ]] || (( $(printf "%.0f" "$value") > $(printf "%.0f" "${MAX_SUMMARIES[$j]:-0}") )); then MAX_SUMMARIES[$j]="$value"; fi
            fi;;
        count) if [[ -n "$value" && "$value" != "null" ]]; then COUNT_SUMMARIES[$j]=$(( ${COUNT_SUMMARIES[$j]:-0} + 1 )); fi;;
        unique) if [[ -n "$value" && "$value" != "null" ]]; then
            if [[ -z "${UNIQUE_VALUES[$j]}" ]]; then UNIQUE_VALUES[$j]="$value"; else UNIQUE_VALUES[$j]+=" $value"; fi; fi;;
        avg)
            if [[ "$datatype" == "int" || "$datatype" == "float" || "$datatype" == "num" ]]; then
                if [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                    if [[ "$datatype" == "float" ]]; then
                        # Use bc to add floating-point values for precision
                        AVG_SUMMARIES[$j]=$(echo "${AVG_SUMMARIES[$j]:-0} + $value" | bc)
                    else
                        # Integer arithmetic for int/num types
                        local int_value=${value%.*}
                        [[ "$int_value" == "$value" ]] && int_value=$value
                        AVG_SUMMARIES[$j]=$((${AVG_SUMMARIES[$j]:-0} + int_value))
                    fi
                    AVG_COUNTS[$j]=$(( ${AVG_COUNTS[$j]:-0} + 1 ))
                fi
            fi
            ;;
    esac
}
format_summary_value() {
    local j="$1" summary_type="$2" datatype="$3" format="$4" summary_value=""
    case "$summary_type" in
        sum)
            if [[ -n "${SUM_SUMMARIES[$j]}" && "${SUM_SUMMARIES[$j]}" != "0" ]]; then
                if [[ "$datatype" == "kcpu" ]]; then
                    summary_value="$(format_with_commas "${SUM_SUMMARIES[$j]}")m"
                elif [[ "$datatype" == "kmem" ]]; then
                    summary_value="$(format_with_commas "${SUM_SUMMARIES[$j]}")M"
                elif [[ "$datatype" == "num" ]]; then
                    summary_value=$(format_num "${SUM_SUMMARIES[$j]}" "$format")
                elif [[ "$datatype" == "int" ]]; then
                    summary_value=$(format_with_commas "${SUM_SUMMARIES[$j]}")
                elif [[ "$datatype" == "float" ]]; then
                    local decimals=${MAX_DECIMAL_PLACES[$j]:-2}
                    local formatted_sum=$(printf "%.${decimals}f" "${SUM_SUMMARIES[$j]}")
                    summary_value=$(format_with_commas "$formatted_sum")
                fi
            fi
            ;;
        min)
            summary_value="${MIN_SUMMARIES[$j]:-}"
            if [[ "$datatype" == "kcpu" && -n "$summary_value" ]]; then
                summary_value="$(format_with_commas "$summary_value")m"
            elif [[ "$datatype" == "kmem" && -n "$summary_value" ]]; then
                summary_value="$(format_with_commas "$summary_value")M"
            elif [[ "$datatype" == "float" && -n "$summary_value" && -n "${MAX_DECIMAL_PLACES[$j]}" ]]; then
                local decimals=${MAX_DECIMAL_PLACES[$j]:-2}
                local formatted_min=$(printf "%.${decimals}f" "$summary_value")
                summary_value=$(format_with_commas "$formatted_min")
            elif [[ "$datatype" == "int" && -n "$summary_value" ]]; then
                summary_value=$(format_with_commas "$summary_value")
            fi
            ;;
        max)
            summary_value="${MAX_SUMMARIES[$j]:-}"
            if [[ "$datatype" == "kcpu" && -n "$summary_value" ]]; then
                summary_value="$(format_with_commas "$summary_value")m"
            elif [[ "$datatype" == "kmem" && -n "$summary_value" ]]; then
                summary_value="$(format_with_commas "$summary_value")M"
            elif [[ "$datatype" == "float" && -n "$summary_value" && -n "${MAX_DECIMAL_PLACES[$j]}" ]]; then
                local decimals=${MAX_DECIMAL_PLACES[$j]:-2}
                local formatted_max=$(printf "%.${decimals}f" "$summary_value")
                summary_value=$(format_with_commas "$formatted_max")
            elif [[ "$datatype" == "int" && -n "$summary_value" ]]; then
                summary_value=$(format_with_commas "$summary_value")
            fi
            ;;
        count)
            summary_value="${COUNT_SUMMARIES[$j]:-0}"
            ;;
        unique)
            if [[ -n "${UNIQUE_VALUES[$j]}" ]]; then
                summary_value=$(echo "${UNIQUE_VALUES[$j]}" | tr ' ' '\n' | sort -u | wc -l)
            else
                summary_value="0"
            fi
            ;;
        blanks)
            summary_value=$(format_with_commas "${blanks_count[$j]:-0}")
            ;;
        nonblanks)
            summary_value=$(format_with_commas "${nonblanks_count[$j]:-0}")
            ;;
        avg)
            if [[ -n "${AVG_SUMMARIES[$j]}" && "${AVG_COUNTS[$j]}" -gt 0 ]]; then
                if [[ "$datatype" == "float" ]]; then
                    local decimals=${MAX_DECIMAL_PLACES[$j]:-2}
                    local avg_result=$(awk "BEGIN {printf \"%.${decimals}f\", (${AVG_SUMMARIES[$j]}) / (${AVG_COUNTS[$j]})}")
                    summary_value=$(format_with_commas "$avg_result")
                elif [[ "$datatype" == "int" ]]; then
                    local avg_result=$((${AVG_SUMMARIES[$j]} / ${AVG_COUNTS[$j]}))
                    summary_value=$(format_with_commas "$avg_result")
                elif [[ "$datatype" == "num" ]]; then
                    local avg_result=$((${AVG_SUMMARIES[$j]} / ${AVG_COUNTS[$j]}))
                    summary_value=$(format_num "$avg_result" "$format")
                else
                    summary_value="$((${AVG_SUMMARIES[$j]} / ${AVG_COUNTS[$j]}))"
                fi
            else
                summary_value="0"
            fi
            ;;
    esac
    echo "$summary_value"
}
calculate_element_width() { local text="$1" total_table_width="$2" position="$3" width_var="$4"; if [[ -n "$text" ]]; then local evaluated_text; evaluated_text=$(eval "echo \"$text\"" 2>/dev/null); evaluated_text=$(replace_color_placeholders "$evaluated_text"); evaluated_text=$(printf '%b' "$evaluated_text"); local text_length; text_length=$(get_display_length "$evaluated_text"); if [[ "$position" == "none" ]]; then declare -g "$width_var"=$((text_length + (2 * DEFAULT_PADDING))); elif [[ "$position" == "full" ]]; then declare -g "$width_var"=$total_table_width; else declare -g "$width_var"=$((text_length + (2 * DEFAULT_PADDING))); [[ ${!width_var} -gt $total_table_width ]] && declare -g "$width_var"=$total_table_width; fi; else declare -g "$width_var"=0; fi; }
calculate_title_width() { calculate_element_width "$1" "$2" "$TITLE_POSITION" "TITLE_WIDTH"; }
calculate_footer_width() { calculate_element_width "$1" "$2" "$FOOTER_POSITION" "FOOTER_WIDTH"; }
calculate_table_width() {
    local total_table_width=0 visible_count=0
    for ((i=0; i<COLUMN_COUNT; i++)); do
        if [[ "${VISIBLES[i]}" == "true" ]]; then
            ((total_table_width += WIDTHS[i])); ((visible_count++))
        fi
    done
    [[ $visible_count -gt 1 ]] && ((total_table_width += visible_count - 1))
    echo "$total_table_width"
}
clip_text_with_colors() {
    local text="$1" width="$2" justification="$3"
    
    # First, convert color placeholders to ANSI codes
    local colored_text
    colored_text=$(replace_color_placeholders "$text")
    
    # Check if clipping is needed after color processing
    local display_length
    display_length=$(get_display_length "$colored_text")
    if [[ $display_length -le $width ]]; then
        echo "$colored_text"
        return
    fi
    
    # Helper function to clip text while preserving ANSI codes
    # This matches the C implementation's logic exactly
    case "$justification" in
        right)
            # For right justification, skip characters from the beginning
            local excess=$(( display_length - width ))
            local pos=0
            local visible_count=0
            local in_ansi=false
            
            # Skip 'excess' visible characters from the beginning
            while [[ $pos -lt ${#colored_text} && $visible_count -lt $excess ]]; do
                local char="${colored_text:$pos:1}"
                if [[ "$char" == $'\033' ]]; then
                    in_ansi=true
                elif [[ "$in_ansi" == true && "$char" == "m" ]]; then
                    in_ansi=false
                elif [[ "$in_ansi" == false ]]; then
                    ((visible_count++))
                fi
                ((pos++))
            done
            
            # Return the remainder
            echo "${colored_text:$pos}"
            ;;
        center)
            # For center justification, clip from both ends
            local excess=$(( display_length - width ))
            local left_clip=$(( excess / 2 ))
            
            # Find start position by skipping left_clip visible characters
            local start_pos=0
            local visible_count=0
            local in_ansi=false
            
            while [[ $start_pos -lt ${#colored_text} && $visible_count -lt $left_clip ]]; do
                local char="${colored_text:$start_pos:1}"
                if [[ "$char" == $'\033' ]]; then
                    in_ansi=true
                elif [[ "$in_ansi" == true && "$char" == "m" ]]; then
                    in_ansi=false
                elif [[ "$in_ansi" == false ]]; then
                    ((visible_count++))
                fi
                ((start_pos++))
            done
            
            # Find end position by taking 'width' visible characters from start_pos
            local end_pos=$start_pos
            visible_count=0
            in_ansi=false
            
            while [[ $end_pos -lt ${#colored_text} && $visible_count -lt $width ]]; do
                local char="${colored_text:$end_pos:1}"
                if [[ "$char" == $'\033' ]]; then
                    in_ansi=true
                elif [[ "$in_ansi" == true && "$char" == "m" ]]; then
                    in_ansi=false
                elif [[ "$in_ansi" == false ]]; then
                    ((visible_count++))
                fi
                ((end_pos++))
            done
            
            # Return the middle portion
            echo "${colored_text:$start_pos:$((end_pos - start_pos))}"
            ;;
        *)
            # For left justification, take first 'width' visible characters
            local pos=0
            local visible_count=0
            local in_ansi=false
            
            while [[ $pos -lt ${#colored_text} && $visible_count -lt $width ]]; do
                local char="${colored_text:$pos:1}"
                if [[ "$char" == $'\033' ]]; then
                    in_ansi=true
                elif [[ "$in_ansi" == true && "$char" == "m" ]]; then
                    in_ansi=false
                elif [[ "$in_ansi" == false ]]; then
                    ((visible_count++))
                fi
                ((pos++))
            done
            
            # Return the left portion
            echo "${colored_text:0:$pos}"
            ;;
    esac
}

clip_text() {
    clip_text_with_colors "$@"
}
render_cell() {
    local content="$1" width="$2" padding="$3" justification="$4" color="$5"
    local content_width=$((width - (2 * padding)))
    local visible_len
    visible_len=$(get_display_length "$content")
    local left_spaces right_spaces
    case "$justification" in
        right)
            left_spaces=$((padding + content_width - visible_len))
            right_spaces=$padding
            ;;
        center)
            local spaces=$(( (content_width - visible_len) / 2 ))
            left_spaces=$((padding + spaces))
            right_spaces=$((padding + content_width - visible_len - spaces))
            ;;
        *)
            left_spaces=$padding
            right_spaces=$((padding + content_width - visible_len))
            ;;
    esac
    printf "%*s${color}%s${THEME[text_color]}%*s${THEME[border_color]}${THEME[v_line]}${THEME[text_color]}" \
        "$left_spaces" "" \
        "$content" \
        "$right_spaces" ""
}
render_table_element() {
    local element_type="$1" total_table_width="$2"
    local element_text element_position element_width color_theme
    if [[ "$element_type" == "title" ]]; then
        [[ -z "$TABLE_TITLE" ]] && return
        element_text=$(eval echo "$TABLE_TITLE" 2>/dev/null)
        element_text=$(replace_color_placeholders "$element_text")
        element_text=$(printf '%b' "$element_text")
        element_position="$TITLE_POSITION"
        element_width="$TITLE_WIDTH"
        color_theme="${THEME[header_color]}"
    else
        [[ -z "$TABLE_FOOTER" ]] && return
        element_text=$(eval echo "$TABLE_FOOTER" 2>/dev/null)
        element_text=$(replace_color_placeholders "$element_text")
        element_text=$(printf '%b' "$element_text")
        element_position="$FOOTER_POSITION"
        element_width="$FOOTER_WIDTH"
        color_theme="${THEME[footer_color]}"
    fi
    local offset=0
    case "$element_position" in
        left) offset=0 ;;
        right) offset=$((total_table_width - element_width)) ;;
        center) offset=$(((total_table_width - element_width) / 2)) ;;
        full) offset=0 ;;
        *) offset=0 ;;
    esac
    if [[ "$element_type" == "title" ]]; then
        [[ $offset -gt 0 ]] && printf "%*s" "$offset" ""
        printf "${THEME[border_color]}%s" "${THEME[tl_corner]}"
        printf "${THEME[h_line]}%.0s" $(seq 1 "$element_width")
        printf "%s${THEME[text_color]}\n" "${THEME[tr_corner]}"
    fi
    [[ $offset -gt 0 ]] && printf "%*s" "$offset" ""
    printf "${THEME[border_color]}%s${THEME[text_color]}" "${THEME[v_line]}"
    local available_width=$((element_width - (2 * DEFAULT_PADDING)))
    element_text=$(clip_text "$element_text" "$available_width" "$element_position")
    case "$element_position" in
        left) printf "%*s${color_theme}%-*s${NC}%*s" "$DEFAULT_PADDING" "" "$available_width" "$element_text" "$DEFAULT_PADDING" "" ;;
        right) printf "%*s${color_theme}%*s${NC}%*s" "$DEFAULT_PADDING" "" "$available_width" "$element_text" "$DEFAULT_PADDING" "" ;;
        center) local element_len; element_len=$(get_display_length "$element_text"); printf "%*s${color_theme}%s${NC}%*s" "$DEFAULT_PADDING" "" "$element_text" "$((available_width - element_len + DEFAULT_PADDING))" "" ;;
        full) local text_len; text_len=$(get_display_length "$element_text"); local spaces=$(( (available_width - text_len) / 2 )); local left_spaces=$(( DEFAULT_PADDING + spaces )); local right_spaces=$(( DEFAULT_PADDING + available_width - text_len - spaces )); printf "%*s${color_theme}%s${NC}%*s" "$left_spaces" "" "$element_text" "$right_spaces" "" ;;
        *) printf "%*s${color_theme}%s${NC}%*s" "$DEFAULT_PADDING" "" "$element_text" "$DEFAULT_PADDING" "" ;;
    esac
    printf "${THEME[border_color]}%s${THEME[text_color]}\n" "${THEME[v_line]}"
    if [[ "$element_type" == "footer" ]]; then
        [[ $offset -gt 0 ]] && printf "%*s" "$offset" ""
        echo -ne "${THEME[border_color]}${THEME[bl_corner]}"
        for i in $(seq 1 "$element_width"); do echo -ne "${THEME[h_line]}"; done
        echo -ne "${THEME[br_corner]}${THEME[text_color]}\n"
    fi
}
get_border_chars() {
    local border_type="$1"
    if [[ "$border_type" == "top" ]]; then
        echo "${THEME[tl_corner]} ${THEME[tr_corner]} ${THEME[t_junct]}"
    else
        echo "${THEME[bl_corner]} ${THEME[br_corner]} ${THEME[b_junct]}"
    fi
}
render_table_border() {
    local border_type="$1" total_table_width="$2" element_offset="$3" element_right_edge="$4" element_width="$5"
    local column_widths_sum=0 column_positions=()
    for ((i=0; i<COLUMN_COUNT-1; i++)); do
        if [[ "${VISIBLES[i]}" == "true" ]]; then
            column_widths_sum=$((column_widths_sum + WIDTHS[i]))
            local has_more_visible=false
            for ((j=$((i+1)); j<COLUMN_COUNT; j++)); do
                [[ "${VISIBLES[j]}" == "true" ]] && has_more_visible=true && break
            done
            [[ "$has_more_visible" == "true" ]] && column_positions+=("$column_widths_sum") && ((column_widths_sum++))
        fi
    done
    local max_width=$((total_table_width + 2))
    [[ -n "$element_width" && $element_width -gt 0 && $((element_width + 2)) -gt $max_width ]] && max_width=$((element_width + 2))
    read -r left_char right_char junction_char <<< "$(get_border_chars "$border_type")"
    [[ -n "$element_width" && $element_width -gt 0 && $element_offset -eq 0 ]] && left_char="${THEME[l_junct]}"
    local border_string=""
    for ((i=0; i<max_width; i++)); do
        local char_to_print="${THEME[h_line]}"
        if [[ $i -eq 0 ]]; then char_to_print="$left_char"
        elif [[ $i -eq $((max_width - 1)) ]]; then
            if [[ -n "$element_width" && $element_width -gt 0 && $element_right_edge -gt $total_table_width ]]; then
                char_to_print=$(if [[ "$border_type" == "top" ]]; then echo "${THEME[br_corner]}"; else echo "${THEME[tr_corner]}"; fi)
            elif [[ -n "$element_width" && $element_width -gt 0 && $element_right_edge -eq $total_table_width ]]; then
                char_to_print="${THEME[r_junct]}"
            else char_to_print="$right_char"; fi
        else
            for pos in "${column_positions[@]}"; do [[ $((pos + 1)) -eq $i ]] && char_to_print="$junction_char" && break; done
            if [[ -n "$element_width" && $element_width -gt 0 ]]; then
                if [[ $i -eq $element_offset && $element_offset -gt 0 && $element_offset -lt $((total_table_width + 1)) ]] || [[ $i -eq $((element_right_edge + 1)) && $((element_right_edge + 1)) -lt $((total_table_width + 1)) ]]; then
                    local is_column_line=false
                    for pos in "${column_positions[@]}"; do [[ $((pos + 1)) -eq $i ]] && is_column_line=true && break; done
                    if [[ "$is_column_line" == "true" ]]; then char_to_print="${THEME[cross]}"
                    else char_to_print=$(if [[ "$border_type" == "top" ]]; then echo "${THEME[b_junct]}"; else echo "${THEME[t_junct]}"; fi); fi
                elif [[ $i -eq $((total_table_width + 1)) && $i -lt $((max_width - 1)) && $element_right_edge -gt $((total_table_width - 1)) ]]; then
                    char_to_print=$(if [[ "$border_type" == "top" ]]; then echo "${THEME[t_junct]}"; else echo "${THEME[b_junct]}"; fi)
                fi
            fi
        fi
        border_string+="$char_to_print"
    done
    printf "${THEME[border_color]}%s${THEME[text_color]}\n" "$border_string"
}
render_table_top_border() {
    local total_table_width
    total_table_width=$(calculate_table_width)
    local title_offset=0 title_right_edge=0 title_width="" title_position="none"
    if [[ -n "$TABLE_TITLE" ]]; then
        title_width=$TITLE_WIDTH; title_position=$TITLE_POSITION
        case "$TITLE_POSITION" in
            left) title_offset=0; title_right_edge=$TITLE_WIDTH ;;
            right) title_offset=$((total_table_width - TITLE_WIDTH)); title_right_edge=$total_table_width ;;
            center) title_offset=$(((total_table_width - TITLE_WIDTH) / 2)); title_right_edge=$((title_offset + TITLE_WIDTH)) ;;
            full) title_offset=0; title_right_edge=$total_table_width ;;
            *) title_offset=0; title_right_edge=$TITLE_WIDTH ;;
        esac
    fi
    render_table_border "top" "$total_table_width" "$title_offset" "$title_right_edge" "$title_width" "$title_position" "$([[ "$title_position" == "full" ]] && echo true || echo false)"
}
render_table_bottom_border() {
    local total_table_width
    total_table_width=$(calculate_table_width)
    local footer_offset=0 footer_right_edge=0 footer_width="" footer_position="none"
    if [[ -n "$TABLE_FOOTER" ]]; then
        footer_width=$FOOTER_WIDTH; footer_position=$FOOTER_POSITION
        case "$FOOTER_POSITION" in
            left) footer_offset=0; footer_right_edge=$FOOTER_WIDTH ;;
            right) footer_offset=$((total_table_width - FOOTER_WIDTH)); footer_right_edge=$total_table_width ;;
            center) footer_offset=$(((total_table_width - FOOTER_WIDTH) / 2)); footer_right_edge=$((footer_offset + FOOTER_WIDTH)) ;;
            full) footer_offset=0; footer_right_edge=$total_table_width ;;
            *) footer_offset=0; footer_right_edge=$FOOTER_WIDTH ;;
        esac
    fi
    render_table_border "bottom" "$total_table_width" "$footer_offset" "$footer_right_edge" "$footer_width" "$footer_position" "$([[ "$footer_position" == "full" ]] && echo true || echo false)"
}
render_table_headers() {
    printf "${THEME[border_color]}%s${THEME[text_color]}" "${THEME[v_line]}"
    for ((i=0; i<COLUMN_COUNT; i++)); do
        local visible="${VISIBLES[i]}"
        if [[ "$visible" == "true" ]]; then
            local header_text="${HEADERS[$i]}" width="${WIDTHS[i]}" padding="${PADDINGS[i]}" justification="${JUSTIFICATIONS[$i]}"
            local content_width=$((width - (2 * padding)))
            header_text=$(clip_text "$header_text" "$content_width" "$justification")
            render_cell "$header_text" "$width" "$padding" "$justification" "${THEME[caption_color]}"
        fi
    done
    printf "\n"
}
render_table_separator() {
    local type="$1"
    local left_char="${THEME[l_junct]}" right_char="${THEME[r_junct]}" middle_char="${THEME[cross]}"
    [[ "$type" == "bottom" ]] && left_char="${THEME[bl_corner]}" && right_char="${THEME[br_corner]}" && middle_char="${THEME[b_junct]}"
    printf "${THEME[border_color]}%s" "${left_char}"
    for ((i=0; i<COLUMN_COUNT; i++)); do
        if [[ "${VISIBLES[i]}" == "true" ]]; then
            local width=${WIDTHS[i]}
            for ((j=0; j<width; j++)); do printf "%s" "${THEME[h_line]}"; done
            if [[ $i -lt $((COLUMN_COUNT-1)) ]]; then
                local next_visible=false
                for ((k=$((i+1)); k<COLUMN_COUNT; k++)); do
                    if [[ "${VISIBLES[k]}" == "true" ]]; then next_visible=true; break; fi
                done
                [[ "$next_visible" == "true" ]] && printf "%s" "${middle_char}"
            fi
        fi
    done
    printf "%s${THEME[text_color]}\n" "${right_char}"
}
render_data_rows() {
    [[ ${#DATA_ROWS[@]} -eq 0 ]] && return
    local last_break_values=()
    for ((j=0; j<COLUMN_COUNT; j++)); do last_break_values[j]=""; done
    for ((row_idx=0; row_idx<${#DATA_ROWS[@]}; row_idx++)); do
        eval "${DATA_ROWS[$row_idx]}"
        # Check if we need a break
        local needs_break=false
        for ((j=0; j<COLUMN_COUNT; j++)); do
            if [[ "${BREAKS[$j]}" == "true" ]]; then
                local key="${KEYS[$j]}" value
                value="${row_data[$key]}"
                if [[ -n "${last_break_values[$j]}" && "$value" != "${last_break_values[$j]}" ]]; then
                    needs_break=true
                    break
                fi
            fi
        done
        if [[ "$needs_break" == "true" ]]; then
    render_table_separator "middle"
        fi
        local -A line_values
        local row_line_count=1
        for ((j=0; j<COLUMN_COUNT; j++)); do
            local key="${KEYS[$j]}" value
            value="${row_data[$key]}"
            local display_value
            local validate_fn="${DATATYPE_HANDLERS[${DATATYPES[j]}_validate]}"
            local validated_value=$("$validate_fn" "$value")
            display_value=$("${DATATYPE_HANDLERS[${DATATYPES[j]}_format]}" "$validated_value" "${FORMATS[j]}" "${STRING_LIMITS[j]}" "${WRAP_MODES[j]}" "${WRAP_CHARS[j]}" "$j")
            if [[ "$validated_value" == "null" ]]; then
                case "${NULL_VALUES[j]}" in 0) display_value="0";; missing) display_value="Missing";; *) display_value="";; esac
            elif [[ "$datatype" == "int" || "$datatype" == "num" || "$datatype" == "float" || "$datatype" == "kcpu" || "$datatype" == "kmem" ]]; then
                local is_zero=0
                local num_val=$(echo "$validated_value" | awk '{print $1 + 0}')
                if [[ $(echo "if ($num_val == 0) 1 else 0" | bc) -eq 1 ]]; then is_zero=1; fi
                if [[ $is_zero -eq 1 ]]; then
                    case "${ZERO_VALUES[j]}" in 0) display_value="0";; missing) display_value="Missing";; *) display_value="";; esac
                fi
            fi
            if [[ -n "${WRAP_CHARS[$j]}" && "${WRAP_MODES[$j]}" == "wrap" && -n "$display_value" && "$value" != "null" ]]; then
                local IFS="${WRAP_CHARS[$j]}"
                read -ra parts <<<"$display_value"
                for k in "${!parts[@]}"; do
                    local part="${parts[k]}"
                    local content_width=$((WIDTHS[j] - (2 * PADDINGS[j])))
                    local part_len; part_len=$(get_display_length "$part")
                    if [[ $part_len -gt $content_width ]]; then
                        case "${JUSTIFICATIONS[$j]}" in
                            right)
                                part="${part: -${content_width}}"
                                ;;
                            center)
                                local excess=$(( part_len - content_width ))
                                local left_clip=$(( excess / 2 ))
                                part="${part:${left_clip}:${content_width}}"
                                ;;
                            *)
                                part="${part:0:${content_width}}"
                                ;;
                        esac
                    fi
                    line_values[$j,$k]="$part"
                done
                [[ ${#parts[@]} -gt $row_line_count ]] && row_line_count=${#parts[@]}
            elif [[ "${WRAP_MODES[$j]}" == "wrap" && -n "$display_value" && "$value" != "null" ]]; then
                local content_width=$((WIDTHS[j] - (2 * PADDINGS[j])))
                local words=()
                IFS=' ' read -ra words <<<"$display_value"
                local current_line=""
                local line_index=0
                for word in "${words[@]}"; do
                    if [[ -z "$current_line" ]]; then
                        current_line="$word"
                    elif [[ $(( ${#current_line} + ${#word} + 1 )) -le $content_width ]]; then
                        current_line="$current_line $word"
                    else
                        line_values[$j,$line_index]="$current_line"
                        current_line="$word"
                        ((line_index++))
                    fi
                done
                if [[ -n "$current_line" ]]; then
                    line_values[$j,$line_index]="$current_line"
                    ((line_index++))
                fi
                [[ $line_index -gt $row_line_count ]] && row_line_count=$line_index
            else
                # No wrapping - handle single line with potential clipping
                local content_width=$((WIDTHS[j] - (2 * PADDINGS[j])))
                
                # Use color-aware clipping to properly handle ANSI codes
                if [[ "${IS_WIDTH_SPECIFIED[j]}" == "true" ]]; then
                    display_value=$(clip_text_with_colors "$display_value" "$content_width" "${JUSTIFICATIONS[$j]}")
                fi
                
                # Store the single line value
                line_values[$j,0]="$display_value"
            fi
        done
        for ((line=0; line<row_line_count; line++)); do
            printf "${THEME[border_color]}%s${THEME[text_color]}" "${THEME[v_line]}"
            for ((j=0; j<COLUMN_COUNT; j++)); do
                if [[ "${VISIBLES[j]}" == "true" ]]; then
                    local display_value="${line_values[$j,$line]:-}"
                    local content_width=$((WIDTHS[j] - (2 * PADDINGS[j])))
                    
                    # Process color placeholders and handle clipping if needed
                    display_value=$(replace_color_placeholders "$display_value")
                    display_value=$(printf '%b' "$display_value")
                    
                    # Only clip if width is specified and the content is too long
                    if [[ "${IS_WIDTH_SPECIFIED[j]}" == "true" ]]; then
                        local display_length
                        display_length=$(get_display_length "$display_value")
                        if [[ $display_length -gt $content_width ]]; then
                            display_value=$(clip_text_with_colors "$display_value" "$content_width" "${JUSTIFICATIONS[$j]}")
                        fi
                    fi
                    
                    render_cell "$display_value" "${WIDTHS[j]}" "${PADDINGS[j]}" "${JUSTIFICATIONS[j]}" "${THEME[text_color]}"
                fi
            done
            printf "\n"
        done
        for ((j=0; j<COLUMN_COUNT; j++)); do
            if [[ "${BREAKS[$j]}" == "true" ]]; then
                local key="${KEYS[$j]}" value
                value="${row_data[$key]}"
                last_break_values[j]="$value"
            fi
        done
    done
}
render_summaries_row() {
    local has_summaries=false
    for ((i=0; i<COLUMN_COUNT; i++)); do
        [[ "${SUMMARIES[$i]}" != "none" ]] && has_summaries=true && break
    done
    if [[ "$has_summaries" == true ]]; then
        render_table_separator "middle"
        printf "${THEME[border_color]}%s${THEME[text_color]}" "${THEME[v_line]}"
        for ((i=0; i<COLUMN_COUNT; i++)); do
            if [[ "${VISIBLES[i]}" == "true" ]]; then
                local summary_value
                summary_value=$(format_summary_value "$i" "${SUMMARIES[$i]}" "${DATATYPES[$i]}" "${FORMATS[$i]}")
                local content_width=$((WIDTHS[i] - (2 * PADDINGS[i])))
                local summary_value_len; summary_value_len=$(get_display_length "$summary_value")
                if [[ $summary_value_len -gt $content_width && "${IS_WIDTH_SPECIFIED[i]}" == "true" ]]; then
                    case "${JUSTIFICATIONS[$i]}" in
                        right)
                                summary_value="${summary_value: -$content_width}"
                            ;;
                        center)
                            local excess=$(( summary_value_len - content_width ))
                            local left_clip=$(( excess / 2 ))
                            summary_value="${summary_value:$left_clip:$content_width}"
                            ;;
                        *)
                            summary_value="${summary_value:0:$content_width}"
                            ;;
                    esac
                fi
                render_cell "$summary_value" "${WIDTHS[i]}" "${PADDINGS[i]}" "${JUSTIFICATIONS[i]}" "${THEME[summary_color]}"
            fi
        done
        printf "\n"
        return 0
    fi
    return 1
}
draw_table() {
    local layout_file="$1" data_file="$2"
    if [[ "$1" == "--help" || "$1" == "-h" ]]; then show_help; return 0; fi
    if [[ "$1" == "--version" ]]; then echo "tables.sh version $TABLES_VERSION"; return 0; fi
    if [[ $# -eq 0 ]]; then show_help; return 0; fi
    if [[ -z "$layout_file" || -z "$data_file" ]]; then
        echo "Error: Both layout and data files are required" >&2
        echo "Use --help for usage information" >&2
        return 1
    fi
    shift 2
    while [[ $# -gt 0 ]]; do
        case "$1" in
            *) echo "Error: Unknown option: $1" >&2; echo "Use --help for usage information" >&2; return 1 ;;
        esac
    done
    validate_input_files "$layout_file" "$data_file" || return 1
    parse_layout_file "$layout_file" || return 1
    get_theme "$THEME_NAME"
    initialize_summaries
    prepare_data "$data_file"
    sort_data
    process_data_rows
    local total_table_width
    total_table_width=$(calculate_table_width)
    if [[ -n "$TABLE_TITLE" ]]; then
        calculate_title_width "$TABLE_TITLE" "$total_table_width"
    fi
    if [[ -n "$TABLE_FOOTER" ]]; then
        calculate_footer_width "$TABLE_FOOTER" "$total_table_width"
    fi
    [[ -n "$TABLE_TITLE" ]] && render_table_element "title" "$total_table_width"
    render_table_top_border
    render_table_headers
    render_table_separator "middle"
    render_data_rows "$MAX_LINES"
    has_summaries=false
    render_summaries_row && has_summaries=true
    render_table_bottom_border
    [[ -n "$TABLE_FOOTER" ]] && render_table_element "footer" "$total_table_width"
}
tables_main() { draw_table "$@"; }
tables_render() { local layout_file="$1" data_file="$2"; shift 2; draw_table "$layout_file" "$data_file" "$@"; }
tables_render_from_json() { local layout_json="$1" data_json="$2"; shift 2; local temp_layout temp_data; temp_layout=$(mktemp); temp_data=$(mktemp); trap 'rm -f "$temp_layout" "$temp_data"' RETURN; echo "$layout_json" > "$temp_layout"; echo "$data_json" > "$temp_data"; draw_table "$temp_layout" "$temp_data" "$@"; }
tables_get_themes() { echo "Available themes: Red, Blue"; }
tables_version() { echo "$TABLES_VERSION"; }
tables_reset() { COLUMN_COUNT=0; MAX_LINES=1; THEME_NAME="Red"; TABLE_TITLE=""; TITLE_WIDTH=0; TITLE_POSITION="none"; TABLE_FOOTER=""; FOOTER_WIDTH=0; FOOTER_POSITION="none"; HEADERS=(); KEYS=(); JUSTIFICATIONS=(); DATATYPES=(); NULL_VALUES=(); ZERO_VALUES=(); FORMATS=(); SUMMARIES=(); IS_WIDTH_SPECIFIED=(); VISIBLES=(); BREAKS=(); STRING_LIMITS=(); WRAP_MODES=(); WRAP_CHARS=(); PADDINGS=(); WIDTHS=(); SORT_KEYS=(); SORT_DIRECTIONS=(); SORT_PRIORITIES=(); ROW_JSONS=(); DATA_ROWS=(); SUM_SUMMARIES=(); COUNT_SUMMARIES=(); MIN_SUMMARIES=(); MAX_SUMMARIES=(); UNIQUE_VALUES=(); AVG_SUMMARIES=(); AVG_COUNTS=(); get_theme "$THEME_NAME"; }
if [[ "${BASH_SOURCE[0]}" != "$0" ]]; then
    export -f tables_render tables_render_from_json tables_get_themes tables_version tables_reset draw_table get_theme format_with_commas get_display_length
else
    tables_main "$@"
fi
