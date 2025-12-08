#!/usr/bin/env fish
# Add #include <Utility/compat.h> to all files that use GET_PATH_SEPARATOR()
# but don't have the include yet

set -l workspace_root (cd (dirname (status -f))/..; pwd)
set -l updated_count 0
set -l skipped_count 0

# List of files that need the include added (from check_compat_includes.fish)
set -l files_to_fix \
    "Core/GameEngine/Source/GameNetwork/FileTransfer.cpp" \
    "Core/Tools/matchbot/generals.cpp" \
    "Core/Tools/matchbot/main.cpp" \
    "Core/Tools/matchbot/encrypt.cpp" \
    "Core/Tools/WW3D/max2w3d/util.cpp" \
    "Core/Tools/WW3D/max2w3d/w3dutil.cpp" \
    "Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp" \
    "Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/WorldHeightMap.cpp" \
    "Generals/Code/Libraries/Source/WWVegas/WW3D2/part_ldr.cpp" \
    "GeneralsMD/Code/Main/WinMain.cpp" \
    "GeneralsMD/Code/Tools/wdump/wdump.cpp" \
    "GeneralsMD/Code/GameEngine/Source/GameNetwork/GameSpyChat.cpp" \
    "GeneralsMD/Code/GameEngine/Source/GameClient/GameText.cpp" \
    "GeneralsMD/Code/GameEngine/Source/GameClient/MapUtil.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/MiniLog.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/Thing/ThingTemplate.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/INI/INIMapCache.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/StatsCollector.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/System/encrypt.cpp" \
    "GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp" \
    "GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindowManagerScript.cpp" \
    "GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp" \
    "GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/InGameChat.cpp" \
    "GeneralsMD/Code/GameEngine/Source/GameClient/Input/Keyboard.cpp"

echo "Adding #include <Utility/compat.h> to files that use GET_PATH_SEPARATOR()"
echo "=========================================================================="
echo ""

for file in $files_to_fix
    set -l full_path "$workspace_root/$file"
    
    if test -f "$full_path"
        # Check if the include already exists
        if grep -q '#include.*[<"]Utility/compat\.h[>"]' "$full_path"
            echo "⏭️  $file (already has include)"
            set skipped_count (math $skipped_count + 1)
        else
            # Find the first #include line to insert after it
            set -l first_include_line (grep -n '#include' "$full_path" | head -1 | cut -d':' -f1)
            
            if test -n "$first_include_line"
                # Insert the include after the first include
                sed -i "${first_include_line}a #include <Utility/compat.h>" "$full_path"
                echo "✅ $file (include added)"
                set updated_count (math $updated_count + 1)
            else
                # No includes found, this shouldn't happen
                echo "⚠️  $file (no includes found to insert after)"
            end
        end
    else
        echo "⚠️  $file (file not found)"
    end
end

echo ""
echo "=========================================================================="
echo "Summary: Updated: $updated_count | Skipped: $skipped_count"
echo "=========================================================================="
