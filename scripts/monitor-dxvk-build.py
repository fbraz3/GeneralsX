#!/usr/bin/env python3
"""Monitor DXVK build progress"""
import os
import time
import subprocess

log_file = "/Users/felipebraz/PhpstormProjects/pessoal/GeneralsX/logs/build_dxvk_macos.log"
max_wait = 60  # minutes

for minute in range(1, max_wait + 1):
    time.sleep(60)
    
    # Check if build done
    try:
        with open(log_file, 'r') as f:
            content = f.read()
            if 'BUILD_DXVK_DONE' in content:
                print("✅ DXVK BUILD COMPLETED!")
                print("=== Final lines ===")
                lines = content.split('\n')
                for line in lines[-10:]:
                    if line.strip():
                        print(line)
                exit(0)
    except:
        pass
    
    # Status update every 6 minutes
    if minute % 6 == 0:
        try:
            size = os.path.getsize(log_file)
            with open(log_file, 'r') as f:
                lines_count = len(f.readlines())
            
            print(f"⏳ DXVK Building... {minute}min ({lines_count} lines, {size} bytes)")
            
            # Check for patch 13
            with open(log_file, 'r') as f:
                if 'Patch 13' in f.read():
                    print("   ✓ Patch 13 detected!")
        except:
            pass

print(f"⚠️ Build did not complete after {max_wait} minutes")
