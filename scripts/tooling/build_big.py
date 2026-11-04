#!/usr/bin/env python3
"""
GeneralsX .BIG Archive Utility
Creates, unpacks, and inspects EA BIG (BIGF) archives for Command & Conquer: Generals and Zero Hour.
"""

import os
import sys
import struct
import argparse
from typing import List, Tuple

BIGF_MAGIC = b"BIGF"


def align_to(offset: int, alignment: int = 4) -> int:
    remainder = offset % alignment
    if remainder == 0:
        return offset
    return offset + (alignment - remainder)


def pack_big(input_dir: str, output_path: str) -> None:
    input_dir = os.path.abspath(input_dir)
    if not os.path.isdir(input_dir):
        raise ValueError(f"Input directory does not exist: {input_dir}")

    # Gather all files
    files_to_pack: List[Tuple[str, str, int]] = []  # (rel_path_with_backslashes, full_path, size)
    for root, _, filenames in os.walk(input_dir):
        for filename in filenames:
            # Skip hidden/system files
            if filename.startswith("."):
                continue
            full_path = os.path.join(root, filename)
            rel_path = os.path.relpath(full_path, input_dir)
            # Use backslashes as expected by SAGE engine
            rel_path_win = rel_path.replace("/", "\\")
            size = os.path.getsize(full_path)
            files_to_pack.append((rel_path_win, full_path, size))

    # Sort files deterministically (case-insensitive)
    files_to_pack.sort(key=lambda x: x[0].lower())

    num_files = len(files_to_pack)

    # Compute header table size
    # Header format:
    # 0x00: "BIGF" (4 bytes)
    # 0x04: total archive size (4 bytes little-endian uint32)
    # 0x08: number of files (4 bytes big-endian uint32)
    # 0x0C: header size / first file offset (4 bytes big-endian uint32)
    # Directory entries start at 0x10 (16 bytes):
    # for each file:
    #   offset: 4 bytes big-endian uint32
    #   size: 4 bytes big-endian uint32
    #   path: len(path) + 1 (null terminator)
    header_entries_size = 0
    for rel_path, _, _ in files_to_pack:
        path_bytes = rel_path.encode("latin1", errors="replace")
        header_entries_size += 4 + 4 + len(path_bytes) + 1

    first_file_offset_raw = 16 + header_entries_size
    first_file_offset = align_to(first_file_offset_raw, 4)
    padding_header = first_file_offset - first_file_offset_raw

    # Assign offsets
    current_offset = first_file_offset
    offsets: List[int] = []
    for _, _, size in files_to_pack:
        offsets.append(current_offset)
        current_offset = align_to(current_offset + size, 4)

    total_archive_size = current_offset if files_to_pack else first_file_offset

    # Ensure target directory exists
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)

    with open(output_path, "wb") as out:
        # Write header
        out.write(BIGF_MAGIC)
        out.write(struct.pack("<I", total_archive_size))
        out.write(struct.pack(">I", num_files))
        out.write(struct.pack(">I", first_file_offset))

        # Write directory table
        for (rel_path, _, size), offset in zip(files_to_pack, offsets):
            out.write(struct.pack(">I", offset))
            out.write(struct.pack(">I", size))
            out.write(rel_path.encode("latin1", errors="replace") + b"\x00")

        if padding_header > 0:
            out.write(b"\x00" * padding_header)

        # Write file contents
        for (rel_path, full_path, size), offset in zip(files_to_pack, offsets):
            pad = offset - out.tell()
            if pad > 0:
                out.write(b"\x00" * pad)
            with open(full_path, "rb") as in_file:
                chunk = in_file.read()
                out.write(chunk)

    print(f"Packed {num_files} file(s) into {output_path} ({total_archive_size} bytes)")


def list_big(big_path: str) -> None:
    if not os.path.isfile(big_path):
        raise FileNotFoundError(f"File not found: {big_path}")

    with open(big_path, "rb") as f:
        magic = f.read(4)
        if magic != BIGF_MAGIC and magic != b"BIG4":
            raise ValueError(f"Invalid magic: {magic} (expected BIGF or BIG4)")

        archive_size = struct.unpack("<I", f.read(4))[0]
        num_files = struct.unpack(">I", f.read(4))[0]
        first_offset = struct.unpack(">I", f.read(4))[0]

        print(f"Archive: {big_path}")
        print(f"Magic: {magic.decode('latin1')}, Size: {archive_size} bytes, Files: {num_files}, Data Offset: {first_offset}")
        print("-" * 80)
        print(f"{'Offset':<12} {'Size':<10} {'Path'}")
        print("-" * 80)

        f.seek(16)
        for _ in range(num_files):
            offset = struct.unpack(">I", f.read(4))[0]
            size = struct.unpack(">I", f.read(4))[0]
            path_chars = bytearray()
            while True:
                b = f.read(1)
                if not b or b == b"\x00":
                    break
                path_chars.extend(b)
            path_str = path_chars.decode("latin1")
            print(f"{offset:<12} {size:<10} {path_str}")


def unpack_big(big_path: str, output_dir: str) -> None:
    if not os.path.isfile(big_path):
        raise FileNotFoundError(f"File not found: {big_path}")

    with open(big_path, "rb") as f:
        magic = f.read(4)
        if magic != BIGF_MAGIC and magic != b"BIG4":
            raise ValueError(f"Invalid magic: {magic} (expected BIGF or BIG4)")

        archive_size = struct.unpack("<I", f.read(4))[0]
        num_files = struct.unpack(">I", f.read(4))[0]
        first_offset = struct.unpack(">I", f.read(4))[0]

        f.seek(16)
        entries: List[Tuple[int, int, str]] = []
        for _ in range(num_files):
            offset = struct.unpack(">I", f.read(4))[0]
            size = struct.unpack(">I", f.read(4))[0]
            path_chars = bytearray()
            while True:
                b = f.read(1)
                if not b or b == b"\x00":
                    break
                path_chars.extend(b)
            entries.append((offset, size, path_chars.decode("latin1")))

        out_root = os.path.realpath(output_dir)
        for offset, size, rel_path in entries:
            # Normalize to current OS path separators
            norm_rel_path = rel_path.replace("\\", os.sep).replace("/", os.sep)
            dest_path = os.path.realpath(os.path.join(out_root, norm_rel_path))
            if os.path.commonpath([out_root, dest_path]) != out_root:
                raise ValueError(f"Unsafe path in archive: {rel_path}")
            os.makedirs(os.path.dirname(dest_path), exist_ok=True)
            f.seek(offset)
            data = f.read(size)
            with open(dest_path, "wb") as out:
                out.write(data)

    print(f"Unpacked {len(entries)} file(s) into {output_dir}")


def main():
    parser = argparse.ArgumentParser(description="GeneralsX .BIG Archive Utility")
    subparsers = parser.add_subparsers(dest="command", required=True)

    pack_parser = subparsers.add_parser("pack", help="Pack directory into a .BIG archive")
    pack_parser.add_argument("input_dir", help="Source directory to pack")
    pack_parser.add_argument("output_big", help="Output .big file path")

    list_parser = subparsers.add_parser("list", help="List contents of a .BIG archive")
    list_parser.add_argument("big_file", help="Path to .big file")

    unpack_parser = subparsers.add_parser("unpack", help="Unpack .BIG archive to directory")
    unpack_parser.add_argument("big_file", help="Path to .big file")
    unpack_parser.add_argument("output_dir", help="Output directory path")

    args = parser.parse_args()

    if args.command == "pack":
        pack_big(args.input_dir, args.output_big)
    elif args.command == "list":
        list_big(args.big_file)
    elif args.command == "unpack":
        unpack_big(args.big_file, args.output_dir)


if __name__ == "__main__":
    main()
