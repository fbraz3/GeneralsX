#!/bin/bash

# Phase 48: Find unnecessary try-catch blocks
# Criteria: No actual throwing code inside try block

echo "Searching for potentially unnecessary try-catch blocks..."
echo ""

# Look for try-catch in game code (not tools)
for file in $(find GeneralsMD/Code/GameEngine GeneralsMD/Code/GameEngineDevice Generals/Code/GameEngine Generals/Code/GameEngineDevice Core/GameEngine -name "*.cpp" -type f 2>/dev/null | grep -v Tools); do
    if grep -q "try {" "$file"; then
        # Extract try-catch blocks
        grep -n "try {" "$file" | while read line; do
            line_num=$(echo "$line" | cut -d: -f1)
            # Get surrounding context (20 lines)
            context_start=$((line_num))
            context_end=$((line_num + 20))
            
            # Check if try block contains any known throwing operations
            context=$(sed -n "${context_start},${context_end}p" "$file")
            
            # Look for operations that throw exceptions
            has_throw=0
            has_iostream=0
            has_fstream=0
            has_new=0
            has_array_access=0
            
            if echo "$context" | grep -q "throw\|NEW\|new \|vector\[\|\.at(\|fopen\|ifstream\|ofstream"; then
                has_throw=1
            fi
            
            # Report only if try block looks unnecessary (no throwing operations)
            if [ $has_throw -eq 0 ]; then
                echo "File: $file (line $line_num)"
                echo "Context:"
                sed -n "${context_start},${context_end}p" "$file" | head -15
                echo "---"
            fi
        done
    fi
done | head -100
