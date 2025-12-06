#!/usr/bin/env fish
# Check for missing #include <Utility/compat.h> in files that use GET_PATH_SEPARATOR()
# This script dynamically finds all files using the macro and validates includes

set -l workspace_root (cd (dirname (status -f))/..; pwd)
set -l missing_count 0
set -l found_count 0
set -l files ()

echo "=== Checking GET_PATH_SEPARATOR() macro includes ==="
echo ""
echo "Finding all files that use GET_PATH_SEPARATOR()..."

# Dynamically find all files that use GET_PATH_SEPARATOR()
# Search in C++ source and header files
set -l search_dirs "Core" "Generals" "GeneralsMD"

for dir in $search_dirs
    if test -d "$workspace_root/$dir"
        set files (grep -r "GET_PATH_SEPARATOR()" "$workspace_root/$dir" \
            --include="*.cpp" --include="*.h" --include="*.hpp" \
            -l 2>/dev/null | sort -u)
        
        for file in $files
            set -l relative_path (string replace "$workspace_root/" "" "$file")
            set -a files_list $relative_path
        end
    end
end

# Remove duplicates
set files_list (printf '%s\n' $files_list | sort -u)

echo "Found (count $files_list) files using GET_PATH_SEPARATOR()"
echo ""
echo "Checking includes:"
echo "---"

for file in $files_list
    set -l full_path "$workspace_root/$file"
    
    if test -f "$full_path"
        if grep -q '#include.*[<"]Utility/compat\.h[>"]' "$full_path"
            echo "✅ $file"
            set found_count (math $found_count + 1)
        else
            echo "❌ $file"
            set missing_count (math $missing_count + 1)
        end
    else
        echo "⚠️  $file (file not found)"
    end
end

echo ""
echo "=== Summary ==="
echo "Files with #include Utility/compat.h: $found_count"
echo "Files missing #include Utility/compat.h: $missing_count"

if test $missing_count -gt 0
    echo ""
    echo "Files that need #include <Utility/compat.h>:"
    for file in $files_list
        set -l full_path "$workspace_root/$file"
        if test -f "$full_path"
            if not grep -q '#include.*[<"]Utility/compat\.h[>"]' "$full_path"
                echo "  • $file"
            end
        end
    end
    exit 1
else
    echo "✅ All files have the required include!"
    exit 0
end
