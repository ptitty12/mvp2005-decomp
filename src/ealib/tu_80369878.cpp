/*
 * EA library TU, .text 0x80369878-0x8036A490 (TU start is a guess: the previous
 * TU's end).
 * Static registration object; ctor 0x803DA440 stores {fn, a, b, c} and links the
 * node into a global list. The 0x60 + 0x2c pair at the end is the GCC
 * __static_initialization_and_destruction_0 / _GLOBAL_.I wrapper.
 *
 * FIFO accesses use the typed page-relative volatile view in ealib/gx_fifo.h.
 */

struct Unk803DA440 {
    Unk803DA440(void (*fn)(), void* a, void* b, void* c); /* 0x803DA440 */
    char pad[0x14];
};

union PPCWGPipe {
    unsigned char u8;
    unsigned short u16;
    unsigned int u32;
    float f32;
};
#include "../../include/ealib/gx_fifo.h"

struct GXColor {
    unsigned char r, g, b, a;
};

struct Unk80369878Param {
    void* m0;
    void* m4;
};

struct Mtx34 {
    float f[12];
};

/*
 * Object built by fn_803E3C08 (x, -1). Offsets seen in fn_8036A16C (stack
 * object, 0xC0 bytes) and fn_80369878.
 */
struct Unk803E3C08 {
    Unk80369878Param* m0;
    unsigned char* m4;
    int* m8;
    int pad0C;
    int* m10;
    int* m14;
    int m18;
    int* m1C;
    int* m20;
    unsigned char* m24;
    void* m28;
    int m2C;
    int m30[8];
    int m50[24];
    int mB0;
    int mB4;
    int mB8;
    int mBC;
};

struct Unk80623320 {
    int m0;
    void* m4;
    int m8;
    int pad0C[10];
    int m34;
    int m38;
    int pad3C[2];
    int m44;
    int pad48;
    int m4C;
    int pad50[7];
    int m6C;
    int m70;
};

extern "C" {
void fn_803EA338(void* a, void* b, void* c, void* d);
void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);
void GXSetNumChans(int n);
void fn_803DD794(void* tex);
int fn_803DB5A0(void* state);
void fn_803DB9C0(int a, int b);
void fn_803DDCD4(void);
void GXCallDisplayList(void* list, int nbytes);
void fn_803DDBF0(void);
void fn_803DDE7C(void);
void GXSetChanAmbColor(int chan, GXColor c);
void GXSetChanMatColor(int chan, GXColor c);
void GXClearVtxDesc(void);
void GXSetVtxDesc(int attr, int type);
void fn_803DBF78(int a, int b, int c, int d, int e);
void fn_803DDB88(int a, int b, int c, int d, int e, int f);
void fn_803DDB5C(int n);
void fn_803DA468(int attr, void* base, int stride);
void fn_803E3C08(Unk803E3C08* self, void* x, int n);
}

struct Unk803DBE08Out {
    int m0;
    inline Unk803DBE08Out(void* state);
    inline void Begin(int n);
    inline void StoreHdr(int per);
};

extern unsigned char* lbl_8062C660;
extern int lbl_8062C664;

extern "C" {
int fn_803DBE08(void* state, Unk803DBE08Out* out);
void DCStoreRange(void* p, int n);
void GXBegin(int a, int b, int c);
void fn_803DA188(int n);
}

inline Unk803DBE08Out::Unk803DBE08Out(void* state)
{
    fn_803DBE08(state, this);
}

inline void Unk803DBE08Out::StoreHdr(int per)
{
    *lbl_8062C660 = (unsigned char)m0;
    *(unsigned short*)(lbl_8062C660 + 1) = per;
}

inline void Unk803DBE08Out::Begin(int n)
{
    GXBegin(0x90, 0, 0);
    EALIB_GX_FIFO(PPCWGPipe).u8 = (unsigned char)m0;
    EALIB_GX_FIFO(PPCWGPipe).u16 = n;
}


struct Unk803DE1BC {
    int m[26];
};

struct SHCoef {
    float r, g, b, pad;
};

extern "C" {
void fn_803DF74C(Mtx34* m);
void GXLoadPosMtxImm(Mtx34* m, int id);
void GXSetCurrentMtx(int id);
void fn_803DE1BC(Unk803DE1BC* o);
void fn_803DE2E8(Unk803DE1BC* o, int a, int b);
void fn_803DE27C(Unk803DE1BC* o, int a, int b, int c, int d);
void fn_803DE25C(Unk803DE1BC* o, int a, int b, int c, int d);
void fn_803DE300(Unk803DE1BC* o, int a);
void PSMTXCopy(void* a, Mtx34* m);
void PSMTXMultVec(Mtx34* m, void* in, float* out);
void fn_803D98C8(int a, int n);
void fn_803D9918(void* p, int n);
void fn_803D999C(void);
void GXCallDisplayList(void* list, int nbytes);
}

extern const char lbl_806232E0[];
extern const char lbl_806232FC[];
extern const char lbl_8060383C[];

extern int lbl_80627290;
extern int lbl_80623314;
extern char lbl_80623318[];
extern Unk80623320 lbl_80623320;
extern int lbl_80623354;
extern int lbl_80623364;
extern int lbl_806EFA90;
extern int lbl_806EFA94;
extern int lbl_806EFA98;
extern int lbl_806EFA9C;
extern int lbl_806EFAA0;

/*
 * Display-list builder (family of the 0x704 function in tu_80373AD8) whose
 * prologue evaluates a 10-term quadratic colour polynomial per vertex
 * (c0 + c1*xx + c2*yy + c3*zz + c4*xy + c5*xz + c6*yz + c7*x + c8*y + c9*z,
 * one chain per channel, clamped to [0, 1], packed RGBA). Levers, as in
 * tu_80368E64 / tu_803682A8: one index counter per branch, "first = 1" after
 * the GXCallDisplayList call, float literals, the table pointer a[0][2].m4 re-read per
 * iteration. Unk803DBE08Out is a single int here (v at sp+0xb4 next to the
 * 8-aligned fctiwz temp); an 8-byte struct pushes the frame out by 8.
 */
extern "C" void fn_80369878(Unk803E3C08* a, Unk80623320* b)
{
    Mtx34 mtx;
    float vec[3];
    Unk803DE1BC o;
    unsigned i1;
    unsigned i2;
    if (a->mB0) {
        b->m4 = a->m0[5].m4;
        fn_803DF74C(&mtx);
        GXLoadPosMtxImm(&mtx, 0);
        GXSetCurrentMtx(0);
        fn_803DDB5C(0);
        fn_803DE1BC(&o);
        fn_803DE2E8(&o, 0xff, 0xff);
        fn_803DE27C(&o, 0xf, 0xf, 0xf, 0xa);
        fn_803DE25C(&o, 7, 7, 7, 5);
        fn_803DE300(&o, 0);
        if (b->m70) {
            GXSetChanCtrl(4, 0, 1, 1, 0, 2, 2);
            GXSetChanCtrl(5, 0, 1, 1, 0, 0, 2);
        } else {
            GXSetNumChans(1);
            GXSetChanCtrl(4, 0, 0, 0, 0, 2, 2);
            GXSetChanCtrl(5, 0, 0, 0, 0, 0, 2);
        }
        fn_803DB5A0(b->m4);
        if (b->m6C == 0) {
            fn_803DB9C0(b->m8, 3);
        }
    }
    int j = 0;
    int count = ((int*)a->m0[0].m4)[1];
    PSMTXCopy(a->m0[1].m4, &mtx);
    mtx.f[3] = 0.0f;
    mtx.f[7] = 0.0f;
    mtx.f[11] = 0.0f;
    fn_803D98C8(lbl_806EFA90, ((int*)a->m0[0].m4)[1] * 4);
    for (; j < count; j++) {
        PSMTXMultVec(&mtx, (char*)a->m0[4].m4 + j * 16, vec);
        SHCoef* c = (SHCoef*)a->m0[2].m4;
        float x = vec[0];
        float y = vec[1];
        float z = vec[2];
        float xx = x * x;
        float yy = y * y;
        float xy = x * y;
        float zz = z * z;
        float xz = x * z;
        float yz = y * z;
        float r = c[0].r + c[1].r * xx + c[2].r * yy + c[3].r * zz + c[4].r * xy + c[5].r * xz
                  + c[6].r * yz + c[7].r * x + c[8].r * y + c[9].r * z;
        float g = c[0].g + c[1].g * xx + c[2].g * yy + c[3].g * zz + c[4].g * xy + c[5].g * xz
                  + c[6].g * yz + c[7].g * x + c[8].g * y + c[9].g * z;
        float bl = c[0].b + c[1].b * xx + c[2].b * yy + c[3].b * zz + c[4].b * xy + c[5].b * xz
                   + c[6].b * yz + c[7].b * x + c[8].b * y + c[9].b * z;
        if (r < 0.0f) {
            r = 0.0f;
        } else if (r > 1.0f) {
            r = 1.0f;
        }
        if (g < 0.0f) {
            g = 0.0f;
        } else if (g > 1.0f) {
            g = 1.0f;
        }
        if (bl < 0.0f) {
            bl = 0.0f;
        } else if (bl > 1.0f) {
            bl = 1.0f;
        }
        int packed = ((int)(r * 255.0f) << 24) + ((int)(g * 255.0f) << 16) + ((int)(bl * 255.0f) << 8) + 0xff;
        fn_803D9918(&packed, 4);
    }
    fn_803D999C();
    fn_803DDCD4();
    if (a->mBC == 0) {
        void* dl = a->m28;
        int dlsz = a->m2C;
        if (a->mB8) {
            EALIB_GX_FIFO(PPCWGPipe).u8 = 0x40;
            EALIB_GX_FIFO(PPCWGPipe).u32 = (unsigned int)dl;
            EALIB_GX_FIFO(PPCWGPipe).u32 = dlsz;
        } else {
            GXCallDisplayList(dl, dlsz);
        }
    } else {
        Unk803DBE08Out v(b->m4);
        int n = *(int*)a->m0[0].m4;
        if (lbl_8062C660) {
            int per = (lbl_8062C664 - 3) / b->m0;
            per = per / 12 * 12;
            fn_803DB9C0(b->m8, 2);
            if (*(int*)lbl_8062C660 == -1) {
                v.StoreHdr(per);
                unsigned char* p = lbl_8062C660 + 3;
                unsigned char i = 0;
                while (i < per) {
                    int next = i + 1;
                    for (int j = 0; j < b->m0; j++) {
                        *p++ = i;
                    }
                    i = next;
                }
                for (; p < lbl_8062C660 + lbl_8062C664; p++) {
                    *p = 0;
                }
                DCStoreRange(lbl_8062C660, lbl_8062C664);
            }
            int sz = (per * b->m0 + 0x22) / 32 * 32;
            int first = 0;
            while (n >= per) {
                if (first) {
                    EALIB_GX_FIFO(PPCWGPipe).u8 = 0x40;
                    EALIB_GX_FIFO(PPCWGPipe).u32 = (unsigned int)lbl_8062C660;
                    EALIB_GX_FIFO(PPCWGPipe).u32 = sz;
                } else {
                    GXCallDisplayList(lbl_8062C660, sz);
                    first = 1;
                }
                int chunk = per;
                if (v.m0 == 0x98) {
                    chunk = per - 2;
                }
                fn_803DA188(chunk);
                n -= chunk;
            }
            v.Begin(n);
            if (b->m0 == 4) {
                for (i1 = 0; i1 < n; i1++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i1 | (i1 << 8) | (i1 << 16) | (i1 << 24);
                }
            } else if (b->m0 == 3) {
                for (i1 = 0; i1 < n; i1++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i1 | (i1 << 8);
                    EALIB_GX_FIFO(PPCWGPipe).u8 = i1;
                }
            } else if (b->m0 == 2) {
                for (i1 = 0; i1 < n; i1++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i1 | (i1 << 8);
                }
            } else if (b->m0 == 1) {
                for (i1 = 0; i1 < n; i1++) {
                    EALIB_GX_FIFO(PPCWGPipe).u8 = i1;
                }
            } else {
                for (i1 = 0; i1 < n; i1++) {
                    for (int j = 0; j < b->m0; j++) {
                        EALIB_GX_FIFO(PPCWGPipe).u8 = i1;
                    }
                }
            }
            n *= b->m0;
            n += 3;
            while (n & 0x1f) {
                n++;
                EALIB_GX_FIFO(PPCWGPipe).u8 = 0;
            }
        } else {
            v.Begin(n);
            if (b->m0 == 4) {
                for (i2 = 0; i2 < n; i2++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i2 | (i2 << 16);
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i2 | (i2 << 16);
                }
            } else if (b->m0 == 3) {
                for (i2 = 0; i2 < n; i2++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i2 | (i2 << 16);
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i2;
                }
            } else if (b->m0 == 2) {
                for (i2 = 0; i2 < n; i2++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i2 | (i2 << 16);
                }
            } else if (b->m0 == 1) {
                for (i2 = 0; i2 < n; i2++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i2;
                }
            } else {
                for (i2 = 0; i2 < n; i2++) {
                    for (int j = 0; j < b->m0; j++) {
                        EALIB_GX_FIFO(PPCWGPipe).u16 = i2;
                    }
                }
            }
            n = (n + n) * b->m0;
            n += 3;
            while (n & 0x1f) {
                n++;
                EALIB_GX_FIFO(PPCWGPipe).u8 = 0;
            }
        }
    }
}


extern "C" void fn_8036A16C(void* x)
{
    Unk803E3C08 a;
    a.m8 = &lbl_806EFA94;
    a.m24 = (unsigned char*)lbl_80623318;
    a.m18 = lbl_80623314;
    a.m14 = &lbl_806EFA98;
    a.m10 = &lbl_806EFA9C;
    a.m1C = &lbl_806EFAA0;
    a.m20 = &lbl_806EFA90;
    fn_803E3C08(&a, x, -1);
    if (a.mB0) {
        GXColor c = { 0xff, 0xff, 0xff, 0xff };
        GXSetChanAmbColor(4, c);
        GXSetChanMatColor(4, c);
        fn_803DDE7C();
        fn_803DDBF0();
    }
    if (a.mB4 != 0 || a.m50[10] != lbl_80623320.m34 || a.m50[14] != lbl_80623320.m44) {
        a.mB8 = 0;
        GXClearVtxDesc();
        lbl_80623354 = a.m50[10];
        if (lbl_80623354) {
            GXSetVtxDesc(9, 2);
        } else {
            GXSetVtxDesc(9, 3);
        }
        lbl_80623364 = a.m50[14];
        if (lbl_80623364) {
            GXSetVtxDesc(0xb, 2);
        } else {
            GXSetVtxDesc(0xb, 3);
        }
    }
    if (a.mB4) {
        fn_803DBF78(0, 9, 1, 4, 0);
        fn_803DBF78(0, 0xb, 1, 5, 0);
        fn_803DDB5C(0);
        GXSetNumChans(1);
        lbl_80623320.m0 = 2;
        lbl_80623320.m8 = 0x1200;
        lbl_80623320.m6C = 1;
        fn_803DB9C0(0x1200, -1);
        lbl_80623320.m70 = 1;
    }
    fn_803DA468(9, a.m0[3].m4, 0xc);
    fn_803DA468(0xb, (void*)lbl_806EFA90, 4);
    fn_80369878(&a, &lbl_80623320);
    if (a.m4) {
        while (a.m4[0] == 2) {
            a.m0[a.m4[1]].m4 = (void*)((int)a.m0[a.m4[1]].m4 - *(int*)(a.m4 + 2) * lbl_80627290);
            a.m4 += 6;
        }
    }
}

Unk803DA440 lbl_80693D10((void (*)())fn_8036A16C, (void*)lbl_806232E0, (void*)lbl_806232FC, (void*)lbl_8060383C);
