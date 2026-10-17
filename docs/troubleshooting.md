# Troubleshooting

## Build Issues

### `PS5SDK environment variable not set`
```bash
export PS5SDK=~/PS5SDK
export PS5SDK_FW=0x451
```

### `socklen_t` / `inet_ntoa_r` errors
The PS5SDK headers expect `__socklen_t`. Already patched in `sony_resolve.c`:
```c
typedef __socklen_t socklen_t;
#include <ps5/libc.h>
```

### `undefined symbol: dlsym` / `dlclose`
Resolved via CRT's `__dlsym` (set by payload_args before `payload_main`). See `sony_resolve.c`:
```c
extern int (*__dlsym)(int module, const char *symbol, void *addr);
```

### Large file warnings on Git push
```bash
git lfs install
git lfs track "build/bin/*.elf" "build/bin/*.ffpkg"
git add .gitattributes
git commit -m "LFS tracking"
git push
```

---

## Runtime (PS5)

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| Black screen, no output | Resolution probe accepted 4K but display can't scan it | Rebuild with 4K removed from ladder in `render.c` |
| No sound | `sceAudioOutInit` failed or port busy | Close other apps; check `audio_init` return |
| No vibration/light bar | Pad handle invalid or user not logged in | Ensure `sceUserServiceGetInitialUser` succeeds |
| Immediate crash | `sceKernelLoadStartModule` failed for a module | Check `[SONY]` logs; module may not exist on FW |
| ELF rejected by Relapse | Wrong ELF format (not PS5 payload) | Must use PS5SDK CRT + payload_main entry |

---

## Debugging

### Enable CRT logs
The PS5SDK CRT prints to stdout. On Relapse, stdout may not be visible. Use a USB serial adapter or check `dmesg` via HEN.

### Resolution probe debug
In `render.c`, `probe_video_size()` prints:
```
[RENDER] 3840x2160 rejected by video out
[RENDER] 1920x1080 accepted
```

### Audio callback
`audio_update_music()` called every frame from main loop. If silent, check:
- `sceAudioOutOpen` returned success
- `audio_fill_buffer` produces non-zero samples
- `sceAudioOutOutput` called with correct buffer size

---

## Known Limitations

| Feature | Status |
|---------|--------|
| Adaptive triggers | ❌ Not in SDK (`scePadSetTriggerEffect` absent) |
| Touchpad | ❌ Not implemented |
| Motion sensor | ❌ Not implemented |
| AT9 audio | ❌ No encoder; using raw PCM16 |
| PNG/JPEG loading | ❌ No decoder; assets embedded as raw RGBA |
| Save data | ❌ Not implemented |
| Network play | ❌ Menu only |