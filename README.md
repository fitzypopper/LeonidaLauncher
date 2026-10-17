# LeonidaLauncher

A **GTA VI main menu replica** for PS5 — built with the **PS5SDK** (john-tornblom/PS5SDK) and loaded via the **Relapse exploit** on jailbroken PS5 (FW 7.00–13.60).

> ⚠️ **Fan project.** Not affiliated with Rockstar Games or Take-Two Interactive.  
> No game code or proprietary assets — everything embedded is procedurally generated or converted from public sources.

---

## Status

| Component | Status |
|-----------|--------|
| PS5SDK + CRT | ✅ Built from source (`~/PS5SDK`) |
| Embedded assets | ✅ Landscape (3840×2160), Logo (2160×2160), Theme (1:28 PCM16) |
| ELF build | ✅ 69 MB, Relapse-ready |
| .ffpkg package | ✅ Created (flat fake pkg) |
| Console test | ⏳ Pending — Relapse → port 9021 |

---

## Features

| Feature | Details |
|---------|---------|
| **Main menu** | Story Mode, GTA Online, Settings, Extras, Quit |
| **Pause menu** | Resume, Settings, Extras, Quit to Title (Options) |
| **Submenus** | Audio, Video, Controls, Gallery, Credits, Legal |
| **Resolution** | Auto-probe: 4K → 1080p → 720p (first accepted wins) |
| **Visuals** | Neon gradient logo, gold accents, animated Vice City skybox with palm silhouettes |
| **Font** | Exo2-Bold XBM bitmap (rescaled at runtime for 4K crispness) |
| **Haptics** | DualSense vibration on navigate/select/back; light bar shifts (pink → violet) |
| **Audio** | Procedural synthwave theme (A2/E3/A3 detuned + 100 BPM sub) + 4 SFX voices |
| **Assets** | Zero external files — landscape/logo/theme embedded in `.rodata` |

---

## Quick Start

```bash
# 1. Install PS5SDK (one-time)
git clone https://github.com/john-tornblom/PS5SDK ~/PS5SDK
export PS5SDK=~/PS5SDK
export PS5SDK_FW=0x451  # FW 13.40 = 0x451
cd ~/PS5SDK && ./build.sh

# 2. Build LeonidaLauncher
export PS5SDK=~/PS5SDK
export PS5SDK_FW=0x451
cd LeonidaLauncher
cmake -B build -G Ninja
ninja -C build

# 3. Output: build/bin/LeonidaLauncher (ELF) + LeonidaLauncher.ffpkg
```

---

## Run on PS5 (Relapse)

```bash
# 1. On PC — host the exploit
cd Relapse-Exploit
python3 serve.py

# 2. On PS5 — open hidden browser → http://<PC_IP>:8080
#    Wait for "ELF loader listening on port 9021"

# 3. On PC — send the payload
nc <PS5_IP> 9021 < build/bin/LeonidaLauncher
```

> **Alternative:** copy `build/bin/LeonidaLauncher.ffpkg` to USB and install via an .ffpkg-compatible homebrew loader.

---

## Controls

| Button | Action |
|--------|--------|
| D-Pad Up/Down | Navigate (subtle buzz) |
| Cross (✕) | Select / enter submenu (firm confirm) |
| Circle (○) | Back / cancel fake load (medium buzz) |
| Options | Open / close pause menu |
| L1 / R1 | Reserved (soft blip) |

---

## Embedded Assets

| Asset | Source | Format |
|-------|--------|--------|
| **Vice City Landscape** | Rockstar artwork page | 3840×2160 RGBA8888 |
| **GTA VI Logo** | Your `/tmp/19382386...jpeg` | 2160×2160 RGBA8888 |
| **Theme Music** | YouTube `aonTwBM1AZM` (yt-dlp → WAV → PCM16) | 48 kHz stereo, 1:28 |

All converted offline → raw binary → embedded in `.rodata` via `objcopy`. Zero runtime decoding.

---

## Project Structure

```
LeonidaLauncher/
├── README.md
├── CMakeLists.txt
├── linker.x
├── make_ffpkg.py          # .ffpkg creator
├── assets/
│   └── raw/               # Source assets (gitignored)
├── assets/raw/embed/      # objcopy output (gitignored)
├── build/                 # Build output (gitignored)
├── include/
│   ├── app_state.h
│   ├── assets.h
│   ├── audio.h
│   ├── input.h
│   ├── menu.h
│   ├── render.h
│   └── sony_resolve.h
└── src/
    ├── main.c             # payload_main, frame loop, menu actions
    ├── menu.c             # Menu state, navigation, rendering
    ├── render.c           # Video init, resolution probe, text/logo
    ├── input.c            # scePad polling, vibration, light bar
    ├── audio.c            # Callback mixer: theme + SFX
    ├── assets.c           # Skybox generation, neon palette
    └── sony_resolve.c     # Runtime dlsym via CRT's __dlsym
```

---

## Credits

- **PS5SDK** — john-tornblom, PS5 payload SDK
- **Relapse Exploit** — ntfargo, Sonic_Iso, Jordy, ufm42, Dr. Yenyen
- **GTA VI Branding** — Rockstar Games (referenced for accuracy)
- **Font** — Exo2-Bold XBM from liborbisXbmFont (rescaled at runtime)
- **Assets** — Rockstar artwork page (landscape), YouTube (theme), user-provided (logo)

---

## License

MIT — see [LICENSE](LICENSE).

> Educational/research only. No piracy, unauthorized access, or misuse. Use only on devices you own or are authorized to test.