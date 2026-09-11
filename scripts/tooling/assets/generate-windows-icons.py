#!/usr/bin/env python3
"""
Generate multi-resolution Windows ICO files from GeneralsX source PNG assets.

Usage:
    python3 scripts/tooling/assets/generate-windows-icons.py
"""

import os
import struct
import subprocess
import tempfile

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))

DEFAULT_SIZES = [16, 24, 32, 48, 64, 128, 256]

def make_ico(src_png: str, dst_ico: str, sizes=DEFAULT_SIZES) -> None:
    if not os.path.exists(src_png):
        raise FileNotFoundError(f"Source PNG not found: {src_png}")

    os.makedirs(os.path.dirname(dst_ico), exist_ok=True)

    with tempfile.TemporaryDirectory() as tmpdir:
        png_data_list = []
        for s in sizes:
            out_png = os.path.join(tmpdir, f"icon_{s}.png")
            res = subprocess.run(
                ["sips", "-z", str(s), str(s), src_png, "--out", out_png],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
            if res.returncode != 0:
                subprocess.run(
                    ["ffmpeg", "-y", "-i", src_png, "-vf", f"scale={s}:{s}", out_png],
                    check=True,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )

            with open(out_png, "rb") as f:
                png_data_list.append((s, f.read()))

        count = len(png_data_list)
        header = struct.pack("<HHH", 0, 1, count)

        current_offset = 6 + count * 16
        entries = bytearray()
        images = bytearray()

        for s, data in png_data_list:
            w = 0 if s == 256 else s
            h = 0 if s == 256 else s
            entry = struct.pack("<BBBBHHII", w, h, 0, 0, 1, 32, len(data), current_offset)
            entries.extend(entry)
            images.extend(data)
            current_offset += len(data)

        with open(dst_ico, "wb") as f:
            f.write(header)
            f.write(entries)
            f.write(images)

    print(f"Generated {dst_ico} ({os.path.getsize(dst_ico)} bytes)")


def main():
    base_src = os.path.join(PROJECT_ROOT, "assets", "generalsx_icon.png")
    base_dst = os.path.join(PROJECT_ROOT, "Generals", "Code", "Main", "Generals.ico")

    zh_src = os.path.join(PROJECT_ROOT, "assets", "generalsx-zh_icon.png")
    zh_dst = os.path.join(PROJECT_ROOT, "GeneralsMD", "Code", "Main", "Generals.ico")

    make_ico(base_src, base_dst)
    make_ico(zh_src, zh_dst)


if __name__ == "__main__":
    main()
