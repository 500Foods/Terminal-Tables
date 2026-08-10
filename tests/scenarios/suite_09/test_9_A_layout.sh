#!/usr/bin/env bash
server_count=$(jq length "$1")
cat << EOF
{
  "theme": "Red",
  "title": "Server Overview Report - Total Servers: $server_count",
  "title_position": "center",
  "footer": "Generated on $(date +%Y-%m-%d)",
  "footer_position": "center",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "Category",
      "key": "category",
      "datatype": "text",
      "justification": "center",
      "summary": "unique"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
EOF
