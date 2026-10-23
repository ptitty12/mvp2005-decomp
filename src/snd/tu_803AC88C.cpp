/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803AC88C-0x803AFDC0 (code-only unit, 43 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */

#include "snd/cmn/sndcmn.h"

struct DtorAt0 { const void* m; };

extern "C" int fn_803B9DDC(DtorAt0* s, int chrg);

extern unsigned char lbl_80625454;
extern void (*lbl_8069C93C)();
extern int lbl_80625460;
extern "C" void fn_80400C1C(int (*fn)(), int a, int b);
extern "C" void SYSTEM_addexit(int (*fn)());
extern "C" int fn_803B3E88();

extern "C" int fn_803B9E08(DtorAt0* s, int chrg);

extern void (*lbl_8062520C)(DtorAt0*, int);
extern void (*lbl_80625210)(DtorAt0*, int);

extern "C" void SNDSYS_service();

struct Unk803ADD08 {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    char pad1C[0x14];
    short m30;
    short m32;
    short m34;
    short m36;
    short m38;
    short m3A;
    short m3C;
    short m3E;
    short m40;
    short m42;
    short m44;
    short m46;
    short m48;
    short m4A;
    short m4C;
    short m4E;
    short m50;
    short m52;
    short m54;
    short m56;
};
extern Unk803ADD08 lbl_8069CBC4[64];

struct Unk803AE0E0 {
    char pad[0x18];
    int m18;
};
extern int lbl_8069E1C4;

extern "C" unsigned int OSDisableInterrupts();
extern "C" void OSRestoreInterrupts(unsigned int);
extern "C" void AXFreeVoice(void* p);
extern "C" void ARQRemoveRequest(void* p);
extern "C" int SNDDRV_dmapost(int a, int b, int c, int d, int e);

struct Unk8069E200_RecA {
    char pad0[0x18];
    int m18;
    char m1C;
    char pad1D[2];
    unsigned char m1F;
    char pad20[4];
    unsigned char m24;
    unsigned char m25;
    char pad26[0x16];
    void* m3C;
};

struct Unk8069E200_RecB {
    int m0;
    int m4;
    char pad8[0x1C];
    int m24;
    int m28;
    int m2C;
    int m30;
    unsigned int m34;
    char pad38[8];
};

struct Unk8069E200 {
    Unk8069E200_RecA recA[0x30];
    char padC00[0x1400 - 0xC00];
    float m1400;
    char pad1404[0xC];
    void* mVoices[16];
    char pad1450[0x1AC0 - 0x1450];
    int m1AC0;
    char pad1AC4[0x9DE8 - 0x1AC4];
    int m9DE8;
    int m9DEC;
    char pad9DF0[0x9E30 - 0x9DF0];
    Unk8069E200_RecB recB[64];
    char padAE30[0xC048 - 0xAE30];
    unsigned char mC048;
    unsigned char mC049;
    char padC04A[6];
    void* mBufs[6];
    unsigned int mAligned[6];
};
extern Unk8069E200 lbl_8069E200;

extern "C" void SNDSYS_linkmaincpumixer();
extern "C" void SNDARAM_restore();
extern "C" int AXRegisterAuxACallback(int a, int b);
extern "C" void OSCancelThread(void* p);
extern "C" void fn_80417808();
extern "C" void SNDSYS_leavecritical();
extern "C" void SNDSYS_entercritical();
extern "C" void SNDDRV_restoremixer();
extern char lbl_8069FCC8[];

struct Unk803AEE98 {
    unsigned int w0;
    void (*fn)(int);
    unsigned char m10;
    unsigned char m11;
    unsigned char m12;
};
extern void (*lbl_806AA4C8)(Unk803AEE98*);
extern void (*lbl_806AA4CC)();

extern "C" int ARGetBaseAddress();

extern "C" void SNDDRV_freeaxvoice(int v, int k);
extern unsigned short lbl_80625468[][4];

struct Unk803AE618 {
    char pad0[0x14];
    int m14;
};

extern "C" void fn_803AE0E0(Unk803AE0E0* p);

extern "C" void ARQPostRequest(void* p, int t, int z, int a, int b, int c, int aligned, void (*cb)(Unk8069E200_RecB*));

extern "C" void SNDSYS_service()
{
    if (sndgs.installed) {
        int i;
        for (i = 0; i < sndgs.numserverclients; i++)
            sndgs.serverclient[i]();
    }
}

extern "C" void fn_803AC904(DtorAt0* s, int chrg)
{
    fn_803B9DDC(s, chrg);
}

extern "C" void fn_803AC924(DtorAt0* s, int chrg)
{
    fn_803B9E08(s, chrg);
}

extern "C" int fn_803AC944()
{
    if (lbl_8062520C == fn_803AC904)
        return 0;
    lbl_8062520C = fn_803AC904;
    lbl_80625210 = fn_803AC924;
    return 0;
}

extern "C" unsigned char SNDBANKI_findfreekey()
{
    int i;
again:
    lbl_80625454++;
    if (lbl_80625454 == 0)
        lbl_80625454++;
    for (i = 0; i < sndgs.voicestotal; i++) {
        CHANPUB* c = &sndgs.chan[i];
        if (c->status != 0 && c->patchkey == lbl_80625454)
            goto again;
    }
    return lbl_80625454;
}

extern "C" int SNDCTRL_timemult(int shandle, int v)
{
    int setvoice;
    if (v > 0x2000)
        v = 0x2000;
    else if (v < 0x800)
        v = 0x800;
    int voice = SNDVOICEI_get(shandle);
    if (voice >= 0) {
        setvoice = -1;
        while (iSNDpatchkey(voice, &setvoice)) {
            sndgs.chan[setvoice].timemult = v;
            SNDPLATFORM_timemult(setvoice, v);
        }
    }
    return 0;
}

extern "C" int fn_803AD994()
{
    SNDSYS_service();
    return 0;
}

extern "C" void fn_803AD9B8()
{
}

extern "C" int SNDSYS_vectortoreal()
{
    lbl_8069C93C = (void (*)())fn_803AD9B8;
    if (lbl_80625460 == 0) {
        fn_80400C1C(fn_803AD994, 0, 1);
        lbl_80625460 = 1;
    }
    SYSTEM_addexit(fn_803B3E88);
    return 0;
}

extern "C" void iSNDcalcvol(int voice);
extern int lbl_80625448;

extern "C" int SNDvol(int shandle, int vol)
{
    int voice = SNDVOICEI_get(shandle);
    if (voice >= 0) {
        int setvoice = -1;
        float v = (float)vol * 0.007874016f;
        while (iSNDpatchkey(voice, &setvoice)) {
            int changed = 0;
            int i;
            CHANPUB* c = &sndgs.chan[setvoice];
            for (i = 0; i < c->numchan; i++) {
                int vc = c->voices[i];
                CHANPUB* p = &sndgs.chan[vc];
                p->fadePer100Hz = 0.0f;
                if (p->programmedVol != v) {
                    p->programmedVol = v;
                    iSNDcalcvol(vc);
                    changed = 1;
                    Snd::Hal::SetVol(vc);
                }
            }
            if (changed) {
                if (c->rendermode & 4) {
                    for (i = 0; i < lbl_80625448; i++)
                        SNDPLATFORM_setfxlevel(voice, i);
                } else {
                    SNDPLATFORM_setfxlevel(voice, 0);
                }
            }
        }
    }
    return voice;
}

extern "C" void SNDDRV_DSPMixerResetChannel(Unk803ADD08* p)
{
    p->m4 = 0x50000000;
    p->m8 = 0;
    p->mC = 0;
    p->m10 = 0;
    p->m54 = 0;
    p->m50 = 0;
    p->m4C = 0;
    p->m48 = 0;
    p->m44 = 0;
    p->m40 = 0;
    p->m3C = 0;
    p->m38 = 0;
    p->m34 = 0;
    p->m30 = 0;
}

extern "C" void SNDDRV_DSPMixerSetPan(Unk803AE0E0* o, float* in, unsigned int v, int flag)
{
    Unk803ADD08* e = &lbl_8069CBC4[o->m18];
    if (lbl_8069E1C4 == 4) {
        unsigned int idx = v >> 8;
        if (flag == 0) {
            e->m3A = lbl_80625468[idx][1];
            e->m52 = lbl_80625468[idx][3];
            e->m4E = lbl_80625468[idx][2];
            e->m36 = lbl_80625468[idx][0];
        } else {
            e->m36 = 0;
            e->m4E = 0;
            e->m52 = 0;
            e->m3A = 0;
        }
    } else {
        e->m3A = (short)(in[0] * 32767.0f);
        e->m36 = (short)(in[1] * 32767.0f);
    }
    e->m4 |= 0x40000000;
}

extern "C" void SNDDRV_DSPMixerSetAuxBus(Unk803AE0E0* p, short v, int kind)
{
    Unk803ADD08* e = &lbl_8069CBC4[p->m18];
    if (kind == 1) {
        e->m46 = v;
        e->m42 = v;
        if (lbl_8069E1C4 == 4) {
            e->m4A = v;
            e->m56 = v;
        }
    } else if (kind == 2) {
        e->m52 = v;
        e->m4E = v;
        if (lbl_8069E1C4 == 3)
            e->m56 = v;
    }
    e->m4 |= 0x40000000;
}

extern "C" void SNDDRV_DSPMixerInit(int v)
{
    int i;
    for (i = 0; i < 64; i++)
        SNDDRV_DSPMixerResetChannel(&lbl_8069CBC4[i]);
    lbl_8069E1C4 = v;
}

struct Unk803ADEEC_Vpb {
    char pad0[0x18];
    int m18;
    int m1C;
    char pad20[0x124];
    short m144;
    char pad146[4];
    unsigned short m14A;
    short m14C;
    unsigned short m14E;
    short m150;
    unsigned short m152;
    short m154;
    unsigned short m156;
    short m158;
    unsigned short m15A;
    short m15C;
    unsigned short m15E;
    short m160;
    unsigned short m162;
    short m164;
    unsigned short m166;
    short m168;
    unsigned short m16A;
    short m16C;
    char pad16E[0x2E];
    short m19C;
    short m19E;
};

extern "C" void SNDDRV_DSPMixerInitChannel(Unk803ADEEC_Vpb* vpb, int flags, int a2, short a3, int a4, float* a5, unsigned int a6, int a7)
{
    Unk803ADD08* e = &lbl_8069CBC4[vpb->m18];
    e->m0 = (int)vpb;
    e->m4 = flags & 4;
    int mask;
    mask = 0;
    e->m8 = a2;
    SNDDRV_DSPMixerSetAuxBus((Unk803AE0E0*)vpb, a3, 1);
    e->m40 = e->m42;
    e->m44 = e->m46;
    e->m48 = e->m4A;
    SNDDRV_DSPMixerSetPan((Unk803AE0E0*)vpb, a5, a6, a7);
    e->m34 = e->m36;
    e->m38 = e->m3A;
    e->m4C = e->m4E;
    e->m50 = e->m52;
    if (e->m4 & 4)
        e->m30 = 0;
    else
        e->m30 = a2;
    unsigned int en = OSDisableInterrupts();
    unsigned short t;
    vpb->m19C = e->m30;
    vpb->m19E = 0;
    vpb->m14A = e->m34;
    if (vpb->m14A != 0)
        mask |= 1;
    vpb->m14C = 0;
    vpb->m14E = e->m38;
    if (vpb->m14E != 0)
        mask |= 2;
    vpb->m150 = 0;
    vpb->m152 = e->m40;
    if (vpb->m152 != 0)
        mask |= 0x10;
    vpb->m154 = 0;
    vpb->m156 = e->m44;
    if (vpb->m156 != 0)
        mask |= 0x20;
    vpb->m158 = 0;
    vpb->m15A = e->m4C;
    if (vpb->m15A != 0)
        mask |= 0x200;
    vpb->m15C = 0;
    vpb->m15E = e->m50;
    if (vpb->m15E != 0)
        mask |= 0x400;
    vpb->m160 = 0;
    vpb->m162 = e->m54;
    if (vpb->m162 != 0)
        mask |= 0x1000;
    vpb->m164 = 0;
    vpb->m166 = e->m3C;
    if (vpb->m166 != 0)
        mask |= 4;
    vpb->m168 = 0;
    vpb->m16A = e->m48;
    if (vpb->m16A != 0)
        mask |= 0x80;
    vpb->m144 = mask;
    vpb->m16C = 0;
    vpb->m1C = vpb->m1C | 0x212;
    OSRestoreInterrupts(en);
}

extern "C" void fn_803AE0E0(Unk803AE0E0* p)
{
    lbl_8069CBC4[p->m18].m0 = 0;
}

extern "C" void fn_803AE0FC(Unk803AE0E0* p, int v)
{
    Unk803ADD08* e = &lbl_8069CBC4[p->m18];
    int t = e->m4;
    e->m8 = v;
    e->m4 = t | 0x10000000;
}

extern "C" void SNDDRV_freenextframe()
{
    int i;
    if (lbl_8069E200.mC049 != 0) {
        for (i = 0; i <= 0x2f; i++) {
            if (lbl_8069E200.recA[i].m1F != 0) {
                AXFreeVoice(lbl_8069E200.recA[i].m3C);
                lbl_8069E200.recA[i].m1F = 0;
                lbl_8069E200.mC049--;
                lbl_8069E200.recA[i].m3C = 0;
                if (lbl_8069E200.mC049 == 0)
                    break;
            }
        }
    }
}

extern "C" void fn_803AE5EC(int v)
{
    SNDVOICEI_free(v + sndgs.sso.set.voicesdsp);
}

extern "C" void SNDDRV_callbackdropvoice(Unk803AE618* o)
{
    int idx = o->m14;
    CHANPUB* chans = sndgs.chan;
    CHANPUB* mc = &chans[idx];
    int m = mc->masterchan;
    int base;
    if (m == -1) base = idx; else base = m;
    mc = &chans[base];
    int i;
    for (i = 0; i < mc->numchan; i++) {
        int v = mc->voices[i];
        if (lbl_8069E200.recA[v].m3C != 0) {
            if (v == o->m14)
                SNDDRV_freeaxvoice(v, 1);
            else
                SNDDRV_freeaxvoice(v, 2);
            lbl_8069E200.recA[mc->voices[i]].m24 = 0;
        }
    }
}

extern CHANPUB* lbl_8069CA48; /* sndgs.chan (sndgs + 0x1CC), named by its own symbols.txt entry */

extern "C" void SNDDRV_freeaxvoice(int v, int mode)
{
    Unk8069E200_RecA* r = &lbl_8069E200.recA[v];
    CHANPUB* c = &lbl_8069CA48[v];
    void* p = r->m3C;
    fn_803AE0E0((Unk803AE0E0*)p);
    r->m18 = -1;
    if (c->status != 0)
        SNDVOICEI_free(v);
    if (mode == 0) {
        if (p != 0) {
            AXFreeVoice(p);
            r->m3C = 0;
        }
    } else if (mode == 2) {
        /* the signed read keeps the compiler from threading this exit past the m25 retest below */
        if ((signed char)r->m25 == 0 && p != 0) {
            r->m1F = 1;
            lbl_8069E200.mC049++;
        }
    } else if (mode == 1) {
        if (r->m1F == 1) {
            r->m1F = 0;
            lbl_8069E200.mC049--;
        }
        r->m3C = 0;
    }
    if (r->m25 == 0)
        r->m25 = 1;
}

extern "C" int SNDDRV_dmaslotenter(int v)
{
    int i = 0;
    unsigned int en = OSDisableInterrupts();
    while (i < 64 && lbl_8069E200.recB[i].m0 != -1)
        i++;
    lbl_8069E200.recB[i].m0 = v;
    OSRestoreInterrupts(en);
    return i;
}

extern "C" void SNDDRV_dmcallback(Unk8069E200_RecB* r)
{
    int i;
    lbl_8069E200.m9DE8 = r->m4;
    for (i = 0; i <= 63; i++) {
        if (lbl_8069E200.recB[i].m0 == r->m4) {
            lbl_8069E200.recB[i].m0 = -1;
            return;
        }
    }
}

extern "C" int SNDDRV_dmapost(int a, int b, int c, int d, int idx)
{
    unsigned int en = OSDisableInterrupts();
    int t;
    int aligned;
    int slot;
    if (idx == -1) {
        slot = SNDDRV_dmaslotenter(lbl_8069E200.m9DEC);
        t = lbl_8069E200.m9DEC;
        lbl_8069E200.recB[slot].m24 = a;
        lbl_8069E200.recB[slot].m28 = b;
        lbl_8069E200.recB[slot].m2C = c;
        int size = d + 0x1f;
        lbl_8069E200.recB[slot].m30 = size & ~0x1f;
        aligned = size;
        lbl_8069E200.recB[slot].m34 = sndgs.audiotick;
    } else {
        slot = idx;
        aligned = d + 0x1f;
        t = lbl_8069E200.recB[slot].m0;
    }
    ARQPostRequest(&lbl_8069E200.recB[slot].m4, t, 0, a, b, c, aligned & ~0x1f, SNDDRV_dmcallback);
    OSRestoreInterrupts(en);
    return lbl_8069E200.m9DEC++;
}

extern "C" void SNDDRV_restoredma(int id)
{
    int i;
    unsigned int en = OSDisableInterrupts();
    for (i = 0; i <= 63; i++) {
        if (lbl_8069E200.recB[i].m0 == id) {
            ARQRemoveRequest(&lbl_8069E200.recB[i].m4);
            SNDDRV_dmapost(lbl_8069E200.recB[i].m24, lbl_8069E200.recB[i].m28, lbl_8069E200.recB[i].m2C, lbl_8069E200.recB[i].m30, i);
        }
    }
    OSRestoreInterrupts(en);
}

struct Unk803AED08_Mix {
    char pad0[0x14];
    int* m14;
};
extern Unk803AED08_Mix lbl_806AA2D4;
extern int lbl_8069E1E0;
extern unsigned char lbl_8069C8BB;
extern int lbl_80625C68;
extern char lbl_8069FFE0[];
extern "C" unsigned int OSEnableInterrupts();
extern "C" void OSWakeupThread(void* p);
extern "C" int fn_803B1364(int i);

extern "C" void SNDDRV_audiocallback(int** outs)
{
    unsigned int en = OSEnableInterrupts();
    if (lbl_8069E200.mC048 != 0) {
        int i;
        for (i = 0; i < lbl_80625448; i++) {
            if (lbl_806AA2D4.m14[i] != 0) {
                float* buf = (float*)fn_803B1364(i);
                int* src = outs[0];
                int j;
                lbl_8069E1E0 = (int)buf;
                for (j = 0; j <= 0x9F; j++)
                    buf[j] = (float)src[j];
            }
        }
        if (lbl_8069C8BB <= 3) {
            Snd::Util::MemCpy(outs[1], (void*)lbl_8069E200.mAligned[0], 0x280);
            Snd::Util::MemCpy(outs[0], (void*)lbl_8069E200.mAligned[1], 0x280);
        } else {
            Snd::Util::MemCpy(outs[1], (void*)lbl_8069E200.mAligned[0], 0x280);
            Snd::Util::MemCpy(outs[2], (void*)lbl_8069E200.mAligned[2], 0x280);
            Snd::Util::MemCpy(outs[0], (void*)lbl_8069E200.mAligned[3], 0x280);
            Snd::Util::MemCpy(outs[3], (void*)lbl_8069E200.mAligned[1], 0x280);
        }
    }
    OSWakeupThread(lbl_8069FFE0);
    lbl_80625C68++;
    OSRestoreInterrupts(en);
}

extern "C" void SNDDRV_initmixer()
{
    Unk803AEE98 b;
    int i;
    SNDSYS_linkmaincpumixer();
    b.m10 = sndgs.sso.set.voicesmaincpu;
    if (sndgs.sso.set.outputchannels != 1)
        b.m11 = sndgs.sso.set.outputchannels;
    else
        b.m11 = 2;
    b.w0 = sndgs.sso.set.outputrate;
    b.fn = fn_803AE5EC;
    b.m12 = 0;
    lbl_806AA4C8(&b);
    SNDSYS_entercritical();
    for (i = 0; i < b.m11; i++) {
        void* p = SNDMEMI_allocz(0x2a0);
        lbl_8069E200.mBufs[i] = p;
        lbl_8069E200.mAligned[i] = ((unsigned int)p + 0x1f) & ~0x1f;
    }
    SNDSYS_leavecritical();
    lbl_8069E200.mC048 = 1;
}

extern "C" void SNDDRV_restoremixer()
{
    int i;
    lbl_8069E200.mC048 = 0;
    SNDSYS_entercritical();
    for (i = 0; i < sndgs.sso.set.outputchannels; i++)
        SNDMEMI_free(lbl_8069E200.mBufs[i]);
    SNDSYS_leavecritical();
    lbl_806AA4CC();
}

extern "C" void SNDSYSI_100hzserver();
extern "C" void SNDSYSI_variabletimerservice();
extern "C" void OSSleepThread(void* p);
extern void (*lbl_806AA4D0)(void*, int);
extern char lbl_806AA268[];
extern float lbl_80625C6C;
extern int lbl_8069CAD4;

/* Packet-fed voice: per-channel packet pointers (m25C4), 0x600-byte DMA buffers (mBuf), counters. */
struct Unk803AF014_Voice {
    char pad0[0x2580];
    int mBuf[6];
    signed char m2598;
    unsigned char m2599;
    char pad259A[2];
    int m259C[6];
    char pad25B4[4];
    unsigned short m25B8[6];
    int m25C4[6];
    int m25DC;
    int m25E0;
};

struct Unk803AF014_Chan {
    char pad0[0x23];
    unsigned char numchan;
};

extern "C" void DCFlushRange(int a, int b);

extern "C" void SNDDRV_fillbufwithpackets(int h, Unk803AF014_Voice* s, Unk803AF014_Chan* c)
{
    int sp8[6];
    int sp20, sp24;
    int remaining = 0x600;
    int i;
    do {
        int n, next;
        if (s->m25C4[0] == 0) {
            for (i = 0; i < c->numchan; i++)
                s->m25C4[i] = (int)SNDPKTPLAYI_get(h, i, &sp20, &sp24);
            s->m25E0 = 0;
            s->m25DC = sp20 << 1;
        }
        n = remaining;
        if (s->m25C4[0] != 0) {
            int avail = s->m25DC - s->m25E0;
            if (avail <= 0) {
                s->m25C4[0] = 0;
                continue;
            }
            if (remaining > avail)
                n = avail;
            next = remaining - n;
            s->m25B8[s->m2599] += n >> 1;
            for (i = 0; i < c->numchan; i++)
                sp8[i] = s->m25C4[i] + s->m25E0;
            s->m25E0 += n;
        } else {
            next = 0;
            for (i = 0; i < c->numchan; i++)
                sp8[i] = lbl_8069E200.m1AC0;
        }
        for (i = 0; i < c->numchan; i++) {
            char* bp = (char*)s->mBuf[i] + 0x600;
            Snd::Util::MemCpy(bp - remaining, (void*)sp8[i], n);
        }
        remaining = next;
    } while (remaining > 0);
    for (i = 0; i < c->numchan; i++) {
        DCFlushRange(s->mBuf[i], 0x600);
        SNDDRV_dmapost(1, s->mBuf[i], s->m259C[i] + s->m2599 * 3 * 0x200, 0x600, -1);
    }
}

/* SND server loop; the lbl_8069CAD4 store goes through a local pointer, as in retail. */
extern "C" void SNDDRV_thread()
{
    for (;;) {
        ((void (*)())lbl_8062520C)();
        if (sndgs.profileenter != 0)
            sndgs.profileenter();
        if (lbl_8069E200.mC048 != 0) {
            lbl_806AA4D0(lbl_806AA268, 0xA0);
            SNDPKTPLAYI_flushcallbackdata();
        }
        if (lbl_80625C68 & 1) {
            SNDSYSI_100hzserver();
            int* pq = &lbl_8069CAD4;
            lbl_80625C6C = lbl_80625C6C + lbl_8069E200.m1400;
            float t = lbl_80625C6C;
            int iv;
            if (!(t < 0.0f))
                iv = (int)(t + 0.5f);
            else
                iv = (int)(t - 0.5f);
            *pq = iv;
            if (!(lbl_80625C6C <= 30.0f))
                lbl_80625C6C -= 30.0f;
            SNDSYSI_variabletimerservice();
        }
        if (sndgs.profileleave != 0)
            sndgs.profileleave();
        ((void (*)())lbl_80625210)();
        OSSleepThread(lbl_8069FFE0);
    }
}

extern "C" int SNDPLATFORM_outputcaps()
{
    sndgs.sso.cap.voicesmaincpumax = 0x20;
    sndgs.sso.cap.voicesdspmax = 0x30;
    sndgs.sso.cap.outputratemax = 0x7d00;
    sndgs.sso.cap.outputchannelsmin = 1;
    sndgs.sso.set.outputspdifmode = 1;
    sndgs.sso.cap.outputratemin = 0x7d00;
    sndgs.sso.cap.numrendermodes = 2;
    sndgs.sso.cap.rendermode[0] = 0x220;
    sndgs.sso.cap.rendermode[1] = 0x24;
    sndgs.sso.set.rendermode[0] = 0x220;
    sndgs.sso.cap.outputchannelsmax = 4;
    sndgs.sso.set.rendermode[1] = 0x24;
    sndgs.sso.set.voicesdsp = 0x30;
    sndgs.sso.set.numrendermodes = 2;
    sndgs.sso.set.voicesmaincpu = 8;
    sndgs.sso.set.maxstreams = 8;
    sndgs.sso.set.outputrate = 0x7d00;
    sndgs.sso.set.outputchannels = 2;
    sndgs.sso.set.initaram = 1;
    int a = ARGetBaseAddress() + 0x100;
    sndgs.sso.set.arampooladdr = a;
    sndgs.sso.set.arampoolsize = 0x800000 - a;
    return 0;
}

extern "C" int SNDPLATFORM_outputset()
{
    SNDSYSSET s = sndgs.sso.set;
    if (sndgs.installed != 0)
        sndgs.sso.set = sndgs.prevset;
    s.outputrate = 0x7D00;
    if (s.outputchannels < sndgs.sso.cap.outputchannelsmin)
        s.outputchannels = sndgs.sso.cap.outputchannelsmin;
    if (s.outputchannels > sndgs.sso.cap.outputchannelsmax)
        s.outputchannels = sndgs.sso.cap.outputchannelsmax;
    if (s.voicesmaincpu > sndgs.sso.cap.voicesmaincpumax)
        s.voicesmaincpu = sndgs.sso.cap.voicesmaincpumax;
    if (s.voicesdsp > sndgs.sso.cap.voicesdspmax)
        s.voicesdsp = sndgs.sso.cap.voicesdspmax;
    if (s.maxstreams > 0x20)
        s.maxstreams = 0x20;
    if (sndgs.installed != 0)
        return 0;
    sndgs.voicestotal = s.voicesmaincpu + s.voicesdsp;
    sndgs.sso.set = s;
    return 0;
}

extern "C" int SNDPLATFORM_restore()
{
    AXRegisterAuxACallback(0, 0);
    if (lbl_8069E200.mC048 != 0)
        SNDDRV_restoremixer();
    SNDARAM_restore();
    if (sndgs.incritical != 0) {
        do {
            SNDSYS_leavecritical();
        } while (sndgs.incritical != 0);
    }
    OSCancelThread(lbl_8069FCC8);
    fn_80417808();
    return 0;
}

extern "C" void fn_803AFD44(int flags, int* a, int* b)
{
    if (flags & 0x200) {
        *a = 0;
        *b = sndgs.sso.set.voicesdsp;
    } else if (flags & 4) {
        *a = sndgs.sso.set.voicesdsp;
        *b = sndgs.voicestotal;
    }
}

extern "C" int SNDPLATFORM_asyncresolvetimbre(SNDIPATCHHEADER* pph, int base, int* poff)
{
    if (*poff == 0)
        *poff = pph->sampleoffset[0];
    *pph->paudioramaddr = base + (pph->sampleoffset[0] - *poff);
    return 0;
}

extern char lbl_8069E23C[];
extern "C" void SNDMIX_setwetgain(int idx, int ch, float v);

int SNDPLATFORM_setfxlevel(int voice, int bus)
{
    CHANPUB* c = &sndgs.chan[voice];
    if (c->rendermode & 0x200) {
        int i = 0;
        float v = c->finalvol * (&c->pFxVolume->fxLevel)[bus];
        for (; i < c->numchan; i++) {
            if (i == 5)
                v = 0.0f;
            ((void (*)(Unk803AE0E0*, unsigned short, int))SNDDRV_DSPMixerSetAuxBus)(*(Unk803AE0E0**)(lbl_8069E23C + (c->voices[i] << 6)), (unsigned short)(int)(v * 32767.0f), 1);
        }
    } else if (c->rendermode & 4) {
        int i = 0;
        float v = c->finalvol * (&c->pFxVolume->fxLevel)[bus];
        for (; i < c->numchan; i++) {
            if (i == 5)
                v = 0.0f;
            SNDMIX_setwetgain(c->voices[i] - sndgs.sso.set.voicesdsp, bus, v);
        }
    } else {
        return -15;
    }
    return 0;
}
