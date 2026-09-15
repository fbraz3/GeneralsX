#!/usr/bin/env python3
"""
Generate multi-resolution Windows .ico files from source PNG icon assets.

Supported resolutions: 16x16, 24x24, 32x32, 48x48, 64x64, 128x128, 256x256.
Preserves transparency and 32-bit RGBA color planes.

Usage:
    python3 scripts/tooling/generate_windows_icons.py
"""

import os
import shutil
import struct
import subprocess
import sys
import tempfile

ICON_SIZES = [16, 24, 32, 48, 64, 128, 256]

def resize_png(input_png: str, output_png: str, size: int):
    """Resize input PNG to size x size preserving alpha."""
    # Method 1: macOS sips
    if shutil.which("sips"):
        cmd = ["sips", "-z", str(size), str(size), input_png, "--out", output_png]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if res.returncode == 0:
            return
            
    # Method 2: PIL
    try:
        from PIL import Image
        with Image.open(input_png) as img:
            resized = img.resize((size, size), Image.Resampling.LANCZOS)
            resized.save(output_png, format="PNG")
            return
    except ImportError:
        pass

    # Method 3: ImageMagick
    for binary in ["magick", "convert"]:
        if shutil.which(binary):
            cmd = [binary, input_png, "-resize", f"{size}x{size}", output_png]
            res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            if res.returncode == 0:
                return

    raise RuntimeError(f"No suitable image resizing tool found (checked sips, PIL, magick, convert)")

def build_ico(png_path: str, ico_path: str):
    """Convert a PNG file into a multi-resolution ICO file."""
    if not os.path.isfile(png_path):
        raise FileNotFoundError(f"Source PNG not found: {png_path}")

    with tempfile.TemporaryDirectory() as tmpdir:
        png_entries = []
        for s in ICON_SIZES:
            tmp_png = os.path.join(tmpdir, f"icon_{s}.png")
            resize_png(png_path, tmp_png, s)
            with open(tmp_png, "rb") as f:
                png_entries.append((s, f.read()))

        # ICO Header: idReserved (0), idType (1 = icon), idCount
        header = struct.pack("<HHH", 0, 1, len(png_entries))
        offset = 6 + len(png_entries) * 16

        dir_entries = bytearray()
        image_data = bytearray()

        for size, data in png_entries:
            w = 0 if size == 256 else size
            h = 0 if size == 256 else size
            data_len = len(data)
            # bWidth, bHeight, bColorCount, bReserved, wPlanes, wBitCount, dwBytesInRes, dwImageOffset
            entry = struct.pack("<BBBBHHII", w, h, 0, 0, 1, 32, data_len, offset + len(image_data))
            dir_entries.extend(entry)
            image_data.extend(data)

        os.makedirs(os.path.dirname(os.path.abspath(ico_path)), exist_ok=True)
        with open(ico_path, "wb") as f:
            f.write(header)
            f.write(dir_entries)
            f.write(image_data)

    print(f"✅ Generated: {ico_path} ({os.path.getsize(ico_path):,} bytes, {len(png_entries)} resolutions)")

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, "..", ".."))

    targets = [
        (
            os.path.join(project_root, "assets", "generalsx_icon.png"),
            os.path.join(project_root, "Generals", "Code", "Main", "Generals.ico"),
        ),
        (
            os.path.join(project_root, "assets", "generalsx-zh_icon.png"),
            os.path.join(project_root, "GeneralsMD", "Code", "Main", "Generals.ico"),
        ),
    ]

    for src, dst in targets:
        print(f"Processing {os.path.relpath(src, project_root)} -> {os.path.relpath(dst, project_root)}...")
        build_ico(src, dst)

if __name__ == "__main__":
    main()
