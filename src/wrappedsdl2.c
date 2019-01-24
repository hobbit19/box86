#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "library_private.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "sdl2rwops.h"
#include "myalign.h"

static int sdl_Yes() { return 1;}
static int sdl_No() { return 0;}
int EXPORT my2_SDL_Has3DNow() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_Has3DNowExt() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasAltiVec() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasMMX() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasMMXExt() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasNEON() __attribute__((alias("sdl_No")));   // No neon in x86 ;)
int EXPORT my2_SDL_HasRDTSC() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE2() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE3() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasSSE41() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasSSE42() __attribute__((alias("sdl_No")));

typedef struct {
  int32_t freq;
  uint16_t format;
  uint8_t channels;
  uint8_t silence;
  uint16_t samples;
  uint32_t size;
  void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  void *userdata;
} SDL_AudioSpec;

KHASH_MAP_INIT_INT(timercb, x86emu_t*)

// TODO: put the wrapper type in a dedicate include
typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFpp_t)(void*, void*);
typedef int32_t (*iFppi_t)(void*, void*, int32_t);
typedef int32_t (*iFpippi_t)(void*, int32_t, void*, void*, int32_t);
typedef int32_t (*iFppp_t)(void*, void*, void*);
typedef void* (*pFpippp_t)(void*, int32_t, void*, void*, void*);
typedef void*  (*pFpp_t)(void*, void*);
typedef void*  (*pFppp_t)(void*, void*, void*);
typedef void  (*vFp_t)(void*);
typedef void  (*vFpp_t)(void*, void*);
typedef int32_t (*iFpupp_t)(void*, uint32_t, void*, void*);
typedef uint32_t (*uFu_t)(uint32_t);
typedef uint32_t (*uFp_t)(void*);
typedef uint32_t (*uFupp_t)(uint32_t, void*, void*);
typedef uint64_t (*UFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef uint32_t (*uFpC_t)(void*, uint8_t);
typedef uint32_t (*uFpW_t)(void*, uint16_t);
typedef uint32_t (*uFpu_t)(void*, uint32_t);
typedef uint32_t (*uFpU_t)(void*, uint64_t);

typedef struct sdl2_my_s {
    iFpp_t     SDL_OpenAudio;
    iFpippi_t  SDL_OpenAudioDevice;
    pFpi_t     SDL_LoadFile_RW;
    pFpi_t     SDL_LoadBMP_RW;
    pFpi_t     SDL_RWFromConstMem;
    pFpi_t     SDL_RWFromFP;
    pFpp_t     SDL_RWFromFile;
    pFpi_t     SDL_RWFromMem;
    iFppi_t    SDL_SaveBMP_RW;
    pFpippp_t  SDL_LoadWAV_RW;
    iFpi_t     SDL_GameControllerAddMappingsFromRW;
    vFp_t      SDL_FreeRW;
    uFp_t      SDL_ReadU8;
    uFp_t      SDL_ReadBE16;
    uFp_t      SDL_ReadBE32;
    UFp_t      SDL_ReadBE64;
    uFp_t      SDL_ReadLE16;
    uFp_t      SDL_ReadLE32;
    UFp_t      SDL_ReadLE64;
    uFpC_t     SDL_WriteU8;
    uFpW_t     SDL_WriteBE16;
    uFpu_t     SDL_WriteBE32;
    uFpU_t     SDL_WriteBE64;
    uFpW_t     SDL_WriteLE16;
    uFpu_t     SDL_WriteLE32;
    uFpU_t     SDL_WriteLE64;
    uFupp_t    SDL_AddTimer;
    uFu_t      SDL_RemoveTimer;
    pFppp_t    SDL_CreateThread;
    vFp_t      SDL_KillThread;
    vFpp_t     SDL_SetEventFilter;
    vFpp_t     SDL_LogSetOutputFunction;
    // timer map
    kh_timercb_t    *timercb;
    uint32_t        settimer;
    // threads
    kh_timercb_t    *threads;
    // evt filter
    x86emu_t        *sdl2_evtfiler;
    void*           sdl2_evtfnc;
    // log output
    x86emu_t        *sdl2_logouput;
} sdl2_my_t;

void* getSDL2My(library_t* lib)
{
    sdl2_my_t* my = (sdl2_my_t*)calloc(1, sizeof(sdl2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(SDL_OpenAudio, iFpp_t)
    GO(SDL_OpenAudioDevice, iFpippi_t)
    GO(SDL_LoadBMP_RW, pFpi_t)
    GO(SDL_RWFromConstMem, pFpi_t)
    GO(SDL_RWFromFP, pFpi_t)
    GO(SDL_RWFromFile, pFpp_t)
    GO(SDL_RWFromMem, pFpi_t)
    GO(SDL_SaveBMP_RW, iFppi_t)
    GO(SDL_LoadWAV_RW, pFpippp_t)
    GO(SDL_GameControllerAddMappingsFromRW, iFpi_t)
    GO(SDL_FreeRW, vFp_t)
    GO(SDL_ReadU8, uFp_t)
    GO(SDL_ReadBE16, uFp_t)
    GO(SDL_ReadBE32, uFp_t)
    GO(SDL_ReadBE64, UFp_t)
    GO(SDL_ReadLE16, uFp_t)
    GO(SDL_ReadLE32, uFp_t)
    GO(SDL_ReadLE64, UFp_t)
    GO(SDL_WriteU8, uFpC_t)
    GO(SDL_WriteBE16, uFpW_t)
    GO(SDL_WriteBE32, uFpu_t)
    GO(SDL_WriteBE64, uFpU_t)
    GO(SDL_WriteLE16, uFpW_t)
    GO(SDL_WriteLE32, uFpu_t)
    GO(SDL_WriteLE64, uFpU_t)
    GO(SDL_AddTimer, uFupp_t)
    GO(SDL_RemoveTimer, uFu_t)
    GO(SDL_CreateThread, pFppp_t)
    GO(SDL_KillThread, vFp_t)
    GO(SDL_SetEventFilter, vFpp_t)
    GO(SDL_LogSetOutputFunction, vFpp_t)
    #undef GO
    my->timercb = kh_init(timercb);
    my->threads = kh_init(timercb);
    return my;
}

void freeSDL2My(void* lib)
{
    sdl2_my_t *my = (sdl2_my_t *)lib;
    x86emu_t *x;
    kh_foreach_value(my->timercb, x, 
        FreeCallback(x);
    );
    kh_destroy(timercb, my->timercb);

    kh_foreach_value(my->threads, x, 
        FreeCallback(x);
    );
    kh_destroy(timercb, my->threads);
    if(my->sdl2_evtfiler) {
        FreeCallback(my->sdl2_evtfiler);
    }
}


void sdl2Callback(void *userdata, uint8_t *stream, int32_t len)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 1, stream);
    SetCallbackArg(emu, 2, (void*)len);
    RunCallback(emu);
}

uint32_t sdl2TimerCallback(void *userdata)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    RunCallback(emu);
    return R_EAX;
}

int32_t sdl2ThreadCallback(void *userdata)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    RunCallback(emu);
    int32_t ret = (int32_t)R_EAX;
    FreeCallback(emu);
    return ret;
}

int32_t sdl2EvtFilterCallback(void *p)
{
    x86emu_t *emu = (x86emu_t*) p;
    RunCallback(emu);
    return (int32_t)R_EAX;
}

void sdl2LogOutputCallback(void *userdata, int32_t category, uint32_t priority, const char* message)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 1, (void*)category);
    SetCallbackArg(emu, 2, (void*)priority);
    SetCallbackArg(emu, 3, (void*)message);
    RunCallback(emu);
}

// TODO: track the memory for those callback
int32_t EXPORT my2_SDL_OpenAudio(x86emu_t* emu, void* d, void* o)
{
    SDL_AudioSpec *desired = (SDL_AudioSpec*)d;

    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // create a callback
    void *fnc = (void*)desired->callback;
    void *olduser = desired->userdata;
    x86emu_t *cbemu = (desired->callback)?AddCallback(emu, (uintptr_t)fnc, 3, olduser, NULL, NULL, NULL):NULL;
    desired->callback = sdl2Callback;
    desired->userdata = cbemu;
    int ret = my->SDL_OpenAudio(desired, (SDL_AudioSpec*)o);
    if (ret!=0) {
        // error, clean the callback...
        desired->callback = fnc;
        desired->userdata = olduser;
        FreeCallback(cbemu);
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;
    desired->userdata = olduser;

    return ret;
}

int32_t EXPORT my2_SDL_OpenAudioDevice(x86emu_t* emu, void* device, int32_t iscapture, void* d, void* o, int32_t allowed)
{
    SDL_AudioSpec *desired = (SDL_AudioSpec*)d;

    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // create a callback
    void *fnc = (void*)desired->callback;
    void *olduser = desired->userdata;
    x86emu_t *cbemu = (desired->callback)?AddCallback(emu, (uintptr_t)fnc, 3, olduser, NULL, NULL, NULL):NULL;
    desired->callback = sdl2Callback;
    desired->userdata = cbemu;
    int ret = my->SDL_OpenAudioDevice(device, iscapture, desired, (SDL_AudioSpec*)o, allowed);
    if (ret<=0) {
        // error, clean the callback...
        desired->callback = fnc;
        desired->userdata = olduser;
        FreeCallback(cbemu);
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;
    desired->userdata = olduser;

    return ret;
}

void EXPORT *my2_SDL_LoadFile_RW(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    void* r = my->SDL_LoadFile_RW(a, b);
    if(b==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
void EXPORT *my2_SDL_LoadBMP_RW(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    void* r = my->SDL_LoadBMP_RW(a, b);
    if(b==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
int32_t EXPORT my2_SDL_SaveBMP_RW(x86emu_t* emu, void* a, void* b, int c)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    int32_t r = my->SDL_SaveBMP_RW(a, b, c);
    if(c==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
void EXPORT *my2_SDL_LoadWAV_RW(x86emu_t* emu, void* a, int b, void* c, void* d, void* e)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    void* r = my->SDL_LoadWAV_RW(a, b, c, d, e);
    if(b==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
int32_t EXPORT my2_SDL_GameControllerAddMappingsFromRW(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    int32_t r = my->SDL_GameControllerAddMappingsFromRW(a, b);
    if(b==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_ReadU8(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadU8(a);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_ReadBE16(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadBE16(a);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_ReadBE32(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadBE32(a);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint64_t EXPORT my2_SDL_ReadBE64(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint64_t r = my->SDL_ReadBE64(a);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_ReadLE16(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadLE16(a);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_ReadLE32(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadLE32(a);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint64_t EXPORT my2_SDL_ReadLE64(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint64_t r = my->SDL_ReadLE64(a);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_WriteU8(x86emu_t* emu, void* a, uint8_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_WriteU8(a, v);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_WriteBE16(x86emu_t* emu, void* a, uint16_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_WriteBE16(a, v);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_WriteBE32(x86emu_t* emu, void* a, uint32_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_WriteBE32(a, v);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_WriteBE64(x86emu_t* emu, void* a, uint64_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_WriteBE64(a, v);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_WriteLE16(x86emu_t* emu, void* a, uint16_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_WriteLE16(a, v);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_WriteLE32(x86emu_t* emu, void* a, uint32_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_WriteLE32(a, v);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my2_SDL_WriteLE64(x86emu_t* emu, void* a, uint64_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    uint32_t r = my->SDL_WriteLE64(a, v);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}

void EXPORT *my2_SDL_RWFromConstMem(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromConstMem(a, b);
    AddNativeRW2(emu, (SDL2_RWops_t*)r);
    return r;
}
void EXPORT *my2_SDL_RWFromFP(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromFP(a, b);
    AddNativeRW2(emu, (SDL2_RWops_t*)r);
    return r;
}
void EXPORT *my2_SDL_RWFromFile(x86emu_t* emu, void* a, void* b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromFile(a, b);
    AddNativeRW2(emu, (SDL2_RWops_t*)r);
    return r;
}
void EXPORT *my2_SDL_RWFromMem(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromMem(a, b);
    AddNativeRW2(emu, (SDL2_RWops_t*)r);
    return r;
}

void EXPORT my2_SDL_FreeRW(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    my->SDL_FreeRW(a);
}

uint32_t EXPORT my2_SDL_AddTimer(x86emu_t* emu, uint32_t a, void* cb, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    x86emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 1, p, NULL, NULL, NULL);
    uint32_t t = my->SDL_AddTimer(a, sdl2TimerCallback, cbemu);
    int ret;
    khint_t k = kh_put(timercb, my->timercb, t, &ret);
    kh_value(my->timercb, k) = cbemu;
    return t;
}

void EXPORT my2_SDL_RemoveTimer(x86emu_t* emu, uint32_t t)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    my->SDL_RemoveTimer(t);
    khint_t k = kh_get(timercb,my->timercb, t);
    if(k!=kh_end(my->timercb))
    {
        FreeCallback(kh_value(my->timercb, k));
        kh_del(timercb, my->timercb, k);
    }
}

void EXPORT my2_SDL_SetEventFilter(x86emu_t* emu, void* a, void* b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    if(my->sdl2_evtfiler) {
        my->SDL_SetEventFilter(NULL, NULL);   // remove old one
        FreeCallback(my->sdl2_evtfiler);
        my->sdl2_evtfiler = NULL;
        my->sdl2_evtfnc = NULL;
    }
    if(a) {
        my->sdl2_evtfnc = a;
        my->sdl2_evtfiler = AddCallback(emu, (uintptr_t)a, 1, b, NULL, NULL, NULL);
        my->SDL_SetEventFilter(sdl2EvtFilterCallback, my->sdl2_evtfiler);
    }
}
void EXPORT *my2_SDL_GetEventFilter(x86emu_t* emu)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    return my->sdl2_evtfnc;
}

EXPORT void my2_SDL_LogSetOutputFunction(x86emu_t* emu, void* cb, void* arg)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    if(my->sdl2_logouput) {
        my->SDL_LogSetOutputFunction(NULL, NULL);   // remove old one
        FreeCallback(my->sdl2_logouput);
        my->sdl2_logouput = NULL;
        my->sdl2_evtfnc = NULL;
    }
    if(cb) {
        my->sdl2_logouput = AddCallback(emu, (uintptr_t)cb, 4, arg, NULL, NULL, NULL);
        my->SDL_LogSetOutputFunction(sdl2EvtFilterCallback, my->sdl2_logouput);
    }
}

EXPORT int my2_SDL_vsnprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vsnprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, emu->scratch);
    return r;
    #else
    return vsnprintf((char*)buff, s, (char*)fmt, V);
    #endif
}


void EXPORT *my2_SDL_CreateThread(x86emu_t* emu, void* cb, void* n, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    x86emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 1, p, NULL, NULL, NULL);
    void* t = my->SDL_CreateThread(sdl2ThreadCallback, n, cbemu);
    int ret;
    khint_t k = kh_put(timercb, my->threads, (uintptr_t)t, &ret);
    kh_value(my->threads, k) = cbemu;
    return t;
}

void EXPORT my2_SDL_KillThread(x86emu_t* emu, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    my->SDL_KillThread(p);
    khint_t k = kh_get(timercb,my->threads, (uintptr_t)p);
    if(k!=kh_end(my->threads))
    {
        FreeCallback(kh_value(my->threads, k));
        kh_del(timercb, my->threads, k);
    }
}

int EXPORT my2_SDL_snprintf(x86emu_t* emu, void* buff, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vsprintf;
    return ((iFppp_t)f)(buff, fmt, emu->scratch);
    #else
    return vsprintf((char*)buff, (char*)fmt, V);
    #endif
}

const char* sdl2Name = "libSDL2-2.0.so";
#define LIBNAME sdl2

#define CUSTOM_INIT \
    box86->sdl2lib = lib; \
    lib->priv.w.priv = NewSDL2RWops(); \
    lib->priv.w.p2 = getSDL2My(lib); \
    lib->altmy = strdup("my2_");

#define CUSTOM_FINI \
    FreeSDL2RWops((sdl2rwops_t**)&lib->priv.w.priv); \
    freeSDL2My(lib->priv.w.p2); \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->sdl2lib = NULL;

#include "wrappedlib_init.h"


