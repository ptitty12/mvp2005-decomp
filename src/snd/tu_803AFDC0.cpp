/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803AFDC0-0x803B21AC (code-only unit, 42 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */

#include "snd/cmn/sndcmn.h"
#include <string.h>

extern "C" void OSInitMutex(void* p);

extern char lbl_806AA030[];

extern "C" void SNDARAM_free(int a);
extern "C" void* SNDARAM_alloc(int a);
extern "C" void DCFlushRange(int a, int b);
extern "C" void SNDDRV_dmapost(int a, int b, int c, int d, int e);
extern int* lbl_806AA2C4;

static inline CHANPUB* ChanAt(CHANPUB* a, int i) { return &a[i]; }

extern "C" int MIX_filteradd(int voice, int ch, int v);
extern "C" int MIX_getframe(int idx);
extern "C" void MIX_sethighpass(int idx, int n);
extern "C" void MIX_setlowpass(int idx, float v);
extern "C" void SNDMIX_setdrygain(int idx, int ch, float v);
extern "C" void MIX_settimemult(int idx, int v);
extern "C" int SNDPLATFORM_memfree(int a, int p);

struct Unk803B0714 {
    char pad[0x2580];
    int m2580[6];
    signed char m2598;
    char pad2599[3];
    int m259C[6];
};

struct Unk803B0E74_Pos {
    char pad0[0x1B2];
    unsigned short m1B2;
    unsigned short m1B4;
};

/* 0x40-byte per-voice record at lbl_8069E200 + voice * 0x40 (0x50 of them), then the voice table. */
struct Unk8069E200_Rec {
    float mVol[6];
    int m18;
    signed char m1C;
    char pad1D[7];
    unsigned char m24;
    char pad25[0x17];
    Unk803B0E74_Pos* m3C;
};

struct Unk8069E200 {
    Unk8069E200_Rec rec[0x50];
    char pad1400[0x10];
    Unk803B0714* mVoices[16];
};
extern Unk8069E200 lbl_8069E200;

extern "C" void OSLockMutex(void* p);

extern "C" void OSUnlockMutex(void* p);

extern "C" void MIX_create();
extern "C" void MIX_destroy();
extern "C" void MIX_audioslice();
extern "C" void MIX_playinit(int slot, int kind, int mode, int a3, int a4, int a5, int a6, int a7, int p8, int p9, int p10, int p11, int p12, int p13);
extern "C" void SFILTER_add(void* head, void* node);
extern "C" int SFILTER_timestretchinit(void* obj, int a, int b);
extern unsigned char lbl_8069C9EB;
/* near-miss: 41 insns, identical except scratch register choice for the early li 0 / lis constants (retail: li r11,0 right after the lwz; ours li r0,0 after the lfs); statement-order variants tried */
extern "C" void MIX_play();
extern "C" void MIX_stop();
extern "C" void MIX_setpitch();
extern void* lbl_806AA4C8;
extern void* lbl_806AA4CC;
extern void* lbl_806AA4D0;
extern void* lbl_806AA4D4;
extern void* lbl_806AA4D8;
extern void* lbl_806AA4DC;
extern void* lbl_806AA4E0;

extern "C" void mixc();
extern "C" void decode16x87();

struct Unk803B18C0_B4 {
    char pad0[0x3C];
    float m3C;
};

struct Unk806AA2D4B {
    char pad0[9];
    unsigned char m9;
    char padA[2];
    short mC;
    char padE[6];
    float* m14;
    void* m18[6][3];
    int m60[6][3];
    void* mA8[2];
    void* mB0;
    struct Unk803B18C0_B4* mB4;
    void* mB8[6];
    float* mD0[6];
    float mE8[6];
    float* m100;
    char pad104[0xD8];
    void* m1DC;
    void* m1E0;
    void* m1E4;
};
extern Unk806AA2D4B lbl_806AA2D4;

extern "C" void* fn_803B1118(int unused, int size);
extern "C" void SNDDRV_restoredma(int id);
extern "C" unsigned int OSDisableInterrupts();
extern "C" void OSRestoreInterrupts(unsigned int);
extern char lbl_806AE200[];

struct Unk803B11FC_Rec {
    int m0;
    char pad4[0x30];
    unsigned int m34;
    char pad38[8];
};

struct Unk803B1378_Node {
    Unk803B1378_Node* m0;
};
extern Unk803B1378_Node** lbl_806AA2C0;
extern int lbl_806262B0;
extern int lbl_80625448;

struct Unk803B21EC_L {
    unsigned char m0;
    unsigned char m1;
    char pad2[2];
    float m4[6];
    float m1C[6];
    float* m34;
    float* m38;
    float m3C;
    struct Unk803B1EF4_Node* m40;
    int m44;
    struct Unk803B1EF4_Node* m48;
    struct Unk803B1EF4_Node* m4C;
    int m50;
    int m54;
    int m58;
};

/* Mixer node: first word is the process callback. */
struct Unk803B1EF4_Node {
    void* m0;
    int m4;
    char pad8[0x10];
    short m18;
    char m1A;
};

struct Unk803B1EF4_Args {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
};
extern Unk803B21EC_L* lbl_806AA4B0;
extern float lbl_8060984C;

extern float lbl_80609848;
extern "C" void SNDSYS_entercritical();
extern "C" void SNDSYS_leavecritical();

extern "C" int SNDDRV_getmastervoice(int v)
{
    int idx = v + sndgs.sso.set.voicesdsp;
    int m = ChanAt(sndgs.chan, idx)->masterchan;
    if (m == -1)
        return idx;
    return m;
}

/* One channel pointer reused for both entries: a fresh pointer dies at the lha and pulls masterchan into its register. */
extern "C" int SNDDRV_getsamplechan(int v)
{
    int idx = v + sndgs.sso.set.voicesdsp;
    CHANPUB* mc = ChanAt(sndgs.chan, idx);
    int m = mc->masterchan;
    if (m == -1)
        return 0;
    mc = ChanAt(sndgs.chan, m);
    int i;
    for (i = 1; i <= 5; i++) {
        if (mc->voices[i] == idx)
            return i;
    }
    return 0;
}

extern "C" int fn_803B0360()
{
    return 0x25e4;
}

extern "C" int SNDPLATFORM_packetplaycreate(int slot, Unk803B0714* p)
{
    memset(p, 0, 0x25e4);
    p->m259C[0] = (int)fn_803B1118(0, 0x4800);
    if (p->m259C[0] == 0)
        return -9;
    int i;
    for (i = 0; i <= 5; i++) {
        p->m259C[i] = p->m259C[0] + i * 0xC00;
        p->m2580[i] = ((int)p + i * 0x640 + 0x1f) & ~0x1f;
    }
    p->m2598 = -1;
    lbl_8069E200.mVoices[slot] = p;
    return 0;
}

extern "C" int fn_803B0714(int idx)
{
    Unk803B0714* p = lbl_8069E200.mVoices[idx];
    SNDPLATFORM_memfree(0, p->m259C[0]);
    int i;
    for (i = 0; i <= 5; i++)
        p->m259C[i] = 0;
    lbl_8069E200.mVoices[idx] = 0;
    return 0;
}

extern "C" int fn_803B0798()
{
    return -15;
}

extern "C" void fn_803AE0FC(void* h, int v);
void Snd::Hal::SetVol(int voice)
{
    CHANPUB* c = (CHANPUB*)((char*)sndgs.chan + voice * 0x90);
    Unk8069E200_Rec* r = &lbl_8069E200.rec[voice];
    float vol;
    if (c->rendermode & 0x200) {
        vol = c->finalvol * (float)c->drylevel;
        fn_803AE0FC(lbl_8069E200.rec[voice].m3C, (int)(vol * 258.0f));
        return;
    }
    vol = c->finalvol * (float)c->drylevel * 0.007874016f;
    unsigned char ch = sndgs.sso.set.outputchannels;
    unsigned char n;
    if (ch == 1)
        n = 2;
    else
        n = ch;
    int i;
    for (i = 0; i < n; i++)
        SNDMIX_setdrygain(voice - sndgs.sso.set.voicesdsp, i, vol * r->mVol[i]);
}

void Snd::Hal::SetDry(int voice)
{
    CHANPUB* c = (CHANPUB*)((char*)sndgs.chan + voice * 0x90);
    if (*(unsigned int*)&c->rendermode & 0x2040000)
        SetVol(voice);
}

extern "C" void fn_803AE0FC(void* h, int v);
extern "C" void SNDDRV_freeaxvoice(int v, int k);
extern "C" void AXSetVoiceState(int h, int v);


extern "C" int SNDPLATFORM_stop(int voice)
{
    CHANPUB* c = &sndgs.chan[voice];
    Unk8069E200_Rec* r = &lbl_8069E200.rec[voice];
    int i;
    if (c->rendermode & 0x200) {
        if (r->m1C >= 0) {
            lbl_8069E200.mVoices[r->m1C]->m2598 = -1;
            r->m1C = -1;
        }
        for (i = 0; i < c->numchan; i++) {
            void* h = lbl_8069E200.rec[c->voices[i]].m3C;
            if (h == 0)
                return -8;
            AXSetVoiceState((int)h, 0);
            if (lbl_8069E200.rec[c->voices[i]].m24 != 0) {
                SNDDRV_freeaxvoice(c->voices[i], 0);
                lbl_8069E200.rec[c->voices[i]].m24 = 0;
            }
        }
    } else {
        for (i = 0; i < c->numchan; i++) {
            ((void (*)(int))lbl_806AA4DC)(c->voices[i] - sndgs.sso.set.voicesdsp);
            SNDVOICEI_free(c->voices[i]);
        }
    }
    return 0;
}

extern "C" int SNDPLATFORM_filteradd(int voice, int v)
{
    CHANPUB* c = &sndgs.chan[voice];
    if (c->rendermode & 4) {
        int i;
        for (i = 0; i < c->numchan; i++)
            MIX_filteradd(c->voices[i] - sndgs.sso.set.voicesdsp, i, v);
        return 0;
    }
    return -7;
}

extern "C" void* fn_803B1118(int unused, int size)
{
    size = (size + 0x1f) & ~0x1f;
    if (sndgs.sso.vec.aramalloc != 0)
        return (void*)sndgs.sso.vec.aramalloc(size);
    return SNDARAM_alloc(size);
}

extern "C" int SNDPLATFORM_memfree(int unused, int p)
{
    if (sndgs.sso.vec.aramfree == 0)
        SNDARAM_free(p);
    else
        sndgs.sso.vec.aramfree(p);
    return 0;
}

extern "C" void fn_803B11A8(int unused, int a, int b, int c)
{
    DCFlushRange(a, c);
    SNDDRV_dmapost(0, a, b, c, -1);
}

extern "C" int fn_803B11FC(int id)
{
    unsigned int en = OSDisableInterrupts();
    int i;
    char* base = lbl_806AE200;
    char* ba = base - 0x61d0;
    char* bb = base - 0x619c;
    for (i = 0; i <= 63; i++) {
        int off = i << 6;
        if (*(volatile int*)(ba + off) != -1 && *(volatile int*)(ba + off) == id) {
            if (sndgs.audiotick - *(unsigned int*)(bb + off) > 200) {
                OSRestoreInterrupts(en);
                SNDDRV_restoredma(id);
            } else {
                OSRestoreInterrupts(en);
            }
            return 0;
        }
    }
    OSRestoreInterrupts(en);
    return 1;
}

extern "C" void fn_803B12A4()
{
    OSInitMutex(lbl_806AA030);
}

extern "C" void fn_803B12CC()
{
}

extern "C" void fn_803B12D0()
{
    OSLockMutex(lbl_806AA030);
}

extern "C" void fn_803B12F8()
{
    OSUnlockMutex(lbl_806AA030);
}

extern "C" int SNDPLATFORM_ModeSetUp(SNDSYSOPTS* p, int kind)
{
    switch (kind) {
    case 0:
        p->set.outputchannels = 1;
        break;
    case 1:
        p->set.outputchannels = 2;
        break;
    case 8:
        p->set.outputchannels = 4;
        break;
    }
    return 0;
}

extern "C" int fn_803B1364(int i)
{
    return lbl_806AA2C4[i];
}

/* near-miss: size expression (n+n+n)<<2 now matches; retail keeps only lbl_80625448 and lbl_806AA2C0 in saved registers (lbl_806AA2C4 is stored through a scratch lis) and does the m14/m100 stores before the memset; ours hoists lbl_806AA2C4 into r28 */
extern "C" void MIX_createFxglobals()
{
    int n = lbl_80625448;
    int t = n + n + n;
    int size = t << 2;
    float* p = (float*)SNDMEMI_allocz(size);
    lbl_806AA2C4 = (int*)p;
    lbl_806AA2D4.m14 = p + lbl_80625448;
    lbl_806AA2D4.m100 = lbl_806AA2D4.m14 + lbl_80625448;
    memset(p, 0, size);
    lbl_806AA2C0 = (Unk803B1378_Node**)SNDMEMI_allocz(lbl_80625448 * 4);
    memset(lbl_806AA2C0, 0, lbl_80625448 * 4);
    lbl_806262B0 = 1;
}

extern "C" void MIX_destroyFxglobals()
{
    if (lbl_806262B0 != 0) {
        SNDMEMI_free(lbl_806AA2C4);
        int i;
        for (i = 0; i < lbl_80625448; i++) {
            Unk803B1378_Node* head = lbl_806AA2C0[i];
            if (head != 0) {
                Unk803B1378_Node* p = head;
                do {
                    Unk803B1378_Node* next = p->m0;
                    SNDMEMI_free(p);
                    p = next;
                } while (p != 0);
            }
        }
        SNDMEMI_free(lbl_806AA2C0);
        lbl_806262B0 = 0;
    }
}

extern "C" void SNDSYS_linkmaincpumixer()
{
    lbl_806AA4C8 = (void*)MIX_create;
    lbl_806AA4CC = (void*)MIX_destroy;
    lbl_806AA4D0 = (void*)MIX_audioslice;
    lbl_806AA4D4 = (void*)MIX_playinit;
    lbl_806AA4D8 = (void*)MIX_play;
    lbl_806AA4DC = (void*)MIX_stop;
    lbl_806AA4E0 = (void*)MIX_setpitch;
}

extern "C" void MIXI_interpolateto0(float* p, float* q)
{
    q[0] = *p * (16.0f / 17.0f) + q[0];
    q[1] = *p * (15.0f / 17.0f) + q[1];
    q[2] = *p * (14.0f / 17.0f) + q[2];
    q[3] = *p * (13.0f / 17.0f) + q[3];
    q[4] = *p * (12.0f / 17.0f) + q[4];
    q[5] = *p * (11.0f / 17.0f) + q[5];
    q[6] = *p * (10.0f / 17.0f) + q[6];
    q[7] = *p * (9.0f / 17.0f) + q[7];
    q[8] = *p * (8.0f / 17.0f) + q[8];
    q[9] = *p * (7.0f / 17.0f) + q[9];
    q[10] = *p * (6.0f / 17.0f) + q[10];
    q[11] = *p * (5.0f / 17.0f) + q[11];
    q[12] = *p * (4.0f / 17.0f) + q[12];
    q[13] = *p * (3.0f / 17.0f) + q[13];
    q[14] = *p * (2.0f / 17.0f) + q[14];
    q[15] = *p * (1.0f / 17.0f) + q[15];
    *p = 0.0f;
}

extern "C" void MIXI_interpolatemix(float* a, float* b, float lo, float hi)
{
    float s = (hi - lo) * (1.0f / 17.0f);
    b[0] = a[0] * (lo + s) + b[0];
    b[1] = a[1] * (lo + 2.0f * s) + b[1];
    b[2] = a[2] * (s * 3.0f + lo) + b[2];
    b[3] = a[3] * (s * 4.0f + lo) + b[3];
    b[4] = a[4] * (s * 5.0f + lo) + b[4];
    b[5] = a[5] * (s * 6.0f + lo) + b[5];
    b[6] = a[6] * (s * 7.0f + lo) + b[6];
    b[7] = a[7] * (s * 8.0f + lo) + b[7];
    b[8] = a[8] * (s * 9.0f + lo) + b[8];
    b[9] = a[9] * (s * 10.0f + lo) + b[9];
    b[10] = a[10] * (s * 11.0f + lo) + b[10];
    b[11] = a[11] * (s * 12.0f + lo) + b[11];
    b[12] = a[12] * (s * 13.0f + lo) + b[12];
    b[13] = a[13] * (s * 14.0f + lo) + b[13];
    b[14] = a[14] * (s * 15.0f + lo) + b[14];
    b[15] = a[15] * (s * 16.0f + lo) + b[15];
}

extern "C" void fn_803B18BC(float)
{
}

extern "C" int SNDMIXI_volramp(Unk803B21EC_L* l)
{
    int i;
    if (((int (*)(void*, int, void*, void*, int))l->m40->m0)(l->m40, 0x10, lbl_806AA2D4.mB0, lbl_806AA2D4.mB4, 0) <= 0) {
        for (i = 0; i < lbl_80625448; i++)
            l->m34[i] = l->m38[i];
        for (i = 0; i < lbl_806AA2D4.m9; i++)
            l->m4[i] = l->m1C[i];
        return 0;
    }
    l->m3C = lbl_806AA2D4.mB4->m3C;
    for (i = 0; i < lbl_80625448; i++) {
        if (((int*)lbl_806AA2D4.m14)[i] != 0) {
            float a = l->m34[i];
            float b = l->m38[i];
            if (a != b) {
                MIXI_interpolatemix((float*)lbl_806AA2D4.mB4, ((float**)lbl_806AA2C4)[i], a, b);
                l->m34[i] = l->m38[i];
            } else if (b != 0.0f) {
                ((void (*)(int, void*, void*, float))lbl_806AA2D4.m1E0)(0x10, lbl_806AA2D4.mB4, ((void**)lbl_806AA2C4)[i], b);
            }
        }
    }
    for (i = 0; i < lbl_806AA2D4.m9; i++) {
        float a = l->m4[i];
        float b = l->m1C[i];
        if (a != b) {
            MIXI_interpolatemix((float*)lbl_806AA2D4.mB4, (float*)lbl_806AA2D4.mD0[i], a, b);
            l->m4[i] = l->m1C[i];
        } else if (b != 0.0f) {
            ((void (*)(int, void*, void*, float))lbl_806AA2D4.m1E0)(0x10, lbl_806AA2D4.mB4, lbl_806AA2D4.mD0[i], b);
        }
    }
    return 0x10;
}

extern "C" void* fn_803B1AF0(int n)
{
    return SNDMEMI_allocz(n);
}

extern "C" void fn_803B1B10(void* p)
{
    SNDMEMI_free(p);
}

extern "C" void MIX_destroy()
{
    int i;
    SNDSYS_entercritical();
    MIX_destroyFxglobals();
    if (lbl_806AA2D4.m1DC != 0) {
        SNDMEMI_free(lbl_806AA2D4.m1DC);
        lbl_806AA2D4.m1DC = 0;
    }
    for (i = 0; i <= 1; i++) {
        if (lbl_806AA2D4.mA8[i] != 0) {
            SNDMEMI_free(lbl_806AA2D4.mA8[i]);
            lbl_806AA2D4.mA8[i] = 0;
        }
    }
    for (i = 0; i < lbl_806AA2D4.m9; i++) {
        if (lbl_806AA2D4.mB8[i] != 0) {
            SNDMEMI_free(lbl_806AA2D4.mB8[i]);
            lbl_806AA2D4.mB8[i] = 0;
        }
    }
    fn_803B18BC(lbl_80609848);
    SNDSYS_leavecritical();
}

extern "C" void fn_803B1EC8()
{
    lbl_806AA2D4.m1E0 = (void*)mixc;
    lbl_806AA2D4.m1E4 = (void*)decode16x87;
    lbl_806AA2D4.mC = 0x140;
}

extern "C" void MIX_playinit(int slot, int kind, int mode, int a3, int a4, int a5, int a6, int a7, int p8, int p9, int p10, int p11, int p12, int p13)
{
    Unk803B21EC_L* e = &lbl_806AA4B0[slot];
    Unk803B1EF4_Args args;
    int idx = 0;
    int k;
    e->m40 = 0;
    e->m48 = 0;
    e->m50 = 0;
    e->m54 = 0;
    e->m58 = 0;
    e->m4C = 0;
    args.m18 = 1;
    k = -1;
    args.m28 = 0;
    if (kind == 7)
        k = 0;
    else if (kind == 10)
        k = 1;
    else if (kind == 4)
        k = 2;
    else if (kind == 22)
        k = 3;
    else if (kind == 16)
        k = 4;
    else if (kind == 23)
        k = 5;
    if (mode == 1) {
        if (p9 > 0)
            idx = 1;
    } else if (mode == 0) {
        idx = 2;
    }
    if (lbl_806AA2D4.m18[k][idx] != 0) {
        e->m48 = (Unk803B1EF4_Node*)SNDMEMI_allocz(lbl_806AA2D4.m60[k][idx]);
        args.m0 = a3;
        args.m4 = a4;
        args.m8 = a6;
        args.m24 = p12;
        args.mC = a7;
        args.m10 = p8;
        args.m14 = p9;
        args.m20 = lbl_8069C9EB;
        args.m1C = slot;
        e->m48->m4 = 0;
        e->m48->m18 = 0xF0;
        e->m48->m1A = p13;
        ((void (*)(void*, void*))lbl_806AA2D4.m18[k][idx])(e->m48, &args);
        e->m44 = args.m28;
        SFILTER_add(&e->m40, e->m48);
    }
    if (a5 != 0) {
        if (mode != 0)
            slot = -1;
        e->m4C = (Unk803B1EF4_Node*)SNDMEMI_allocz(0x1834);
        e->m4C->m4 = 0;
        e->m4C->m18 = 0xC8;
        e->m4C->m1A = p13;
        SFILTER_timestretchinit(e->m4C, a5, slot);
        SFILTER_add(&e->m40, e->m4C);
    }
    e->m0 = 1;
}

extern "C" void MIX_play(int idx)
{
    Unk803B21EC_L* e = &lbl_806AA4B0[idx];
    int i;
    float k = lbl_8060984C;
    e->m1 = 0;
    e->m3C = k;
    for (i = 0; i < lbl_80625448; i++)
        e->m34[i] = e->m38[i];
    for (i = 0; i < lbl_806AA2D4.m9; i++)
        e->m4[i] = e->m1C[i];
    e->m0 = 2;
}

int SNDPLATFORM_timemult(int voice, int timemult)
{
    CHANPUB* c = &sndgs.chan[voice];
    if (c->rendermode & 4) {
        int i;
        for (i = 0; i < c->numchan; i++)
            MIX_settimemult(c->voices[i] - sndgs.sso.set.voicesdsp, timemult);
    }
    return 0;
}

void SNDPLATFORM_lowpass(int voice, int cutofffreq)
{
    CHANPUB* c = &sndgs.chan[voice];
    if (c->rendermode & 4) {
        float f = (float)cutofffreq * (1.0f / ((float)(unsigned)sndgs.sso.set.outputrate * 0.5f));
        int i;
        for (i = 0; i < c->numchan; i++)
            MIX_setlowpass(c->voices[i] - sndgs.sso.set.voicesdsp, f);
    }
}

void SNDPLATFORM_highpass(int voice, int cutofffreq)
{
    CHANPUB* c = &sndgs.chan[voice];
    if (c->rendermode & 4) {
        int i;
        for (i = 0; i < c->numchan; i++)
            MIX_sethighpass(c->voices[i] - sndgs.sso.set.voicesdsp, cutofffreq);
    }
}

int SNDPLATFORM_getcurframe(int voice)
{
    Unk8069E200_Rec* r = &lbl_8069E200.rec[voice];
    CHANPUB* c = &sndgs.chan[voice];
    if (c->rendermode & 0x200) {
        Unk803B0E74_Pos* p = r->m3C;
        if (p == 0)
            return -8;
        int pos = (p->m1B2 << 16) + p->m1B4;
        if (c->samplerep == 0x12) {
            int d = pos - r->m18;
            return (d * 7) >> 3;
        }
        return pos - r->m18;
    }
    return MIX_getframe(voice - sndgs.sso.set.voicesdsp);
}

extern "C" void AXSetVoiceSrcRatio(int h, float pitch);
extern char lbl_8069E23C[];

int SNDPLATFORM_setpitch(int voice)
{
    CHANPUB* c = &sndgs.chan[voice];
    unsigned q;
    int i;
    if (c->rendermode & 0x200) {
        q = (c->samplerate << 12) / sndgs.sso.set.outputrate;
        q = (q * c->finalpitch) >> 12;
        float f = (float)q * 0.000244140625f;
        for (i = 0; i < c->numchan; i++) {
            int h = *(int*)(lbl_8069E23C + (c->voices[i] << 6));
            if (h == 0)
                return -8;
            AXSetVoiceSrcRatio(h, f);
        }
        return 0;
    } else {
        q = (c->samplerate << 15) / sndgs.sso.set.outputrate;
        q = (q * c->finalpitch) >> 11;
        for (i = 0; i < c->numchan; i++)
            ((void (*)(int, unsigned))lbl_806AA4E0)(c->voices[i] - sndgs.sso.set.voicesdsp, q);
        return 0;
    }
}

extern CHANPUB* lbl_8069CA48; /* sndgs.chan, named by its own symbols.txt entry */
extern unsigned char lbl_8069CBBC;
extern int lbl_8069CBC0;
extern unsigned char lbl_8069C8BB;
extern unsigned short lbl_80625468[][4];
extern "C" void SNDI_aztospkrvol(unsigned short az, Unk8069E200_Rec* r);
extern "C" void SNDDRV_DSPMixerSetPan(void* o, Unk8069E200_Rec* r, unsigned short az, int lfe);

void Snd::Hal::SetPan(int voice)
{
    Unk8069E200_Rec* r = &lbl_8069E200.rec[voice];
    CHANPUB* c = &lbl_8069CA48[voice];
    int i;
    if (c->isLfe != 0) {
        for (i = 0; i < lbl_8069CBBC; i++)
            r->mVol[i] = 0.0f;
    } else if (lbl_8069CBC0 == 0) {
        r->mVol[0] = 0.7071068f;
        r->mVol[1] = 0.7071068f;
        r->mVol[2] = 0.0f;
        r->mVol[3] = 0.0f;
        r->mVol[4] = 0.0f;
        r->mVol[5] = 0.0f;
    } else {
        SNDI_aztospkrvol(c->azimuth, r);
    }
    if ((c->rendermode & 4) && lbl_8069C8BB > 2 && c->isLfe == 0) {
        int o = (c->azimuth >> 5) & 0x7F8;
        char* tb = (char*)lbl_80625468;
        char* t2 = tb + 2;
        char* t6 = tb + 6;
        char* t4 = tb + 4;
        r->mVol[0] = (float)*(unsigned short*)(t2 + o) * 3.051851e-05f;
        r->mVol[1] = (float)*(unsigned short*)(t6 + o) * 3.051851e-05f;
        r->mVol[2] = (float)*(unsigned short*)(t4 + o) * 3.051851e-05f;
        r->mVol[5] = 0.0f;
        r->mVol[3] = (float)*(unsigned short*)(tb + o) * 3.051851e-05f;

    }
    if (c->rendermode & 0x200)
        SNDDRV_DSPMixerSetPan(r->m3C, r, c->azimuth, c->isLfe);
    else
        Snd::Hal::SetVol(voice);
}
