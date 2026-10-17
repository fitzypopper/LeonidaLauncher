/*
 * Sony library resolver - uses the CRT's __dlsym (from payload_args)
 * to resolve Sony library symbols at runtime.
 */

#include "sony_resolve.h"

#include <ps5/libkernel.h>

// Define socklen_t before libc.h (PS5SDK uses __socklen_t)
typedef __socklen_t socklen_t;
#include <ps5/libc.h>
#include <stdio.h>

// CRT's dlsym function pointer (set by CRT from payload_args before payload_main)
extern int (*__dlsym)(int module, const char *symbol, void *addr);

// Module handles (obtained via sceKernelLoadStartModule)
static void *g_scePad = NULL;
static void *g_sceVideoOut = NULL;
static void *g_sceAudioOut = NULL;
static void *g_sceUserService = NULL;
static void *g_sceSystemService = NULL;
static void *g_sceNet = NULL;
static void *g_sceGnmDriver = NULL;

// Function pointer definitions (storage)
int (*scePadInit)(void);
int (*scePadOpen)(int userId, int type, int index, void *param);
int (*scePadClose)(int handle);
int (*scePadRead)(int handle, void *data, int count);
int (*scePadReadState)(int handle, void *data);
int (*scePadSetVibration)(int handle, const ScePadVibrationParam *param);
int (*scePadSetLightBar)(int handle, const ScePadLightBarParam *param);

int (*sceVideoOutOpen)(int userId, int type, int index, const void *param);
int (*sceVideoOutClose)(int handle);
int (*sceVideoOutRegisterBuffers)(int handle, int initialIndex, void *const *addr, int numBuf, const SceVideoOutBufferAttribute *attr);
int (*sceVideoOutUnregisterBuffers)(int handle, int indexAttr);
int (*sceVideoOutSubmitFlip)(int handle, int indexBuf, unsigned int flipMode, int64_t flipArg);
void (*sceVideoOutSetBufferAttribute)(SceVideoOutBufferAttribute *attr, unsigned int format, unsigned int tmode, unsigned int aspect, unsigned int width, unsigned int height, unsigned int pixelPitch);
int (*sceVideoOutSetFlipRate)(int handle, int flipRate);
int (*sceVideoOutAddFlipEvent)(int eq, int handle, void *data);
int (*sceVideoOutGetFlipStatus)(int handle, SceVideoOutFlipStatus *status);
int (*sceVideoOutGetDeviceInfo)(int handle, SceVideoOutDeviceInfo *info);

int (*sceAudioOutInit)(void);
int (*sceAudioOutOpen)(int portType, int index, int channelCount, int sampleRate, int bitsPerSample, int bufferSize, void *param);
int (*sceAudioOutClose)(int portType, int index);
int (*sceAudioOutOutput)(int portType, int index, const void *data, int dataSize);

int (*sceUserServiceInitialize)(void);
int (*sceUserServiceGetInitialUser)(int *userId);

int (*sceSystemServiceHideSplashScreen)(void);

int (*sceNetSocket)(int domain, int type, int protocol);
int (*sceNetHtons)(uint16_t hostshort);
int (*sceNetInetPton)(int af, const char *src, void *dst);

void (*sceGnmFlushGarlic)(void);

// Helper: resolve a symbol using the CRT's __dlsym
static int resolve_sym(int module, const char *name, void *out_ptr)
{
    if (!__dlsym) {
        printf("[SONY] __dlsym not initialized\n");
        return -1;
    }
    int ret = __dlsym(module, name, out_ptr);
    if (ret != 0) {
        printf("[SONY] Failed to resolve %s (module %d, ret=%d)\n", name, module, ret);
        return -1;
    }
    return 0;
}

// Load a module and get its module ID
static int load_module(const char *name)
{
    int mod = 0;
    int ret = sceKernelLoadStartModule(name, 0, 0, 0, 0, &mod);
    if (ret != 0) {
        printf("[SONY] sceKernelLoadStartModule(%s) failed: 0x%x\n", name, ret);
        return -1;
    }
    printf("[SONY] Loaded %s, module ID=%d\n", name, mod);
    return mod;
}

#define RESOLVE(mod, name, funcptr) \
    do { \
        if (resolve_sym(mod, name, (void*)&funcptr) != 0) return -1; \
    } while (0)

int sony_resolve_all(void)
{
    // Load Sony libraries
    int M_scePad         = load_module("libScePad.sprx");
    int M_sceVideoOut    = load_module("libSceVideoOut.sprx");
    int M_sceAudioOut    = load_module("libSceAudioOut.sprx");
    int M_sceUserService = load_module("libSceUserService.sprx");
    int M_sceSystemService = load_module("libSceSystemService.sprx");
    int M_sceNet         = load_module("libSceNet.sprx");
    int M_sceGnmDriver   = load_module("libSceGnmDriver.sprx");

    if (M_scePad < 0 || M_sceVideoOut < 0 || M_sceAudioOut < 0 ||
        M_sceUserService < 0 || M_sceSystemService < 0 || M_sceNet < 0 || M_sceGnmDriver < 0) {
        return -1;
    }

    // Resolve libScePad
    RESOLVE(M_scePad, "scePadInit", scePadInit);
    RESOLVE(M_scePad, "scePadOpen", scePadOpen);
    RESOLVE(M_scePad, "scePadClose", scePadClose);
    RESOLVE(M_scePad, "scePadRead", scePadRead);
    RESOLVE(M_scePad, "scePadReadState", scePadReadState);
    RESOLVE(M_scePad, "scePadSetVibration", scePadSetVibration);
    RESOLVE(M_scePad, "scePadSetLightBar", scePadSetLightBar);

    // Resolve libSceVideoOut
    RESOLVE(M_sceVideoOut, "sceVideoOutOpen", sceVideoOutOpen);
    RESOLVE(M_sceVideoOut, "sceVideoOutClose", sceVideoOutClose);
    RESOLVE(M_sceVideoOut, "sceVideoOutRegisterBuffers", sceVideoOutRegisterBuffers);
    RESOLVE(M_sceVideoOut, "sceVideoOutUnregisterBuffers", sceVideoOutUnregisterBuffers);
    RESOLVE(M_sceVideoOut, "sceVideoOutSubmitFlip", sceVideoOutSubmitFlip);
    RESOLVE(M_sceVideoOut, "sceVideoOutSetBufferAttribute", sceVideoOutSetBufferAttribute);
    RESOLVE(M_sceVideoOut, "sceVideoOutSetFlipRate", sceVideoOutSetFlipRate);
    RESOLVE(M_sceVideoOut, "sceVideoOutAddFlipEvent", sceVideoOutAddFlipEvent);
    RESOLVE(M_sceVideoOut, "sceVideoOutGetFlipStatus", sceVideoOutGetFlipStatus);
    RESOLVE(M_sceVideoOut, "sceVideoOutGetDeviceInfo", sceVideoOutGetDeviceInfo);

    // Resolve libSceAudioOut
    RESOLVE(M_sceAudioOut, "sceAudioOutInit", sceAudioOutInit);
    RESOLVE(M_sceAudioOut, "sceAudioOutOpen", sceAudioOutOpen);
    RESOLVE(M_sceAudioOut, "sceAudioOutClose", sceAudioOutClose);
    RESOLVE(M_sceAudioOut, "sceAudioOutOutput", sceAudioOutOutput);

    // Resolve libSceUserService
    RESOLVE(M_sceUserService, "sceUserServiceInitialize", sceUserServiceInitialize);
    RESOLVE(M_sceUserService, "sceUserServiceGetInitialUser", sceUserServiceGetInitialUser);

    // Resolve libSceSystemService
    RESOLVE(M_sceSystemService, "sceSystemServiceHideSplashScreen", sceSystemServiceHideSplashScreen);

    // Resolve libSceNet
    RESOLVE(M_sceNet, "sceNetSocket", sceNetSocket);
    RESOLVE(M_sceNet, "sceNetHtons", sceNetHtons);
    RESOLVE(M_sceNet, "sceNetInetPton", sceNetInetPton);

    // Resolve libSceGnmDriver
    RESOLVE(M_sceGnmDriver, "sceGnmFlushGarlic", sceGnmFlushGarlic);

    printf("[SONY] All libraries resolved successfully\n");
    return 0;
}

void sony_resolve_cleanup(void)
{
    // Note: no dlclose available; modules stay loaded
    (void)g_scePad;
    (void)g_sceVideoOut;
    (void)g_sceAudioOut;
    (void)g_sceUserService;
    (void)g_sceSystemService;
    (void)g_sceNet;
    (void)g_sceGnmDriver;
}