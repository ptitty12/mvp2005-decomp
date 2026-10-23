/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803B4BEC-0x803B6AF8 (code-only unit, 46 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */
#include "snd/cmn/sndcmn.h"
#include "snd/copy_callback.h"

extern SNDPACKETSTATE lbl_806AA500;
/* Existing retail symbol for snd packet-state pointer table at +0x304, 32 entries. */
extern SNDPACKETCHAN* lbl_806AA804[32];

struct Unk803B5E44_Sub {
    int (*m0)(Unk803B5E44_Sub*, int, float*, float*, int);
};

struct Unk803B5E44 {
    void* m0;
    int m4;
    Unk803B5E44_Sub* m8;
    int mC;
    int m10;
    int m14;
    char pad18[2];
    unsigned char m1A;
    char pad1B;
    float m1C[12];
};

extern "C" int SFILTER_hpfFIR8(Unk803B5E44* o, int a, float* b, float* c);
extern "C" int SFILTER_lpfRC(Unk803B5E44* o, int a, float* b, float* c);
extern "C" void fn_803B6428(float* p);
extern "C" int SFILTER_rsf(void* o);
extern "C" int rsflc(void* o);
extern "C" void SFILTER_unpackfinit();
extern "C" void SFILTER_unpacklfinit();
extern "C" void SFILTER_unpackpfinit();
extern "C" void SFILTER_unpackmtfinit();
extern "C" void SFILTER_unpackmtpfinit();
extern "C" void SFILTER_unpackmtlfinit();
extern "C" void SFILTER_unpackxafinit();
extern "C" void SFILTER_unpackxalfinit();
extern "C" void SFILTER_unpackxapfinit();
extern float lbl_80609BA0;
extern float lbl_80609BF4;
extern int lbl_806AA2D4[];

struct Unk806AA88C_Entry {
    unsigned int m0;
    unsigned int m4;
};

struct Unk806AA88C {
    unsigned short m0;
    unsigned short m2;
    unsigned int m4;
    unsigned int m8;
    Unk806AA88C_Entry* mC;
};

struct Unk803B5F9C {
    char pad0[8];
    Unk803B5F9C* m8;
    char padC[4];
    Unk803B5F9C* m10;
    char pad14[4];
    unsigned short m18;
};
extern Unk806AA88C lbl_806AA88C;
extern int lbl_806AA884;
extern int lbl_806AA888;
extern float lbl_80609B98;

extern "C" void SNDSYS_entercritical();
extern "C" void SNDSYS_leavecritical();
extern "C" int fn_803B0360();
extern "C" int fn_803B0714(int h);

extern "C" int SFILTER_ft24_32(int unused, int n, float* in, int* out);

struct Unk803B5E2C {
    void* m0;
    int m4;
};

extern "C" void SNDI_fir8(float* st, int n, float* in, float* out);

struct Unk803B5EC4 {
    void* m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    char pad18[3];
    char m1B;
    float m1C[8];
};

struct Unk803B66CC {
    char pad[0x1C];
    int m1C;
};

struct Unk803B69AC {
    void* m0;
    int m4;
    char pad8[0x18];
    int m20;
    void* m24;
    short m28;
    short m2A;
    float m2C[4];
};

extern "C" int SNDstop(int shandle);

extern "C" void SNDSYS_entercritical();
extern "C" void SNDSYS_leavecritical();
struct Unk803B0714;
extern "C" int SNDPLATFORM_packetplaycreate(int slot, Unk803B0714* pmem);
extern "C" int fn_803B0360();
extern "C" int fn_803B042C(int h, int voice, int timemult, int lowpass, int highpass, SNDSAMPLEFORMAT* pssf, SNDSAMPLEATTR* pssa);
extern "C" int fn_803B58E8(int* prenderindex, SNDIPATCHHEADER* pph);
extern "C" void fn_803AFD44(int rendermode, int* pmin, int* pmax);
extern "C" void SetDefaultAzimuths(CHANPUB* pvoice);

/* SNDPKTPLAY_create (source structure from the ported spktplay.c reference). */

/* SNDPKTPLAY_submit */

/* SNDPKTPLAYI_get */

/* SNDPKTPLAY_start */

extern "C" int SNDPKTPLAYI_voicetopackethandle(int voice)
{
    int i;
    for (i = 0; i < sndgs.sso.set.maxstreams; i++) {
        SNDPACKETCHAN* p = lbl_806AA500.ppc[i];
        if (p != 0 && SNDVOICEI_get(p->shandle) == voice)
            return i;
    }
    return -1;
}

int SNDPKTPLAYI_gethighchannel(int chan, int n)
{
    int i;
    int best = -1;
    int idx = -1;
    for (i = 0; i < n; i++) {
        int v = sndgs.chan[chan].voices[i];
        if (v > best) {
            best = v;
            idx = i;
        }
    }
    return idx;
}

int SNDPKTPLAYI_overhead(int n)
{
    return (n << 5) + 0x9c;
}

extern "C" int SNDPKTPLAY_overhead(int n)
{
    int a = SNDPKTPLAYI_overhead(n);
    return a + fn_803B0360();
}

extern "C" int SNDPKTPLAY_create(void (*preleasefunc)(void*, void*), void (*pframesfunc)(int, int, void*), void* pclientdata, void* pmem, int memsize)
{
    SNDPACKETCHAN* ppchan;
    int packetinstancehandle;
    void* pplatformmem;
    int i = 0;

    SNDSYS_entercritical();

    for (; i < sndgs.sso.set.maxstreams; i++) {
        if (lbl_806AA804[i] == 0) {
            goto allocated;
        }
    }

    SNDSYS_leavecritical();
    return -9;

allocated:
    packetinstancehandle = SNDPLATFORM_packetplaycreate(i, (Unk803B0714*)pmem);
    if (packetinstancehandle < 0) {
        SNDSYS_leavecritical();
        return -6;
    }

    pplatformmem = pmem;
    pmem = (void*)((int)pmem + fn_803B0360());
    ppchan = (SNDPACKETCHAN*)pmem;
    memsize -= fn_803B0360();

    ppchan->maxpackets = (memsize - 0x9C) / sizeof(SNDPACKETENTRY);
    ppchan->pplatformmem = pplatformmem;
    ppchan->preleasefunc = preleasefunc;
    ppchan->pframesfunc = pframesfunc;
    ppchan->pclientdata = pclientdata;
    ppchan->shandle = -1;
    lbl_806AA500.ppc[i] = ppchan;

    SNDSYS_leavecritical();
    return i;
}

extern "C" int SNDPKTPLAY_start(int packetinstancehandle, SNDSAMPLEFORMAT* pssf, SNDSAMPLEATTR* pssa, SNDPLAYOPTS* pspo)
{
    SNDIPATCHHEADER ph;
    SNDPACKETCHAN* ppchan;
    int minvoicerange;
    int maxvoicerange;
    int rendermodeindex = 0;
    int rendermode;
    int shandle;
    int voice;
    int ret;
    int voicesneeded;
    int i;
    int done = 0;

    SNDI_checkplayopts(pspo);

    voicesneeded = pssf->channels;
    ph.numchan = pssf->channels;
    ph.rendermode = pssa->rendermode;
    rendermodeindex = 0;

    ppchan = lbl_806AA500.ppc[packetinstancehandle];
    ppchan->sampleformat = *pssf;
    ppchan->lastpackethandle = 0;
    ppchan->outstandingframes = 0;
    ppchan->outstandingplatformframes = 0;
    ppchan->masterCurrentPacket = 0;
    ppchan->masterReleasePacket = 0;
    ppchan->submitpacket = 0;
    ppchan->releasepacket = 0;

    for (i = 0; i < ppchan->sampleformat.channels; i++) {
        ppchan->numPacketsProcessed[i] = 0;
        ppchan->outstandingpackets[i] = 0;
        ppchan->playpacket[i] = 0;
        ppchan->ptsdata[i] = pssa->ptsdata[i];
    }

    while (!done && (rendermode = fn_803B58E8(&rendermodeindex, &ph))) {
        CHANPUB* pVoice;
        fn_803AFD44(rendermode, &minvoicerange, &maxvoicerange);

        voice = SNDVOICEI_alloc(voicesneeded, 0x65, &shandle, minvoicerange, maxvoicerange);
        if (voice < 0) {
            ppchan->shandle = -9;
            continue;
        }
        ppchan->shandle = shandle;

        ppchan->masterpacketchan = SNDPKTPLAYI_gethighchannel(voice, pssf->channels);

        pVoice = &sndgs.chan[voice];
        pVoice->sustainend = -1;
        pVoice->frames = 0;
        pVoice->samplerate = pssf->samplerate;
        pVoice->samplerep = pssf->samplerep;
        pVoice->numchan = pssf->channels;

        if (pssf->channels == 1) {
            pVoice->azimuth = pspo->azimuth;
        } else {
            SetDefaultAzimuths(pVoice);
        }

        pVoice->bhandle = -1;
        pVoice->builtinazimuth = 0;
        pVoice->initialdetune = pssa->detune;
        pVoice->progpitch = pspo->pitchmult;
        pVoice->pitchbend = pspo->bend;
        pVoice->progfxlevel = pspo->fxlevel0;
        pVoice->pFxVolume->fxLevel = pspo->fxlevel0 * (1.0f / 127.0f);
        pVoice->bendrange = pssa->bendrange * 100;
        pVoice->patchkey = 0;
        pVoice->ismaster = 0;
        pVoice->pbendtable = 0;
        pVoice->ppitchlfo = 0;
        pVoice->programmedVol = pspo->vol * (1.0f / 127.0f);

        for (i = 0; i < pssf->channels; i++) {
            pVoice->azimuthOffsets[i] = pssa->azimuth[i] - sndgs.srcchancfg3d[pssf->channels - 1][i];

            CHANPUB* pSourceVoice = &sndgs.chan[pVoice->voices[i]];
            pSourceVoice->rendermode = rendermode;
            pSourceVoice->drylevel = pspo->drylevel;
            pSourceVoice->builtinvol = pssa->vol;
            pSourceVoice->programmedVol = pVoice->programmedVol;
            pSourceVoice->pvoltable = 0;
            pSourceVoice->numenvelopes = 1;
            pSourceVoice->releaseenvelope = 0;
            pSourceVoice->curenvelope = 0;
            pSourceVoice->fadePer100Hz = 0.0f;
            pSourceVoice->envvol = 0x7F0000;
            pSourceVoice->envduration = 0x7FFFFFFF;
            pSourceVoice->envpertick = 0;
            pSourceVoice->paenvelope = 0;
            pSourceVoice->pvollfo = 0;
            Snd::Util::FastVol(pSourceVoice);

            if (i == 5) {
                pSourceVoice->isLfe = 1;
            } else {
                pSourceVoice->isLfe = 0;
            }
            pSourceVoice->elevation = 0;
        }

        if (pssf->channels == 1) {
            pVoice->elevation = pspo->elevation;
        }

        for (i = 0; i < pssf->channels; i++) {
            CHANPUB* pSourceVoice = &sndgs.chan[pVoice->voices[i]];
            pSourceVoice->azimuth += pVoice->azimuthOffsets[i];
        }
        pVoice->detunepitch = 0;

        iSNDcalcpitch(voice);
        ret = fn_803B042C(packetinstancehandle, voice, pspo->timemult, pspo->lowpasscutoff, pspo->highpasscutoff, pssf, pssa);
        if (ret >= 0) {
            break;
        }

        for (i = 0; i < voicesneeded; i++) {
            SNDVOICEI_free(sndgs.chan[voice].voices[i]);
        }

        ppchan->shandle = ret;
    }

    return ppchan->shandle;
}

extern "C" int SNDPKTPLAY_submit(int packetinstancehandle, SNDPACKET* psp)
{
    SNDPACKETCHAN* ppchan = lbl_806AA500.ppc[packetinstancehandle];
    SNDPACKETENTRY* pentry;
    int packethandle;
    int i;

    for (i = 0; i < ppchan->sampleformat.channels; i++) {
        if (ppchan->outstandingpackets[i] >= ppchan->maxpackets - 1) {
            return -13;
        }
    }

    pentry = &ppchan->pe[ppchan->submitpacket];
    pentry->numframes = psp->numframes;
    pentry->continuation = psp->continuation;
    pentry->handle = ppchan->lastpackethandle;

    for (i = 0; i < ppchan->sampleformat.channels; i++) {
        pentry->psamples[i] = psp->psamples[i];
        ppchan->outstandingpackets[i]++;
    }

    ppchan->outstandingframes += psp->numframes;
    packethandle = ppchan->lastpackethandle;
    ppchan->lastpackethandle++;
    ppchan->submitpacket++;

    if (ppchan->submitpacket >= ppchan->maxpackets) {
        ppchan->submitpacket = 0;
    }

    return packethandle;
}

extern "C" int SNDPKTPLAY_submitspace(int h)
{
    SNDPACKETCHAN* p = lbl_806AA500.ppc[h];
    int max = 0;
    int i;
    for (i = 0; i < p->sampleformat.channels; i++) {
        if (p->outstandingpackets[i] >= max)
            max = p->outstandingpackets[i];
    }
    return p->maxpackets - max - 1;
}

extern "C" int SNDPKTPLAY_framesoutstanding(int h)
{
    SNDPACKETCHAN* p = lbl_806AA500.ppc[h];
    return p->outstandingframes + p->outstandingplatformframes;
}

extern "C" int SNDPKTPLAY_purge(int, int, int)
{
    return -15;
}

extern "C" int SNDPKTPLAY_stop(int h)
{
    SNDPACKETCHAN* p = lbl_806AA500.ppc[h];
    int i = 0;
    SNDstop(p->shandle);
    SNDPKTPLAYI_flushcallbackdata();
    SNDPKTPLAY_purge(h, 0, 0x7fffffff);
    p->shandle = -1;
    for (i = 0; i < p->sampleformat.channels; i++) {
        if (p->ptsdata[i] != 0)
            SNDMEMI_free(p->ptsdata[i]);
    }
    return 0;
}

extern "C" int SNDPKTPLAY_destroy(int h)
{
    SNDSYS_entercritical();
    fn_803B0714(h);
    lbl_806AA500.ppc[h] = 0;
    SNDSYS_leavecritical();
    return 0;
}

void* SNDPKTPLAYI_get(int packetinstancehandle, int chan, int* pframes, int* pcontinuation)
{
    SNDPACKETCHAN* ppchan = lbl_806AA500.ppc[packetinstancehandle];
    SNDPACKETENTRY* pentry;
    SNDPACKETENTRY* pspe;
    int i;
    int releaseFlag = 1;
    int increaseMaster = 1;

    for (i = 0; i < ppchan->sampleformat.channels; i++) {
        if (ppchan->numPacketsProcessed[i] <= ppchan->masterReleasePacket) {
            releaseFlag = 0;
        }
    }

    if (releaseFlag != 0) {
        pspe = &ppchan->pe[ppchan->releasepacket];
        if (ppchan->preleasefunc != 0) {
            SNDIPACKETCALLBACK* pspcb = &lbl_806AA500.spcb[lbl_806AA500.callbackindex];
            pspcb->type = 1;
            pspcb->chan = packetinstancehandle;
            pspcb->u.psamples = pspe->psamples[0];
            lbl_806AA500.callbackindex++;
        }

        ppchan->releasepacket++;
        ppchan->masterReleasePacket++;

        if (ppchan->releasepacket >= ppchan->maxpackets) {
            ppchan->releasepacket = 0;
        }
    }

    if (ppchan->outstandingpackets[chan] == 0) {
        return 0;
    }

    pentry = &ppchan->pe[ppchan->playpacket[chan]];

    *pframes = pentry->numframes;
    *pcontinuation = pentry->continuation;
    ppchan->playpacket[chan]++;
    ppchan->numPacketsProcessed[chan]++;

    i = 0;
    do {
        if (ppchan->numPacketsProcessed[i] <= ppchan->masterCurrentPacket) {
            increaseMaster = 0;
        }
        i++;
    } while (increaseMaster == 1 && i < ppchan->sampleformat.channels);

    if (increaseMaster != 0) {
        ppchan->masterCurrentPacket++;
        ppchan->outstandingframes -= pentry->numframes;
        ppchan->outstandingplatformframes += pentry->numframes;
    }

    if (ppchan->playpacket[chan] >= ppchan->maxpackets) {
        ppchan->playpacket[chan] = 0;
    }

    ppchan->outstandingpackets[chan]--;
    if (pentry->psamples[chan] == 0) {
        return (void*)-1;
    }

    return pentry->psamples[chan];
}

void SNDPKTPLAYI_freeframes(int h, int chan, int frames)
{
    SNDPACKETCHAN* p = lbl_806AA500.ppc[h];
    if (chan != p->masterpacketchan)
        return;
    p->outstandingplatformframes -= frames;
    if (p->pframesfunc == 0)
        return;
    SNDIPACKETCALLBACK* e = lbl_806AA500.spcb + lbl_806AA500.callbackindex;
    e->type = 0;
    e->chan = h;
    e->u.frames = frames;
    lbl_806AA500.callbackindex++;
}

void SNDPKTPLAYI_flushcallbackdata()
{
    int i;
    for (i = 0; i < lbl_806AA500.callbackindex; i++) {
        SNDIPACKETCALLBACK* e = &lbl_806AA500.spcb[i];
        SNDPACKETCHAN* p = lbl_806AA500.ppc[e->chan];
        if (e->type == 0)
            p->pframesfunc(e->chan, e->u.frames, p->pclientdata);
        else
            p->preleasefunc(e->u.psamples, p->pclientdata);
    }
    lbl_806AA500.callbackindex = 0;
}

void Snd::Util::ReallocBuf(void** buffer, int* curSize, int newMinSize, int sizeofDataType)
{
    int cur = *curSize;
    if (cur < newMinSize) {
        if (cur == 0) {
            *curSize = newMinSize;
        } else if (cur < newMinSize) {
            int n = cur;
            do {
                n += n;
            } while (n < newMinSize);
            *curSize = n;
        }
        void* ob = *buffer;
        void* nb = SNDMEMI_allocz(*curSize * sizeofDataType);
        if (ob != 0) {
            Snd::Util::MemCpy(nb, ob, cur * sizeofDataType);
            SNDMEMI_free(ob);
        }
        *buffer = nb;
    }
}

extern "C" int randrange(int range)
{
    if (range > 0x10000)
        range = 0x10000;
    else if (range < 0)
        range = 0;
    int r = (int)(iSNDrandom() & 0x7FFF) - 0x4000;
    return (r * range) >> 14;
}

extern "C" void fn_803B5A00(int v)
{
    lbl_806AA884 = v;
}

extern "C" void fn_803B5A0C(int v)
{
    lbl_806AA888 = v;
}

extern "C" int fn_803B5A18(SndCopyCallback v)
{
    lbl_80626518 = v;
    return 0;
}

extern "C" void SNDARAM_init(int n)
{
    SNDSYS_entercritical();
    lbl_806AA88C.mC = (Unk806AA88C_Entry*)SNDMEMI_allocz(n * 8);
    SNDSYS_leavecritical();
    lbl_806AA88C.m2 = n;
    lbl_806AA88C.m0 = 0;
}

extern "C" void SNDARAM_setpool(unsigned int a, unsigned int b)
{
    lbl_806AA88C.m8 = (a + b) & ~4;
    if (a <= 0x1f)
        a = 0x20;
    lbl_806AA88C.m4 = a;
    if ((a & 0x1f) != 0)
        lbl_806AA88C.m4 = (a + 0x1f) & ~0x1f;
    lbl_806AA88C.m4 = lbl_806AA88C.m4 >> 5;
    lbl_806AA88C.m8 = lbl_806AA88C.m8 >> 5;
}

extern "C" void SNDARAM_restore()
{
    if (lbl_806AA88C.mC != 0) {
        SNDSYS_entercritical();
        SNDMEMI_free(lbl_806AA88C.mC);
        SNDSYS_leavecritical();
        lbl_806AA88C.mC = 0;
    }
}

extern "C" void SNDARAM_constrain(unsigned int* a, unsigned int* b)
{
    unsigned int x = *a;
    unsigned int lo = lbl_806AA88C.m4;
    if (x < lo) {
        *a = lo;
        *b = *b - (lo - x);
    }
    if (*a + *b > lbl_806AA88C.m8)
        *b = lbl_806AA88C.m8 - *a;
}

/* near-miss (99.3%): 113 insns, same flow, only the loop-body temporaries differ (retail off=r8 arr=r10, ours off=r7 arr=r8); address-form, declaration-order and pointer-vs-indexed variants tried */
extern "C" int SNDARAM_alloc(int size)
{
    int sp8, spC;
    int i = 0;
    if (!(lbl_806AA88C.m0 < lbl_806AA88C.m2))
        return 0;
    size += 0x1F;
    size >>= 5;
    if (lbl_806AA88C.m0 == 0) {
        sp8 = lbl_806AA88C.m4;
        spC = lbl_806AA88C.m8 - lbl_806AA88C.m4;
        SNDARAM_constrain((unsigned int*)&sp8, (unsigned int*)&spC);
        if (size > spC)
            return 0;
        goto insert;
    }
    for (; i < lbl_806AA88C.m0; i++) {
        int off = i << 3;
        char* a = (char*)lbl_806AA88C.mC;
        if (i == 0) {
            sp8 = lbl_806AA88C.m4;
            spC = *(int*)(a + off) - sp8;
        } else {
            int po = off - 8;
            sp8 = *(int*)(a + po) + *(int*)(a + po + 4);
            spC = *(int*)(a + off) - sp8;
        }
        SNDARAM_constrain((unsigned int*)&sp8, (unsigned int*)&spC);
        if (size <= spC) {
            int k;
            for (k = lbl_806AA88C.m0; k > i; k--)
                lbl_806AA88C.mC[k] = lbl_806AA88C.mC[k - 1];
            goto insert;
        }
    }
    {
        Unk806AA88C_Entry* e = &lbl_806AA88C.mC[i - 1];
        sp8 = e->m0 + e->m4;
        spC = lbl_806AA88C.m8 - sp8;
    }
    SNDARAM_constrain((unsigned int*)&sp8, (unsigned int*)&spC);
    if (size > spC)
        return 0;
insert:
    Unk806AA88C_Entry* e = &lbl_806AA88C.mC[i];
    e->m0 = sp8;
    e->m4 = size;
    lbl_806AA88C.m0++;
    return sp8 << 5;
}

extern "C" void SNDARAM_free(unsigned int addr)
{
    unsigned int key = addr >> 5;
    int i;
    for (i = 0; i < lbl_806AA88C.m0; i++) {
        if (*(unsigned int*)((char*)lbl_806AA88C.mC + (i << 3)) == key) {
            lbl_806AA88C.m0--;
            for (; i < lbl_806AA88C.m0; i++)
                lbl_806AA88C.mC[i] = lbl_806AA88C.mC[i + 1];
            return;
        }
    }
}

extern "C" int SFILTER_ft24_32(int unused, int n, float* in, int* out)
{
    int i;
    for (i = 0; i < n; i++) {
        int v = (int)in[i];
        int r;
        if (v < -0x7fff)
            r = -0x7fff;
        else if (v > 0x7fff)
            r = 0x7fff;
        else
            r = v;
        out[i] = r;
    }
    return n;
}

extern "C" void fn_803B5E2C(Unk803B5E2C* o)
{
    o->m4 = 0;
    o->m0 = (void*)SFILTER_ft24_32;
}

extern "C" int SFILTER_hpfFIR8(Unk803B5E44* o, int a, float* in, float* out)
{
    int n;
    Unk803B5E44_Sub* s = o->m8;
    if (s != 0) {
        n = s->m0(s, a, out, in, o->m1A);
        if (n <= 0)
            return n;
    } else {
        n = a;
    }
    SNDI_fir8(o->m1C, n, in, out);
    return n;
}

extern "C" int SFILTER_createHPFFIR8(Unk803B5EC4* o)
{
    o->m8 = 0;
    o->mC = 0;
    o->m10 = 0;
    o->m14 = 0;
    o->m1B = 0;
    o->m4 = 0;
    o->m0 = (void*)SFILTER_hpfFIR8;
    fn_803B6428(o->m1C);
    return 0;
}

extern "C" void SFILTER_add(Unk803B5F9C** head, Unk803B5F9C* n)
{
    Unk803B5F9C* cur = *head;
    Unk803B5F9C* prev = 0;
    if (cur != 0 && cur->m18 < n->m18) {
        do {
            prev = cur;
            cur = prev->m8;
        } while (cur != 0 && cur->m18 < n->m18);
    }
    n->m8 = cur;
    if (cur != 0)
        cur->m10 = n;
    if (prev == 0) {
        *head = n;
        return;
    }
    prev->m8 = n;
    n->m10 = prev;
}

extern "C" void SFILTER_remove(Unk803B5F9C** head, Unk803B5F9C* n)
{
    Unk803B5F9C* p = *head;
    if (n == p) {
        *head = p->m8;
        return;
    }
    while (p->m8 != 0 && p->m8 != n)
        p = p->m8;
    if (p->m8 != 0 && p->m8 == n) {
        if (n->m8 != 0)
            n->m8->m10 = p;
        p->m8 = p->m8->m8;
    }
}

extern "C" void fn_803B6428(float* p)
{
    float z = lbl_80609B98;
    p[0] = z;
    p[1] = z;
    p[2] = z;
    p[3] = z;
    p[4] = z;
    p[5] = z;
    p[6] = z;
    p[7] = z;
}

extern "C" void SNDI_fir8(float* st, int n, float* in, float* out)
{
    int i;
    for (i = 0; i < n; i++) {
        float t7 = in[i] + st[7];
        float t0 = st[0] + st[6];
        float t1 = st[1] + st[5];
        float t2 = st[2] + st[4];
        out[i] = st[12] * st[3] + (st[11] * t2 + (st[10] * t1 + (st[8] * t7 + st[9] * t0)));
        st[7] = st[6];
        st[6] = st[5];
        st[5] = st[4];
        st[4] = st[3];
        st[3] = st[2];
        st[2] = st[1];
        st[1] = st[0];
        st[0] = in[i];
    }
}

extern "C" int SFILTER_lpfRC(Unk803B5E44* o, int n, float* in, float* out)
{
    Unk803B5E44_Sub* s = o->m8;
    if (s != 0) {
        int r = s->m0(s, n, out, in, o->m1A);
        if (r <= 0)
            return r;
    }
    float* st = o->m1C;
    int i;
    for (i = 0; i < n; i++) {
        st[0] = st[0] * st[1] + st[2] * in[i];
        out[i] = st[0];
    }
    return n;
}

/* near-miss: 13-14/16 after 1800 annealed statement orders: retail stores m1B (stb) last, the scheduler hoists it first */
extern "C" int SFILTER_createLPFRC(Unk803B5EC4* o)
{
    o->m1C[0] = lbl_80609BA0;
    o->m4 = 0;
    o->m0 = (void*)SFILTER_lpfRC;
    o->m8 = 0;
    o->mC = 0;
    o->m10 = 0;
    o->m14 = 0;
    o->m1B = 0;
    return 0;
}

extern "C" void fn_803B66CC(Unk803B66CC* p, int v)
{
    p->m1C = v;
}

extern "C" void SFILTER_rsfinit(Unk803B69AC* o, int a, int flag)
{
    o->m0 = (void*)SFILTER_rsf;
    o->m4 = 0;
    o->m20 = 0;
    o->m28 = 0;
    o->m2A = 0;
    if (flag == 0)
        o->m24 = (void*)rsflc;
    int i;
    for (i = 0; i < 4; i++)
        o->m2C[i] = lbl_80609BF4;
}

extern "C" void MIXI_initunpack16()
{
    lbl_806AA2D4[0x18 / 4] = (int)SFILTER_unpackfinit;
    lbl_806AA2D4[0x60 / 4] = 0x2c;
    lbl_806AA2D4[0x1c / 4] = (int)SFILTER_unpacklfinit;
    lbl_806AA2D4[0x64 / 4] = 0x30;
    lbl_806AA2D4[0x20 / 4] = (int)SFILTER_unpackpfinit;
    lbl_806AA2D4[0x68 / 4] = 0x34;
}

extern "C" void MIXI_initunpackmt()
{
    lbl_806AA2D4[0x30 / 4] = (int)SFILTER_unpackmtfinit;
    lbl_806AA2D4[0x78 / 4] = 0x2c;
    lbl_806AA2D4[0x38 / 4] = (int)SFILTER_unpackmtpfinit;
    lbl_806AA2D4[0x80 / 4] = 0x3c;
    lbl_806AA2D4[0x34 / 4] = (int)SFILTER_unpackmtlfinit;
    lbl_806AA2D4[0x7c / 4] = 0x30;
    lbl_806AA2D4[0x88 / 4] = 0x30;
    lbl_806AA2D4[0x84 / 4] = 0x2c;
    lbl_806AA2D4[0x44 / 4] = (int)SFILTER_unpackmtpfinit;
    lbl_806AA2D4[0x8c / 4] = 0x3c;
    lbl_806AA2D4[0x40 / 4] = (int)SFILTER_unpackmtlfinit;
    lbl_806AA2D4[0x3c / 4] = (int)SFILTER_unpackmtfinit;
}

extern "C" void MIXI_initunpackxa()
{
    lbl_806AA2D4[0x24 / 4] = (int)SFILTER_unpackxafinit;
    lbl_806AA2D4[0x6c / 4] = 0x2c;
    lbl_806AA2D4[0x70 / 4] = 0x54;
    lbl_806AA2D4[0x28 / 4] = (int)SFILTER_unpackxalfinit;
    lbl_806AA2D4[0x2c / 4] = (int)SFILTER_unpackxapfinit;
    lbl_806AA2D4[0x74 / 4] = 0x3c;
}

struct Unk803B5F18 {
    char pad0[0x1C];
    char m1C[0x34];
    float m50;
};

extern "C" void calcFIRCoeffs(void* o, int type);

extern "C" void SFILTER_modifyHPFFIR8(Unk803B5F18* o, int* buf)
{
    float f = (float)(buf[0] >> 7) / (float)(buf[1] >> 8);
    o->m50 = f;
    calcFIRCoeffs(o->m1C, 3);
}

struct Unk803B65FC {
    char pad0[0x20];
    float m20;
    float m24;
};

extern "C" void SFILTER_modifyLPFRC(Unk803B65FC* o, int* in)
{
    float f = (2.0f * (float)in[0]) / (float)in[1];
    float g = 1.0f - f;
    o->m24 = f;
    o->m20 = g;
    f = f * 0.00390625f;
    g = g * 0.00390625f;
    o->m24 = f * (float)in[2];
    o->m20 = g * (float)in[2];
}
