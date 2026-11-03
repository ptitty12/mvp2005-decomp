/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803B7DA0-0x803B998C (code-only unit, 29 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */
#include "csis/csis_internal.h"
#include <cstring>
#include "snd/sndo.h"

using namespace Csis;

extern ModList lbl_806AA89C;

extern "C" {
int fn_803B9DDC();
int fn_803B9E08();
}

struct SndPair {
    float a;
    float b;
};

struct SndT3 {
    int a;
    int b;
    int c;
};

/* 0xA8-byte sample-stream object (fn_803B826C initialises it). */
struct SndStream {
    int pad0;
    int m4;
    int m8;
    int mC;
    float* m10;
    char pad14[0x80];
    int m94;
    float m98;
    float m9C;
    void* mA0;
    float* mA4;
};

/* 0xD68-byte decoder object (fn_803B9448 initialises it). The first 0xC bytes are the bit
 * reader used by getbits/fn_803B8584. */
struct SndDec {
    unsigned char* m0;
    unsigned m4;
    int m8;
    int mC;
    int m10;
    float m14[64];
    float m114[12];
    float m144[12];
    float m174[0x144];
    char pad684[0x6C0];
    int mD44;
    unsigned char* mD48;
    int mD4C;
    short mD50;
    short mD52;
    int mD54;
    int mD58;
    int mD5C;
    int mD60;
    int mD64;
};

extern unsigned lbl_80626940[];

extern void* (*lbl_806AA884)();
extern void (*lbl_806AA888)();

/* ADPCM-style block decoder state: m0 samples left, m4/m8 history, mC byte cursor, m10 output (history at m10[-2], m10[-1]). */
struct SndRdU16 {
    int m0;
    float m4;
    float m8;
    unsigned char* mC;
    float* m10;
};

extern float lbl_80626520[8];
extern float lbl_80626540[256];
extern "C" void fn_803B7FA8(SndRdU16* a);

struct Unk803B7DA0_Parser {
    unsigned char* m0;
    int m4;
    int m8;
    int mC;
    int m10;
};

extern "C" int SNDI_gettag(Unk803B7DA0_Parser* s);
extern unsigned short lbl_8069CA54[6][6];

/* Reads the tagged header of a sample stream: ids 0..0x2A and 0x80..0xAD fill two int tables each, id 0x14 collects user-data blobs, 0xFE ends. The break is written as "if (id != 0xFE) continue; break;" to get retail's bne-over-b pair. */
extern "C" void SNDI_patchtohdrgen(int unused, unsigned char* pdata, SNDSAMPLEFORMAT* pssf, SNDSAMPLEATTR* pssa, SNDSAMPLEDESC* pssd)
{
    int hdrA[0x2B];
    int hdrB[0x2B];
    int hdrC[0x2E];
    int hdrD[0x2E];
    Unk803B7DA0_Parser p;
    int nuser = 0;
    int i;
    memset(pssf, 0, 4);
    memset(pssa, 0, 0x68);
    memset(pssd, 0, 0x1C);
    SND_attrsetdef(pssa);
    p.m0 = pdata + 8;
    for (i = 0; i <= 0x2A; i++)
        hdrB[i] = 0;
    for (i = 0; i <= 0x2D; i++)
        hdrD[i] = 0;
    hdrC[2] = 1;
    hdrC[4] = 0xBB80;
    hdrC[5] = 0;
    hdrC[0x20] = 10;
    while (SNDI_gettag(&p)) {
        if (p.m4 <= 0x2A) {
            hdrA[p.m4] = p.m8;
            hdrB[p.m4] = p.mC;
            if (p.m4 == 0x14) {
                pssa->puserdata[nuser] = (void*)p.mC;
                pssa->userdatasize[nuser] = p.m10;
                nuser++;
            }
        } else {
            unsigned k = p.m4 - 0x80;
            if (k > 0x2D) {
                if (p.m4 != 0xFE)
                    continue;
                break;
            } else {
                hdrC[k] = p.m8;
                hdrD[p.m4 - 0x80] = p.mC;
            }
        }
    }
    pssf->channels = hdrC[2];
    pssf->samplerep = hdrC[0x20];
    pssf->samplerate = hdrC[4];
    pssd->totalframes = hdrC[5];
    pssa->rendermode = 4;
    pssa->platformver = 3;
    for (i = 0; i < pssf->channels; i++)
        pssa->azimuth[i] = lbl_8069CA54[pssf->channels - 1][i];
}

extern "C" void fn_803B80C4(SndRdU16* s)
{
    unsigned char* p = s->mC;
    s->m10[-2] = s->m8;
    s->m10[-1] = s->m4;
    for (; s->m0 > 0; p = s->mC) {
        if (*p == 0xEE) {
            fn_803B7FA8(s);
            s->m0 -= 0x1C;
        } else {
            int i;
            char* tb = (char*)lbl_80626540;
            int k = (*s->mC >> 2) & 0x3C;
            s->m0 -= 0x1C;
            float c0 = *(float*)((char*)lbl_80626520 + k);
            float c1 = *(float*)((char*)lbl_80626520 + 0x10 + k);
            int row = (*s->mC++ << 6) & 0x3C0;
            for (i = 0; i < 14; i++) {
                unsigned char b = s->mC[i];
                s->m10[0] = s->m10[-2] * c1 + (s->m10[-1] * c0 + *(float*)(tb + (((b >> 2) & 0x3C) + row)));
                s->m10[1] = s->m10[-1] * c1 + (s->m10[0] * c0 + *(float*)(tb + (((b << 2) & 0x3C) + row)));
                s->m10 += 2;
            }
            s->mC += 14;
            s->m8 = s->m10[-2];
            s->m4 = s->m10[-1];
        }
    }
}

extern "C" void* fn_803B8214()
{
    return lbl_806AA884();
}

extern "C" void fn_803B8240()
{
    lbl_806AA888();
}

extern "C" SndStream* fn_803B826C(SndStream* o)
{
    o->mC = 0;
    o->m8 = 0;
    o->m4 = 0;
    o->m98 = 0.0f;
    o->m9C = 0.0f;
    return o;
}

extern "C" int fn_803B8294(SndStream* o, void* data, int bytes, int cnt)
{
    if (data == 0)
        return -1;
    if (o->m8 != 0)
        return -1;
    o->m8 = cnt;
    o->mC = bytes;
    o->mA0 = data;
    return 0;
}

extern "C" int fn_803B82D0(SndStream* o, float** pout, int want)
{
    int total = 0;
    int j;
    o->mA4 = *pout;
    if (o->m8 == 0)
        return 0;
    int n = o->m8;
    if (n > want)
        n = want;
    if (o->m4 != 0) {
        int k;
        if (o->m4 <= n)
            k = o->m4;
        else
            k = n;
        for (j = 0; j < k; j++) {
            *o->mA4 = *o->m10;
            o->mA4++;
            o->m10++;
        }
        n -= k;
        o->m8 -= k;
        o->m4 -= k;
        total = k;
    }
    int blocks = n / 28 * 28;
    int rem = n - blocks;
    o->m94 = blocks;
    if (blocks > 0) {
        total += blocks;
        fn_803B80C4((SndRdU16*)&o->m94);
        o->m8 -= blocks;
    }
    if (rem > 0) {
        float* dst = o->mA4;
        o->m94 = rem;
        o->mA4 = (float*)((char*)o + 0x24);
        j = 0;
        fn_803B80C4((SndRdU16*)&o->m94);
        o->m8 -= rem;
        o->m10 = o->mA4 + o->m94;
        o->m4 = -o->m94;
        o->mA4 = o->mA4 - 28;
        for (; j < rem; j++)
            dst[j] = o->mA4[j];
        total += rem;
    }
    if (o->m8 <= 0)
        o->m4 = 0;
    return total;
}

extern "C" SndPair fn_803B8488(SndStream* o)
{
    SndPair t;
    t.a = o->m98;
    t.b = o->m9C;
    return t;
}

extern "C" void fn_803B84B8(SndStream* o, SndPair* p)
{
    o->m98 = p->a;
    o->m9C = p->b;
}

extern "C" void* fn_803B84CC()
{
    return lbl_806AA884();
}

extern "C" void fn_803B84F8()
{
    lbl_806AA888();
}

extern "C" unsigned getbits(SndDec* s, int n)
{
    unsigned res = s->m4 & lbl_80626940[n];
    s->m4 >>= n;
    s->m8 -= n;
    if (s->m8 <= 7) {
        s->m4 |= *s->m0++ << s->m8;
        s->m8 += 8;
    }
    return res;
}

extern "C" void fn_803B8584(SndDec* s, int n)
{
    s->m4 >>= n;
    s->m8 -= n;
    if (s->m8 <= 7) {
        s->m4 |= *s->m0++ << s->m8;
        s->m8 += 8;
    }
}

extern "C" void interpolate(float* a)
{
    int i;
    for (i = 0; i <= 107; i += 2) {
        a[i] = (a[i - 1] + a[i + 1]) * 0.5973859429359436f + (a[i - 3] + a[i + 3]) * -0.1145915612578392f
            + (a[i - 5] + a[i + 5]) * 0.018032679334282875f;
    }
}

extern "C" void reftolpc(float* ac, float* out)
{
    float tmp[12];
    float k[12];
    int i, m;
    for (i = 10; i >= 0; i--)
        tmp[i + 1] = ac[i];
    tmp[0] = 1.0f;
    for (m = 0; m <= 11; m++) {
        float acc = -ac[11] * tmp[11];
        for (i = 10; i >= 0; i--) {
            acc -= ac[i] * tmp[i];
            tmp[i + 1] = ac[i] * acc + tmp[i];
        }
        tmp[0] = acc;
        k[m] = acc;
        for (i = 0; i < m; i++)
            acc -= out[i] * k[m - i - 1];
        out[m] = acc;
    }
}

extern "C" void fn_803B8EE0(void* unused, unsigned char* data, SndDec* s, int flag)
{
    int i;
    data++;
    s->m0 = data;
    s->m8 = 8;
    s->m4 = data[-1];
    if (flag != 0) {
        s->mC = getbits(s, 1);
        s->m10 = 32 - getbits(s, 4);
        s->m14[0] = (float)(unsigned)(getbits(s, 4) + 1) * 8.0f;
        float f = (float)(unsigned)getbits(s, 6) * 0.001f + 1.04f;
        for (i = 1; i <= 0x3F; i++)
            s->m14[i] = s->m14[i - 1] * f;
        for (i = 0; i <= 11; i++) {
            s->m114[i] = 0.0f;
            s->m144[i] = 0.0f;
        }
        for (i = 0; i <= 0x143; i++)
            s->m174[i] = 0.0f;
    }
}

extern "C" SndDec* fn_803B9448(SndDec* d)
{
    d->mD44 = 0;
    d->mD54 = 0;
    d->mD64 = 1;
    d->mD58 = 0;
    d->mD50 = 0;
    d->mD52 = 0;
    d->mD5C = 0;
    d->mD60 = 0;
    return d;
}

extern "C" int fn_803B9478(SndDec* d, unsigned char* data, int a5, int a6)
{
    if (data == 0 || d->mD54 != 0)
        return -1;
    int mode = d->mD64;
    int z = d->mD54;
    d->mD54 = a6;
    d->mD58 = a5;
    d->mD44 = z;
    d->mD48 = data;
    if (mode == 1) {
        if (data[0] == 0xEE)
            d->mD60 = mode;
        else
            d->mD60 = z;
        fn_803B8EE0(d, data + 1, d, d->mD5C);
    } else {
        fn_803B8EE0(d, data, d, d->mD5C);
    }
    return 0;
}

extern "C" SndT3 fn_803B9730(SndDec* d)
{
    SndT3 t;
    t.a = d->m4;
    t.c = d->mD5C;
    t.b = d->mD44;
    return t;
}

extern "C" void fn_803B9770(SndDec* d, SndT3* t, int unused)
{
    d->m4 = t->a;
    d->mD5C = t->c;
    d->mD44 = t->b;
}

extern "C" void fn_803B978C(SndDec* d, int v)
{
    d->mD64 = v;
}

extern "C" void rsflc(int n, float* src, float* dst, int* idx, unsigned* fr, unsigned stepHi, unsigned stepLo)
{
    int i;
    unsigned step = (stepHi << 16) | (stepLo >> 16);
    *fr = ((unsigned short*)fr)[0];
    for (i = 0; i < n; i++) {
        float a = src[*idx];
        float t = (float)*fr * 1.5258e-5f;
        dst[i] = t * (src[*idx + 1] - a) + a;
        *fr += step;
        *idx += *fr >> 16;
        *fr = ((unsigned short*)fr)[1];
    }
    *fr <<= 16;
}

extern "C" int fn_803B9858(FunctionHandle* h, const InterfaceId* id)
{
    fn_803B9DDC();
    int r = h->SetFast(id);
    fn_803B9E08();
    return r;
}

Result FunctionHandle::SetFast(const InterfaceId* id)
{
    int tried = 0;
retry:
    {
        Module* m;
        for (ModLink* l = lbl_806AA89C.head; l != 0; l = m->link.next) {
            m = (Module*)((char*)l - 0x20);
            FnEntry* tbl = m->fn;
            int n = m->nFn;
            if (m->crc == id->systemCrc || tried != 0) {
                for (int i = 0; i < n; i++) {
                    FnEntry* e = (FnEntry*)(i * 12 + (int)tbl);
                    if (id->interfaceCrc == e->u.k.crc && strcmp(id->pString, e->name) == 0) {
                        mpPrivate = e;
                        mKey = e->u.key;
                        return RESULT_OK;
                    }
                }
            }
        }
        if (tried != 0)
            return RESULT_ERR_NOTFOUND;
        tried = 1;
        goto retry;
    }
}
