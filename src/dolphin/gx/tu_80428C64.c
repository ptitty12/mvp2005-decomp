/*
 * Dolphin SDK GX (MWCC GC/1.2.5n, cflags_dolphin_sdk + -opt nopeephole: -O4,p),
 * .text 0x80428C64-0x80428F14 (code-only unit, 3 functions; placeholder name).
 * Head of GXInit.c (shape of the code, with the reference dolsdk2004 GXInit.c): the default texture and
 * TLUT region callbacks and __GXShutdown. Split off dolphin/exi/tu_80426CD0.c (docs/DECISIONS.md 2026-10-12).
 */

#include <dolphin.h>
#include <dolphin/os.h>
#include <dolphin/gx.h>
#include <dolphin/hw_regs.h>

#include "__gx.h"

extern u32 lbl_806EF748;  /* __GXShutdown peCount */
extern OSTime lbl_806EF750;  /* __GXShutdown time */
extern u32 lbl_806EF758;  /* __GXShutdown calledOnce */
OSTime OSGetTime(void);

/* compile with: MWCC GC/1.2.5n, cflags_dolphin_sdk (-O4,p) + -opt nopeephole */
/* GXInit.c part (0x80428C64..): fn_80428C64 __GXDefaultTexRegionCallback, fn_80428D60
 * __GXDefaultTlutRegionCallback, fn_80428D84 __GXShutdown. Placeholder callees: GXSetBreakPtCallback
 * GXSetBreakPtCallback, GXSetDrawSyncCallback, fn_8042C6F8 GXSetDrawDoneCallback,
 * PPCSync PPCSync, fn_8042C07C __GXAbort. */
GXBreakPtCallback GXSetBreakPtCallback(GXBreakPtCallback cb);
GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback cb);
int fn_8042C6F8(void* cb);
void PPCSync(void);
void fn_8042C07C(void);

GXTexRegion* fn_80428C64(const GXTexObj* t_obj, GXTexMapID id) {
    GXTexFmt fmt;
    u8 mm;

    fmt = GXGetTexObjFmt(t_obj);
    mm = GXGetTexObjMipMap(t_obj);
    id = (GXTexMapID)(id % GX_MAX_TEXMAP);

    switch (fmt) {
    case GX_TF_RGBA8:
        if (mm) {
            return &__GXData->TexRegions2[id];
        }
        return &__GXData->TexRegions1[id];
    case GX_TF_C4:
    case GX_TF_C8:
    case GX_TF_C14X2:
        return &__GXData->TexRegions0[id];
    default:
        if (mm) {
            return &__GXData->TexRegions1[id];
        }
        return &__GXData->TexRegions0[id];
    }
}

GXTlutRegion* fn_80428D60(u32 idx) {
    if (idx >= 20) {
        return NULL;
    }
    return &__GXData->TlutRegions[idx];
}

int fn_80428D84(BOOL final) {
    u32 reg;
    u32 peCountNew;
    OSTime timeNew;

    if (!final) {
        if (!lbl_806EF758) {
            lbl_806EF748 = __GXReadMEMCounterU32(0x28, 0x27);
            lbl_806EF750 = OSGetTime();
            lbl_806EF758 = 1;
            return 0;
        }

        timeNew = OSGetTime();
        peCountNew = __GXReadMEMCounterU32(0x28, 0x27);

        if (timeNew - lbl_806EF750 < 10) {
            return 0;
        }

        if (peCountNew != lbl_806EF748) {
            lbl_806EF748 = peCountNew;
            lbl_806EF750 = timeNew;
            return 0;
        }

    } else {
        GXSetBreakPtCallback(NULL);
        GXSetDrawSyncCallback(NULL);
        fn_8042C6F8(NULL);

        GX_WRITE_U32(0);
        GX_WRITE_U32(0);
        GX_WRITE_U32(0);
        GX_WRITE_U32(0);
        GX_WRITE_U32(0);
        GX_WRITE_U32(0);
        GX_WRITE_U32(0);
        GX_WRITE_U32(0);

        PPCSync();

        reg = 0;
        GX_SET_CP_REG(1, reg);

        reg = 3;
        GX_SET_CP_REG(2, reg);

        __GXData->abtWaitPECopy = 1;

        fn_8042C07C();
    }

    return 1;
}
