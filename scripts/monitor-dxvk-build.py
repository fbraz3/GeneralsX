#!/usr/bin/env python3
"""Monitor DXVK build progress.

Usage: monitor-dxvk-build.py [--log <path>] [--max-wait <minutes>]

Defaults:
  --log       logs/build_dxvk_macos.log  (relative to the repo root)
  --max-wait  60
"""
import argparse
import os
import sys
import time

def main():
    parser = argparse.ArgumentParser(description="Monitor DXVK build progress")
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.dirname(script_dir)
    default_log = os.path.join(repo_root, "logs", "build_dxvk_macos.log")
    parser.add_argument("--log", default=default_log,
                        help="Path to DXVK build log file")
    parser.add_argument("--max-wait", type=int, default=60,
                        help="Maximum minutes to wait (default: 60)")
    args = parser.parse_args()

    log_file = args.log
    max_wait = args.max_wait

    if not os.path.exists(log_file):
        print(f"ERROR: Log file not found: {log_file}", file=sys.stderr)
        sys.exit(1)

    for minute in range(1, max_wait + 1):
        time.sleep(60)

        # Check if build done
        try:
            with open(log_file, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read()
            if 'BUILD_DXVK_DONE' in content:
                print("DXVK BUILD COMPLETED!")
                print("=== Final lines ===")
                lines = content.split('\n')
                for line in lines[-10:]:
                    if line.strip():
                        print(line)
                sys.exit(0)
        except OSError as e:
            print(f"WARNING: Could not read log file: {e}", file=sys.stderr)
        except Exception as e:
            print(f"ERROR: Unexpected error reading log: {e}", file=sys.stderr)
            sys.exit(1)

        # Status update every 6 minutes
        if minute % 6 == 0:
            try:
                size = os.path.getsize(log_file)
                with open(log_file, 'r', encoding='utf-8', errors='replace') as f:
                    lines_count = len(f.readlines())
                print(f"DXVK Building... {minute}min ({lines_count} lines, {size} bytes)")
                with open(log_file, 'r', encoding='utf-8', errors='replace') as f:
                    if 'Patch 13' in f.read():
                        print("   Patch 13 detected!")
            except OSError as e:
                print(f"WARNING: Could not read log file: {e}", file=sys.stderr)

    print(f"WARNING: Build did not complete after {max_wait} minutes")
    sys.exit(1)

if __name__ == "__main__":
    main()
