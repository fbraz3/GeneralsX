#!/usr/bin/env python3
import struct
import sys
import os

def parse_crc_bin(filepath):
    with open(filepath, 'rb') as f:
        data = f.read()

    # Read OS header if present (null terminated string followed by 4 byte length)
    header_len = 0
    os_info = ""
    # Look for the first 0x00 which terminates the OS string
    null_idx = data.find(b'\x00')
    if null_idx != -1 and null_idx < 128:
        os_info = data[:null_idx].decode('utf-8', errors='ignore')
        header_len = null_idx + 1

    print(f"--- Desync Dump: {os.path.basename(filepath)} ---")
    if os_info:
        print(f"OS/Arch: {os_info}")
    
    # Locate "MARKER:Objects\x00"
    marker = b"MARKER:Objects\x00"
    marker_pos = data.find(marker)
    
    if marker_pos == -1:
        print("Error: Could not find MARKER:Objects in the dump.")
        return

    objects_data = data[marker_pos + len(marker):]
    print(f"\nFound MARKER:Objects at offset {marker_pos}")
    print(f"Objects data size: {len(objects_data)} bytes")

    # Read first object (Assuming it caused the desync)
    # The format is:
    # xferVersion (4 bytes, little endian uint32)
    # objectID (4 bytes, little endian uint32)
    # chunk of data
    
    if len(objects_data) >= 8:
        version = struct.unpack('<I', objects_data[0:4])[0]
        obj_id = struct.unpack('<I', objects_data[4:8])[0]
        print(f"\nFirst Object in buffer:")
        print(f"  Version:   {version}")
        print(f"  Object ID: {obj_id} (Hex: 0x{obj_id:08X})")
        
        # If version >= 7, next 48 bytes are Matrix3D (3x4 floats)
        if version >= 7 and len(objects_data) >= 8 + 48:
            print(f"  Transform Matrix (Matrix3D):")
            matrix_bytes = objects_data[8:8+48]
            floats = struct.unpack('<12f', matrix_bytes)
            print(f"    Row 0: [{floats[0]:.6f}, {floats[1]:.6f}, {floats[2]:.6f}, {floats[3]:.6f}]")
            print(f"    Row 1: [{floats[4]:.6f}, {floats[5]:.6f}, {floats[6]:.6f}, {floats[7]:.6f}]")
            print(f"    Row 2: [{floats[8]:.6f}, {floats[9]:.6f}, {floats[10]:.6f}, {floats[11]:.6f}]")

    print("\n-------------------------------------------------\n")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: ./parse_deep_crc.py <path_to_bin_file> [path_to_another_bin_file ...]")
        sys.exit(1)
        
    for file in sys.argv[1:]:
        parse_crc_bin(file)
