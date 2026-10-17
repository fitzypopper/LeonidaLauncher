#ifndef SONY_RESOLVE_H
#define SONY_RESOLVE_H

#include <stdint.h>

// libScePad
extern int (*scePadInit)(void);
extern int (*scePadOpen)(int userId, int type, int index, void *param);
extern int (*scePadClose)(int handle);
extern int (*scePadRead)(int handle, void *data, int count);
extern int (*scePadReadState)(int handle, void *data);

// Vibration and light bar
typedef struct ScePadVibrationParam {
    uint8_t valid;
    uint8_t motorLeft;
    uint8_t motorRight;
} ScePadVibrationParam;

typedef struct ScePadLightBarParam {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t option;
} ScePadLightBarParam;

extern int (*scePadSetVibration)(int handle, const ScePadVibrationParam *param);
extern int (*scePadSetLightBar)(int handle, const ScePadLightBarParam *param);

// libSceVideoOut
typedef struct SceVideoOutBufferAttribute {
    int32_t format;
    int32_t tmode;
    int32_t aspect;
    uint32_t width;
    uint32_t height;
    uint32_t pixelPitch;
    uint64_t reserved[2];
} SceVideoOutBufferAttribute;

typedef struct SceVideoOutFlipStatus {
    uint64_t num;
    uint64_t ptime;
    uint64_t stime;
    int64_t flipArg;
    uint64_t reserved[2];
    int32_t numGpuFlipPending;
    int32_t numFlipPending;
    int32_t currentBuffer;
    uint32_t reserved1;
} SceVideoOutFlipStatus;

typedef struct SceVideoOutDeviceInfo {
    int displayWidth;
    int displayHeight;
    uint32_t pixelFormat;
} SceVideoOutDeviceInfo;

extern int (*sceVideoOutOpen)(int userId, int type, int index, const void *param);
extern int (*sceVideoOutClose)(int handle);
extern int (*sceVideoOutRegisterBuffers)(int handle, int initialIndex, void *const *addr, int numBuf, const SceVideoOutBufferAttribute *attr);
extern int (*sceVideoOutUnregisterBuffers)(int handle, int indexAttr);
extern int (*sceVideoOutSubmitFlip)(int handle, int indexBuf, unsigned int flipMode, int64_t flipArg);
extern void (*sceVideoOutSetBufferAttribute)(SceVideoOutBufferAttribute *attr, unsigned int format, unsigned int tmode, unsigned int aspect, unsigned int width, unsigned int height, unsigned int pixelPitch);
extern int (*sceVideoOutSetFlipRate)(int handle, int flipRate);
extern int (*sceVideoOutAddFlipEvent)(int eq, int handle, void *data);
extern int (*sceVideoOutGetFlipStatus)(int handle, SceVideoOutFlipStatus *status);
extern int (*sceVideoOutGetDeviceInfo)(int handle, SceVideoOutDeviceInfo *info);

// libSceAudioOut
extern int (*sceAudioOutInit)(void);
extern int (*sceAudioOutOpen)(int portType, int index, int channelCount, int sampleRate, int bitsPerSample, int bufferSize, void *param);
extern int (*sceAudioOutClose)(int portType, int index);
extern int (*sceAudioOutOutput)(int portType, int index, const void *data, int dataSize);

// libSceUserService
extern int (*sceUserServiceInitialize)(void);
extern int (*sceUserServiceGetInitialUser)(int *userId);

// libSceSystemService
extern int (*sceSystemServiceHideSplashScreen)(void);

// libSceNet
extern int (*sceNetSocket)(int domain, int type, int protocol);
extern int (*sceNetHtons)(uint16_t hostshort);
extern int (*sceNetInetPton)(int af, const char *src, void *dst);

// libSceGnmDriver
extern void (*sceGnmFlushGarlic)(void);

// Initialization
int sony_resolve_all(void);
void sony_resolve_cleanup(void);

#endif // SONY_RESOLVE_H