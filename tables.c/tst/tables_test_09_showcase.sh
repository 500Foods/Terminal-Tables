#!/usr/bin/env bash

# Test Suite 9: Showcase - Ultimate demonstration of tables.sh capabilities
# This test suite presents 20 comprehensive examples to flex the full power of the tables system,
# featuring headers, footers, summaries, various justifications, wrapping, breaking, and more.

# Create temporary files for our JSON
layout_file=$(mktemp)
data_file=$(mktemp)
tables_script="$(dirname "$0")/../tables"

# Cleanup function
cleanup() {
    rm -f "$layout_file" "$data_file"
}
trap cleanup EXIT

# Check for --debug or --debug_layout flag
DEBUG_FLAG=""
if [[ "$1" == "--debug" ]] || [[ "$1" == "--debug_layout" ]]; then
    DEBUG_FLAG="--debug_layout"
    echo "Debug mode enabled"
fi

# Setup comprehensive test data showcasing various datatypes and long text for wrapping
cat > "$data_file" << 'EOF'
[
  {
    "id": 1,
    "server_name": "web-server-01",
    "category": "Web",
    "cpu_cores": 4,
    "load_avg": 2.45,
    "cpu_usage": "1250m",
    "memory_usage": "2048Mi",
    "status": "Running",
    "description": "Primary web server for frontend applications with a detailed setup for high availability.",
    "tags": "frontend,app,ui,primary,loadbalancer",
    "location": "US-East",
    "uptime_days": 120,
    "bandwidth_mbps": 1000.5
  },
  {
    "id": 2,
    "server_name": "db-server-01",
    "category": "Database",
    "cpu_cores": 8,
    "load_avg": 5.12,
    "cpu_usage": "3200m",
    "memory_usage": "8192Mi", 
    "status": "Running",
    "description": "Main database server handling critical data operations with replication enabled.",
    "tags": "db,sql,storage,primary,backend",
    "location": "US-West",
    "uptime_days": 180,
    "bandwidth_mbps": 500.25
  },
  {
    "id": 3,
    "server_name": "cache-server",
    "category": "Cache",
    "cpu_cores": 2,
    "load_avg": 0.85,
    "cpu_usage": "500m",
    "memory_usage": "1024Mi",
    "status": "Starting",
    "description": "In-memory cache for speeding up data access across multiple services.",
    "tags": "cache,redis,fast,memory",
    "location": "EU-Central",
    "uptime_days": 10,
    "bandwidth_mbps": 200.75
  },
  {
    "id": 4,
    "server_name": "api-gateway",
    "category": "Web",
    "cpu_cores": 6,
    "load_avg": 3.21,
    "cpu_usage": "2100m",
    "memory_usage": "4096Mi",
    "status": "Running",
    "description": "API gateway managing incoming requests and routing to appropriate services.",
    "tags": "api,gateway,routing,web,interface",
    "location": "US-East",
    "uptime_days": 90,
    "bandwidth_mbps": 750.0
  },
  {
    "id": 5,
    "server_name": "backup-server",
    "category": "Storage",
    "cpu_cores": 4,
    "load_avg": 1.15,
    "cpu_usage": "800m",
    "memory_usage": "4096Mi",
    "status": "Running",
    "description": "Dedicated backup server for periodic data snapshots and recovery.",
    "tags": "backup,storage,recovery,secondary",
    "location": "US-West",
    "uptime_days": 200,
    "bandwidth_mbps": 300.0
  }
]
EOF

# TestC 9-A: Basic table with centered title and footer, mixed summaries
# Pre-evaluate the server count for the title
server_count=$(jq length "$data_file")
cat > "$layout_file" << EOF
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

echo "TestC 9-A: Basic table with centered title and footer, mixed summaries"
echo "-------------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-B: Long title and footer with wrapping description
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Server Performance Report Q2 2023",
  "title_position": "left",
  "footer": "Data Compiled by IT Department",
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
      "header": "Description",
      "key": "description",
      "datatype": "text",
      "justification": "left",
      "width": 30,
      "wrap_mode": "wrap"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "avg"
    }
  ]
}
EOF

echo -e "\nTestC 9-B: Long title and footer with wrapping description"
echo "-------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-C: Multiple summary types with centered tags and full-width title
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Comprehensive Resource Utilization Dashboard for Enterprise Systems Monitoring",
  "title_position": "full",
  "footer": "Data Accurate as of $(date '+%Y-%m-%d %H:%M:%S')",
  "footer_position": "center",
  "columns": [
    {
      "header": "Server",
      "key": "server_name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "CPU Usage",
      "key": "cpu_usage",
      "datatype": "kcpu",
      "justification": "right",
      "summary": "max"
    },
    {
      "header": "Tags",
      "key": "tags",
      "datatype": "text",
      "justification": "center",
      "width": 20,
      "wrap_mode": "wrap",
      "wrap_char": ","
    }
  ]
}
EOF

echo -e "\nTestC 9-C: Multiple summary types with centered tags and full-width title"
echo "----------------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-D: Breaking by category with right-aligned footer and mixed summaries
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Server Inventory by Category",
  "title_position": "right",
  "footer": "Report Generated for Internal Use Only",
  "footer_position": "right",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Category",
      "key": "category",
      "datatype": "text",
      "justification": "left",
      "break": true,
      "summary": "unique"
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center"
    },
    {
      "header": "Memory Usage",
      "key": "memory_usage",
      "datatype": "kmem",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
EOF

echo -e "\nTestC 9-D: Breaking by category with right-aligned footer and mixed summaries"
echo "--------------------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-E: Elaborate full-width footer with summaries and mixed justifications
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Enterprise Server Management System",
  "title_position": "center",
  "footer": "Detailed Analytics and Performance Metrics for Strategic Planning - IT Dept. $(date +%Y)",
  "footer_position": "full",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "min"
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
      "header": "Description",
      "key": "description",
      "datatype": "text",
      "justification": "right",
      "width": 25,
      "wrap_mode": "wrap"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "center",
      "summary": "max"
    }
  ]
}
EOF

echo -e "\nTestC 9-E: Elaborate full-width footer with summaries and mixed justifications"
echo "---------------------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-F: Left-aligned title and footer with bandwidth summaries
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Network Bandwidth Analysis",
  "title_position": "left",
  "footer": "Data Source: Network Monitoring System",
  "footer_position": "left",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Server",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "Bandwidth (Mbps)",
      "key": "bandwidth_mbps",
      "datatype": "float",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Location",
      "key": "location",
      "datatype": "text",
      "justification": "center",
      "summary": "unique"
    }
  ]
}
EOF

echo -e "\nTestC 9-F: Left-aligned title and footer with bandwidth summaries"
echo "--------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-G: Uptime report with mixed data types and centered elements
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Server Uptime Report",
  "title_position": "center",
  "footer": "Uptime Data as of Today",
  "footer_position": "center",
  "columns": [
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "Uptime (Days)",
      "key": "uptime_days",
      "datatype": "int",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center"
    },
    {
      "header": "Category",
      "key": "category",
      "datatype": "text",
      "justification": "center",
      "summary": "unique"
    }
  ]
}
EOF

echo -e "\nTestC 9-G: Uptime report with mixed data types and centered elements"
echo "-----------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-H: Detailed server specs with wrapping and multiple summaries
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Detailed Server Specifications",
  "title_position": "full",
  "footer": "Specifications Compiled for Review",
  "footer_position": "right",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Server",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Memory",
      "key": "memory_usage",
      "datatype": "kmem",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Description",
      "key": "description",
      "datatype": "text",
      "justification": "left",
      "width": 25,
      "wrap_mode": "wrap"
    }
  ]
}
EOF

echo -e "\nTestC 9-H: Detailed server specs with wrapping and multiple summaries"
echo "-----------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-I: Location-based analysis with breaking and summaries
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Server Distribution by Location",
  "title_position": "center",
  "footer": "Location Data for Infrastructure Planning",
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
      "header": "Location",
      "key": "location",
      "datatype": "text",
      "justification": "left",
      "break": true,
      "summary": "unique"
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
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "avg"
    }
  ]
}
EOF

echo -e "\nTestC 9-I: Location-based analysis with breaking and summaries"
echo "-----------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-J: Status report with centered title and right-aligned footer
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Server Status Report",
  "title_position": "center",
  "footer": "Status Updated Hourly",
  "footer_position": "right",
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
      "justification": "left"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center",
      "summary": "unique"
    },
    {
      "header": "Uptime (Days)",
      "key": "uptime_days",
      "datatype": "int",
      "justification": "right",
      "summary": "max"
    }
  ]
}
EOF

echo -e "\nTestC 9-J: Status report with centered title and right-aligned footer"
echo "-----------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-K: Performance metrics with full-width footer and summaries
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Performance Metrics Dashboard",
  "title_position": "left",
  "footer": "Performance Data for System Optimization and Capacity Planning - IT Operations Team",
  "footer_position": "full",
  "columns": [
    {
      "header": "Server",
      "key": "server_name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "CPU Usage",
      "key": "cpu_usage",
      "datatype": "kcpu",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "Bandwidth (Mbps)",
      "key": "bandwidth_mbps",
      "datatype": "float",
      "justification": "right",
      "summary": "max"
    }
  ]
}
EOF

echo -e "\nTestC 9-K: Performance metrics with full-width footer and summaries"
echo "----------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-L: Category breakdown with breaking and left-aligned elements
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Server Category Breakdown",
  "title_position": "left",
  "footer": "Category Analysis for Resource Allocation",
  "footer_position": "left",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Category",
      "key": "category",
      "datatype": "text",
      "justification": "left",
      "break": true,
      "summary": "unique"
    },
    {
      "header": "Server",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Memory",
      "key": "memory_usage",
      "datatype": "kmem",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
EOF

echo -e "\nTestC 9-L: Category breakdown with breaking and left-aligned elements"
echo "------------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-M: Resource utilization with centered footer and mixed justifications
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Resource Utilization Overview",
  "title_position": "right",
  "footer": "Resource Data for Capacity Planning",
  "footer_position": "center",
  "columns": [
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left",
      "summary": "count"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "CPU Usage",
      "key": "cpu_usage",
      "datatype": "kcpu",
      "justification": "center",
      "summary": "sum"
    },
    {
      "header": "Memory Usage",
      "key": "memory_usage",
      "datatype": "kmem",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
EOF

echo -e "\nTestC 9-M: Resource utilization with centered footer and mixed justifications"
echo "-------------------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-N: Bandwidth report with full-width title and summaries
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Network Bandwidth Utilization Report for All Data Centers and Regions Worldwide",
  "title_position": "full",
  "footer": "Bandwidth Metrics for Network Optimization",
  "footer_position": "left",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Server",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "Location",
      "key": "location",
      "datatype": "text",
      "justification": "center",
      "summary": "unique"
    },
    {
      "header": "Bandwidth (Mbps)",
      "key": "bandwidth_mbps",
      "datatype": "float",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
EOF

echo -e "\nTestC 9-N: Bandwidth report with full-width title and summaries"
echo "------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-O: Server description with wrapping and right-aligned title
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Server Descriptions Catalog",
  "title_position": "right",
  "footer": "Descriptions for Documentation",
  "footer_position": "right",
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
      "justification": "left"
    },
    {
      "header": "Description",
      "key": "description",
      "datatype": "text",
      "justification": "left",
      "width": 30,
      "wrap_mode": "wrap"
    }
  ]
}
EOF

echo -e "\nTestC 9-O: Server description with wrapping and right-aligned title"
echo "---------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-P: Tag analysis with centered elements and summaries
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Server Tag Analysis",
  "title_position": "center",
  "footer": "Tag Data for Classification",
  "footer_position": "center",
  "columns": [
    {
      "header": "Server",
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
      "header": "Tags",
      "key": "tags",
      "datatype": "text",
      "justification": "center",
      "width": 25,
      "wrap_mode": "wrap",
      "wrap_char": ","
    }
  ]
}
EOF

echo -e "\nTestC 9-P: Tag analysis with centered elements and summaries"
echo "--------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-Q: Mixed data types with full-width footer and breaking
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Mixed Data Type Analysis",
  "title_position": "left",
  "footer": "Comprehensive Server Data for All Metrics and Categories - IT Operations $(date +%Y)",
  "footer_position": "full",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Location",
      "key": "location",
      "datatype": "text",
      "justification": "left",
      "break": true,
      "summary": "unique"
    },
    {
      "header": "Server",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "CPU Usage",
      "key": "cpu_usage",
      "datatype": "kcpu",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Bandwidth",
      "key": "bandwidth_mbps",
      "datatype": "float",
      "justification": "right",
      "summary": "avg"
    }
  ]
}
EOF

echo -e "\nTestC 9-Q: Mixed data types with full-width footer and breaking"
echo "-----------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-R: Detailed uptime with summaries and centered title
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Detailed Uptime Analysis",
  "title_position": "center",
  "footer": "Uptime Metrics for Reliability Assessment",
  "footer_position": "right",
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
      "justification": "left"
    },
    {
      "header": "Uptime (Days)",
      "key": "uptime_days",
      "datatype": "int",
      "justification": "right",
      "summary": "avg"
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center",
      "summary": "unique"
    },
    {
      "header": "Location",
      "key": "location",
      "datatype": "text",
      "justification": "left",
      "summary": "unique"
    }
  ]
}
EOF

echo -e "\nTestC 9-R: Detailed uptime with summaries and centered title"
echo "--------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-S: Complex layout with all features combined
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Ultimate Server Dashboard - All Metrics and Insights for Enterprise IT Management",
  "title_position": "full",
  "footer": "Complete Server Data for Strategic Decision Making - Generated on $(date +%Y-%m-%d)",
  "footer_position": "full",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count"
    },
    {
      "header": "Category",
      "key": "category",
      "datatype": "text",
      "justification": "left",
      "break": true,
      "summary": "unique"
    },
    {
      "header": "Server",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "Description",
      "key": "description",
      "datatype": "text",
      "justification": "left",
      "width": 20,
      "wrap_mode": "wrap"
    },
    {
      "header": "CPU Cores",
      "key": "cpu_cores",
      "datatype": "num",
      "justification": "right",
      "summary": "sum"
    },
    {
      "header": "Load Avg",
      "key": "load_avg",
      "datatype": "float",
      "justification": "center",
      "summary": "avg"
    },
    {
      "header": "Memory",
      "key": "memory_usage",
      "datatype": "kmem",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
EOF

echo -e "\nTestC 9-S: Complex layout with all features combined"
echo "------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-T: Final showcase with diverse justifications and summaries
cat > "$layout_file" << 'EOF'
{
  "theme": "Blue",
  "title": "Final Showcase of Server Analytics",
  "title_position": "center",
  "footer": "End of Comprehensive Server Report - IT Team $(date +%Y)",
  "footer_position": "center",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "min"
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
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "right",
      "summary": "unique"
    },
    {
      "header": "CPU Usage",
      "key": "cpu_usage",
      "datatype": "kcpu",
      "justification": "center",
      "summary": "max"
    },
    {
      "header": "Bandwidth",
      "key": "bandwidth_mbps",
      "datatype": "float",
      "justification": "right",
      "summary": "sum"
    }
  ]
}
EOF

echo -e "\nTestC 9-T: Final showcase with diverse justifications and summaries"
echo "----------------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-U: Dual dynamic commands in footer
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "Dynamic Dual Command Footer Test",
  "title_position": "center",
  "footer": "Generated on $(date +%Y-%m-%d) at $(uptime | cut -d' ' -f2-3)",
  "footer_position": "center",
  "columns": [
    {
      "header": "ID",
      "key": "id",
      "datatype": "int",
      "justification": "right",
      "summary": "count",
      "break": true
    },
    {
      "header": "Server Name",
      "key": "server_name",
      "datatype": "text",
      "justification": "left"
    },
    {
      "header": "Description",
      "key": "description",
      "datatype": "text",
      "width": 75,
      "wrap_mode": "wrap",
      "justification": "left"
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

echo -e "\nTestC 9-U: Dual dynamic commands in footer"
echo "----------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG

# TestC 9-V: Multiple double-wide emojis in title and footer
cat > "$layout_file" << 'EOF'
{
  "theme": "Red",
  "title": "🚀 Server Status 📊 Performance Report 🌟 $(date +%Y-%m-%d) 💻",
  "footer": "✅ Data Complete 🔥 Analysis Ready 📈 Generated $(date +%H:%M:%S) 🎯",
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
      "justification": "left"
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
    },
    {
      "header": "Status",
      "key": "status",
      "datatype": "text",
      "justification": "center"
    }
  ]
}
EOF

echo -e "\nTestC 9-V: Multiple double-wide emojis in title and footer"
echo "-------------------------------------------------------"
"$tables_script" "$layout_file" "$data_file" $DEBUG_FLAG