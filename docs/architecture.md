# Architecture

## Overview

LeonidaLauncher is a **PS5 payload** built with the **PS5SDK** (john-tornblom). It runs as a userspace ELF loaded by the Relapse elfldr on port 9021.

```
┌─────────────────────────────────────────────────────────────┐
│                    LeonidaLauncher                          │
├─────────────────────────────────────────────────────────────┤
│  payload_main(args)  ←  CRT entry (libps5sdk_crt.a)        │
│       │                                                   │
│       ▼                                                   │
│  sony_resolve_all()  ←  runtime dlsym via CRT's __dlsym  │
│       │                                                   │
│       ▼                                                   │
│  Main loop (60 Hz)                                        │
│  ├── update()   → input, menu state, audio, loading      │
│  └── draw()     → render_begin, skybox, menu, render_end │
└─────────────────────────────────────────────────────────────┘
```

## Module Map

| File | Responsibility |
|------|----------------|
| `main.c` | `payload_main`, frame loop, menu actions, loading screen |
| `menu.c` | Menu tree, navigation, selection animation, pause overlay |
| `render.c` | Video init, resolution probe, framebuffer mgmt, text/logo |
| `input.c` | `scePad` polling, vibration, light bar |
| `audio.c` | Callback mixer: synth theme + 4 SFX voices |
| `assets.c` | Skybox generation, neon palette cycling |
| `sony_resolve.c` | Runtime symbol resolution via CRT's `__dlsym` |

## Memory Layout (from ELF)

```
LOAD 0x00001000  R E  0x024ba4  (.text + .rodata + embedded assets)
LOAD 0x0001e000  RW   0x008010  (.data + .bss)
Entry: 0x1316c  (CRT _start)
```

- **~69 MB** total (assets dominate: landscape 33 MB, logo 18 MB, theme 16 MB)
- All assets in `.rodata` — no filesystem access at runtime

## Initialization Sequence

```c
payload_main(args) {
    // 1. CRT already resolved libkernel, libc, initialized __dlsym
    sony_resolve_all();          // dlopen + dlsym all Sony modules
    sceSystemServiceHideSplashScreen();
    sceUserServiceInitialize();
    scePadInit();
    sceAudioOutInit();
    
    // 2. App init
    render_init();               // Probe 4K→1080p→720p, alloc video mem
    audio_init();                // Open audio port, start callback
    assets_init();               // Generate skybox texture
    input_init(pad_handle);      // Set lightbar
    create_main_menu();
    
    // 3. Frame loop (~60 Hz)
    while (running) {
        update();  // input, menu, loading, audio
        draw();    // skybox + menu/logo
    }
}
```

## Resolution Probe

No `sceVideoOutGetDeviceInfo` in SDK → **probe by attempting buffer registration**:

```c
static bool probe_video_size(w, h) {
    // Allocate video memory (2 buffers, 2 MB aligned)
    // Attempt sceVideoOutRegisterBuffers(w, h)
    // Success = console accepts this resolution
}
Ladder: 3840×2160 → 1920×1080 → 1280×720
```

## Audio System

- **Callback mixer** (`audio_fill_buffer` @ 48 kHz, 1024 samples)
- **Theme**: 3 detuned sines (A2/E3/A3) + 100 BPM sub-bass pulse
- **SFX**: 4 voice pool with exponential decay
  - Navigate: 240→420 Hz sine sweep (55 ms)
  - Select: 440+880 Hz chord (110 ms)
  - Back: 420→240 Hz sine sweep (55 ms)
  - Error: 150 Hz square (220 ms)
- **Volumes**: `g_music_vol` / `g_sfx_vol` (0–32768)

## Input System

- `scePadRead` every frame → tracks pressed/released/held
- Vibration: `scePadSetVibration(handle, {valid, left, right})`
- Light bar: `scePadSetLightBar(handle, {r, g, b, option})`
- Colors: Main = `#FF1493` (deep pink), Submenu = `#8A2BE2` (violet)

## Rendering

- **Software framebuffer** (allocated via `sceKernelAllocateDirectMemory`)
- **Double-buffered** with vsync flip (`sceVideoOutSubmitFlip`)
- **Text**: Exo2-Bold XBM (16×16), rescaled by `ui_scale` (1x/2x)
- **Gradient text**: Per-row lerp between top/bottom ARGB colors
- **Skybox**: 512×512 tiled texture, vertical drift per frame

## Link Dependencies

| Library | Purpose |
|---------|---------|
| `libps5sdk_crt.a` | CRT: `_start`, `payload_main` entry, `__dlsym` |
| `libScePad.sprx` | Controller input, vibration, light bar |
| `libSceVideoOut.sprx` | Framebuffer, resolution, flip |
| `libSceAudioOut.sprx` | Audio output |
| `libSceUserService.sprx` | User ID for pad open |
| `libSceSystemService.sprx` | Hide splash screen |
| `libSceGnmDriver.sprx` | `sceGnmFlushGarlic` (liborbis2d compat) |
| `libSceNet.sprx` | Socket ops (unused here, resolved anyway) |

---

> The architecture is intentionally minimal — no dynamic linking at load time, all Sony symbols resolved at runtime via the CRT's `__dlsym` from the payload_args provided by the elfldr.