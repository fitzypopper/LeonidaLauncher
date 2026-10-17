# PS5SDK Toolchain Setup

## Prerequisites

```bash
sudo apt-get update && sudo apt-get install -y \
  git cmake ninja-build clang llvm lld \
  python3 python3-pip python3-venv
```

---

## PS5SDK Install

```bash
# Clone SDK
git clone https://github.com/john-tornblom/PS5SDK ~/PS5SDK
cd ~/PS5SDK

# Build SDK (CRT + libc + libkernel + samples)
./build.sh

# Verify
ls ~/PS5SDK/lib/libps5sdk_crt.a
# Should exist
```

### Firmware Version

Set `PS5SDK_FW` to match your console:

| FW | Hex | Notes |
|----|-----|-------|
| 9.00 | 0x900 | |
| 10.00 | 0xA00 | |
| 11.00 | 0xB00 | |
| 12.00 | 0xC00 | |
| 13.00 | 0xD00 | |
| 13.40 | **0x451** | **Used here** |

```bash
export PS5SDK=~/PS5SDK
export PS5SDK_FW=0x451
```

---

## Build LeonidaLauncher

```bash
export PS5SDK=~/PS5SDK
export PS5SDK_FW=0x451

cd LeonidaLauncher
cmake -B build -G Ninja
ninja -C build

# Output:
# build/bin/LeonidaLauncher      (ELF, ~69 MB)
# build/bin/LeonidaLauncher.ffpkg (flat fake pkg)
```

---

## Relapse Exploit (Port 9021)

```bash
# 1. Clone Relapse
git clone https://github.com/ntfargo/Relapse-Exploit
cd Relapse-Exploit

# 2. Serve exploit page
python3 serve.py
# Serves on http://0.0.0.0:8080

# 3. On PS5: open hidden browser → http://<PC_IP>:8080
#    Wait for "ELF loader listening on port 9021"

# 4. Send payload
nc <PS5_IP> 9021 < build/bin/LeonidaLauncher
```

### Relapse Tips

- **DNS**: Set PS5 primary DNS to `45.56.67.85` (Relapse host) for online use
- **Retries**: WebKit may stall; reload page if stuck
- **Kernel panic**: Reboot PS5 and retry
- **Port 9021**: ELF loader only appears after successful kernel exploit

---

## .ffpkg Creation (Optional)

```bash
python3 make_ffpkg.py build/bin/LeonidaLauncher build/bin/LeonidaLauncher.ffpkg
```

Result: `LeonidaLauncher.ffpkg` — installable via .ffpkg-compatible homebrew launchers.

---

## Common Build Errors

| Error | Fix |
|-------|-----|
| `PS5SDK environment variable not set` | `export PS5SDK=~/PS5SDK` |
| `socklen_t` unknown | Already patched in `sony_resolve.c` |
| `undefined symbol: dlsym` | Use CRT's `__dlsym` (see `sony_resolve.c`) |
| `undefined symbol: sceAudioOutInit` | Call `sony_resolve_all()` before use |
| `socklen_t` in libc.h | Already patched with `typedef __socklen_t socklen_t` |

---

## Directory Layout After Build

```
LeonidaLauncher/
├── build/
│   ├── bin/
│   │   ├── LeonidaLauncher      # 69 MB ELF
│   │   └── LeonidaLauncher.ffpkg
│   └── CMakeFiles/...
├── assets/
│   ├── raw/                     # Source assets (gitignored)
│   │   ├── logo.rgba
│   │   ├── landscape.rgba
│   │   └── theme.pcm
│   └── embed/                   # objcopy output (gitignored)
│       ├── logo.o
│       ├── landscape.o
│       └── theme.o
├── build/                       # CMake build dir
├── include/
├── src/
└── docs/
```

---

## Asset Pipeline (One-time)

```bash
# Logo (your file)
ffmpeg -i /tmp/your-logo.jpeg -pix_fmt rgba -f rawvideo assets/raw/logo.rgba

# Landscape (Rockstar URL)
ffmpeg -i "https://www.rockstargames.com/VI/...landscape.jpg" \
  -pix_fmt rgba -f rawvideo assets/raw/landscape.rgba

# Theme (YouTube → WAV → PCM16)
yt-dlp -x --audio-format wav "https://youtube.com/watch?v=..." -o assets/theme.wav
ffmpeg -i assets/theme.wav -ar 48000 -ac 2 -f s16le assets/raw/theme.pcm

# Embed via objcopy
cd assets/raw
mkdir -p embed
objcopy -I binary -O elf64-x86-64 --binary-architecture i386:x86-64 \
  --rename-section .data=.rodata.logo \
  logo.rgba embed/logo.o
# ... repeat for landscape, theme
```

Then rebuild: `ninja -C build`