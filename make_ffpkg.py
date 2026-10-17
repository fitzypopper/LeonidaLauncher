#!/usr/bin/env python3
"""
Quick .ffpkg (Flat Fake Package) creator for PS5 homebrew ELF.
Usage: python3 make_ffpkg.py <input.elf> <output.ffpkg> [title] [app_id]
"""

import struct
import os
import sys

def make_ffpkg(elf_path, out_path, title="GTA 6 Fake Menu", app_id="GTA6FAKE01"):
    """Create a flat fake package from an ELF."""
    
    with open(elf_path, 'rb') as f:
        elf_data = f.read()
    
    elf_size = len(elf_data)
    
    # Sanitize strings
    title = title[:127].encode('utf-8')
    app_id = app_id[:15].encode('ascii')
    
    # .ffpkg header (32 bytes)
    # Magic: "FPKG" (4 bytes)
    # Version: 1 (uint32)
    # Entry count: 1 (uint32) 
    # Reserved: 4 bytes
    header = struct.pack('<4sII4s', b'FPKG', 1, 1, b'\x00\x00\x00\x00')
    
    # Entry table (1 entry, 32 bytes each)
    # Filename: 16 bytes (null-padded)
    # Offset: uint64
    # Size: uint64
    entry_name = b'gta6_fake_menu.elf\x00'  # 16 bytes
    entry_name = entry_name[:16].ljust(16, b'\x00')
    entry_offset = 32 + 32  # header + entry table
    entry_size = elf_size
    entry = struct.pack('<16sQQ', entry_name, entry_offset, entry_size)
    
    # Write package
    with open(out_path, 'wb') as f:
        f.write(header)
        f.write(entry)
        f.write(elf_data)
    
    print(f"Created {out_path}: {os.path.getsize(out_path)} bytes")
    print(f"  ELF: {elf_size} bytes")
    print(f"  Title: {title.decode()}")
    print(f"  App ID: {app_id.decode()}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python3 make_ffpkg.py <input.elf> <output.ffpkg> [title] [app_id]")
        sys.exit(1)
    
    elf_path = sys.argv[1]
    out_path = sys.argv[2]
    title = sys.argv[3] if len(sys.argv) > 3 else "GTA 6 Fake Menu"
    app_id = sys.argv[4] if len(sys.argv) > 4 else "GTA6FAKE01"
    
    make_ffpkg(elf_path, out_path, title, app_id)