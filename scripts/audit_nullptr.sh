#!/bin/bash

# Phase 48 nullptr audit script
# Extracts nullptr returns with context for categorization

echo "=== Phase 48 nullptr Categorization Audit ==="
echo ""

# Categories
CRITICAL=0
STUB=0
LEGITIMATE=0
FUTURE=0

# Function to analyze file
analyze_file() {
    local file="$1"
    local count=$(grep -c "return nullptr" "$file" 2>/dev/null || echo 0)
    
    if [ "$count" -eq 0 ]; then
        return
    fi
    
    echo "FILE: $file ($count occurrences)"
    echo "---"
    
    # Extract lines with nullptr with context
    grep -B 2 -A 1 "return nullptr" "$file" | while read line; do
        if [[ "$line" == *"return nullptr"* ]]; then
            # Extract just the line
            echo "$line"
        elif [[ "$line" == "--" ]]; then
            echo ""
        elif [[ ! -z "$line" ]]; then
            echo "  $line"
        fi
    done
    
    echo ""
    echo "---"
    echo ""
}

# Analyze files with most nullptr (top 10 files)
files=$(find GeneralsMD Generals Core -name "*.cpp" -type f -exec grep -l "return nullptr" {} \; 2>/dev/null | while read f; do echo "$(grep -c 'return nullptr' "$f"):$f"; done | sort -rn | head -10 | cut -d: -f2-)

for file in $files; do
    analyze_file "$file"
done
