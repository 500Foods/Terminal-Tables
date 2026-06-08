#!/usr/bin/env bash

# Test Suite 8: Footer Positions - Validation of table footer positions
# This test suite focuses on validating the rendering of table footers in different positions
# (not supplied, left, center, right) with footers of varying lengths relative to table width
# (less than, equal to, greater than table width).

# Create temporary files for our JSON
layout_file=$(mktemp)
data_file=$(mktemp)
tables_script="$(dirname "$0")/../tables.sh"

# Cleanup function
cleanup() {
    rm -f "$layout_file" "$data_file"
}
trap cleanup EXIT

# Check for --debug flag
DEBUG_FLAG=""
if [[ "$1" == "--debug" ]]; then
    DEBUG_FLAG="--debug"
    echo "Debug mode enabled"
fi

# Note: If ANSI color codes appear as strings in output, ensure your terminal supports ANSI escape sequences.
# If the issue persists, consider checking 'tables.sh' for color handling or adding a no-color option if available.

# Setup test data with consistent server information
cat > "$data_file" << 'EOF'
[
  {
    "id": 1,
    "server_name": "web-server-01",
    "cpu_cores": 4,
    "load_avg": 2.45
  },
  {
    "id": 2,
    "server_name": "db-server-01",
    "cpu_cores": 8,
    "load_avg": 5.12
  },
  {
    "id": 3,
    "server_name": "cache-server",
    "cpu_cores": 2,
    "load_avg": 0.85
  },
  {
    "id": 4,
    "server_name": "api-gateway",
    "cpu_cores": 6,
    "load_avg": 3.21
  }
]
EOF

# Test 8-A: Not Supplied - Footer less than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "footer": "Summary Report",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo "Test 8-A: Not Supplied - Footer less than table width"
echo "----------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-B: Not Supplied - Footer equal to table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "footer": "Summary Performance Metric Report Data 23",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-B: Not Supplied - Footer equal to table width"
echo "---------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-C: Not Supplied - Footer greater than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "footer": "Detailed Summary Performance and Configuration Analysis Report for Q2 2023",
  "columns": [
    {
      "header": "ID Number",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name Identifier",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores Count",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Average Value",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-C: Not Supplied - Footer greater than table width"
echo "-------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-D: Left - Footer less than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "Summary Report",
  "footer_position": "left",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-D: Left - Footer less than table width"
echo "--------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-E: Left - Footer equal to table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "Summary Performance Metric Report Data 23",
  "footer_position": "left",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-E: Left - Footer equal to table width"
echo "-------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-F: Left - Footer greater than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "Detailed Summary Performance and Configuration Analysis Report for Q2 2023",
  "footer_position": "left",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-F: Left - Footer greater than table width"
echo "-----------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-G: Center - Footer less than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "footer": "Summary Report",
  "footer_position": "center",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-G: Center - Footer less than table width"
echo "----------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-H: Center - Footer equal to table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "footer": "Summary Performance Metric Report Data 23",
  "footer_position": "center",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-H: Center - Footer equal to table width"
echo "---------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-I: Center - Footer greater than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "footer": "Detailed Summary Performance and Configuration Analysis Report for Q2 2023",
  "footer_position": "center",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-I: Center - Footer greater than table width"
echo "-------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-J: Right - Footer less than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "Summary Report",
  "footer_position": "right",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-J: Right - Footer less than table width"
echo "---------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-K: Right - Footer equal to table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "Summary Performance Metric Report Data 23",
  "footer_position": "right",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-K: Right - Footer equal to table width"
echo "--------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-L: Right - Footer greater than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "Detailed Summary Performance and Configuration Analysis Report for Q2 2023",
  "footer_position": "right",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-L: Right - Footer greater than table width"
echo "------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-M: Full - Footer with color
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "{RED}Summary Report{RESET}",
  "footer_position": "full",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-M: Full - Footer with color"
echo "----------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-N: Full - Footer greater than table width
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "Detailed Summary Performance and Configuration Analysis Report for Q2 2023 which is a very long footer text to test clipping behavior in full position",
  "footer_position": "full",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    }
  ]
}
EOF

echo -e "\nTest 8-N: Full - Footer greater than table width"
echo "-----------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-O: Right - Dynamic footer with date calculations (wide table)
cat > "$data_file" << 'EOF'
[
  {
    "id": 1,
    "server_name": "web-server-01",
    "cpu_cores": 4,
    "memory_gb": 16,
    "load_avg": 2.45,
    "status": "Active",
    "location": "US-East"
  },
  {
    "id": 2,
    "server_name": "db-server-01",
    "cpu_cores": 8,
    "memory_gb": 32,
    "load_avg": 5.12,
    "status": "Active",
    "location": "US-West"
  },
  {
    "id": 3,
    "server_name": "cache-server",
    "cpu_cores": 2,
    "memory_gb": 8,
    "load_avg": 0.85,
    "status": "Standby",
    "location": "EU-Central"
  }
]
EOF

cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "footer": "Date: $(date +%A) Date: $(date '+%B %d') Time: $(date '+%H:%M:%S')",
  "footer_position": "right",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Memory (GB)",
      "key": "memory_gb",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center"
    },
    {
      "header": "Location",
      "key": "location",
      "datatype": "text",
      "justification": "left"
    }
  ]
}
EOF

echo -e "\nTest 8-O: Right - Dynamic footer with date calculations (wide table)"
echo "------------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-P: Dynamic title and footer with date calculations (testing both elements)
cat > "$data_file" << 'EOF'
[
  {
    "id": 1,
    "server_name": "web-server-01",
    "cpu_cores": 4,
    "memory_gb": 16,
    "load_avg": 2.45,
    "status": "Active",
    "location": "US-East"
  },
  {
    "id": 2,
    "server_name": "db-server-01",
    "cpu_cores": 8,
    "memory_gb": 32,
    "load_avg": 5.12,
    "status": "Active",
    "location": "US-West"
  }
]
EOF

cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Report {RED}──{NC} $(date '+%Y-%m-%d %H:%M:%S')",
  "title_position": "center",
  "footer": "End {RED}──{NC} $(date +%A) {RED}──{NC} $(date '+%B %d') {RED}──{NC} $(date '+%H:%M:%S')",
  "footer_position": "right",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Memory (GB)",
      "key": "memory_gb",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center"
    },
    {
      "header": "Location",
      "key": "location",
      "datatype": "text",
      "justification": "left"
    }
  ]
}
EOF

echo -e "\nTest 8-P: Dynamic title and footer with date calculations (testing both elements)"
echo "------------------------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# Test 8-Q: Unicode double-width characters (emojis and checkmarks)
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Report {RED}──{NC} $(date '+%Y-%m-%d %H:%M:%S') 😊",
  "title_position": "center",
  "footer": "✓ End {RED}──{NC} $(date +%A) {RED}──{NC} $(date '+%B %d') {RED}──{NC} $(date '+%H:%M:%S') ✓",
  "footer_position": "right",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Memory (GB)",
      "key": "memory_gb",
      "datatype": "num",
      "justification": "right"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center"
    },
    {
      "header": "Location",
      "key": "location",
      "datatype": "text",
      "justification": "left"
    }
  ]
}
EOF

echo -e "\nTest 8-Q: Unicode double-width characters (emojis and checkmarks)"
echo "----------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG
