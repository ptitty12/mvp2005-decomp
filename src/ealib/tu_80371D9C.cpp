/*
 * EA library TU, .text 0x80371D9C-0x803728E8 (static-init anchored; issue #69).
 * Registers one static node (5 words, intrusive list, ctor 0x803DA440) whose
 * arguments are a creator function and three data tables.
 */

class Unk803DA440 {
public:
    Unk803DA440(void (*fn)(), void* a, void* b, void* c);
    int m[5];
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

struct Unk80371D9CParam {
    void* m0;
    void* m4;
};

/*
 * Object built by fn_803E3C08 (x, -1). Offsets seen in fn_80372638 (stack
 * object, 0xC0 bytes) and fn_80371D9C.
 */
struct Unk803E3C08 {
    Unk80371D9CParam* m0;
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

struct Unk80371D9CMtx {
    float f[16];
};

struct Unk803DE1BC {
    int m[26];
};

struct Unk803728E8Param {
    void* m0;
    void* m4;
};

struct Unk80623C2C {
    int m0;
    void* m4;
    int m8;
    int pad0C[10];
    int m34;
    int pad38[3];
    int m44;
    int pad48;
    int m4C;
    int pad50[7];
    int m6C;
    int m70;
};

extern "C" {
void fn_803DFBAC(Unk80371D9CMtx* in, Unk80371D9CMtx* out);
void GXLoadPosMtxImm(Unk80371D9CMtx* m, int id);
void GXSetCurrentMtx(int id);
void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);
int fn_803DB5A0(void* state);
void fn_803DDCD4(void);
void GXCallDisplayList(void* list, int nbytes);
void fn_803EA338(void* a, void* b, void* c, void* d);
void fn_803DBBE0(void* tex);
void fn_803DE1BC(Unk803DE1BC* o);
void fn_803DE2E8(Unk803DE1BC* o, int a, int b);
void fn_803DE27C(Unk803DE1BC* o, int a, int b, int c, int d);
void fn_803DE25C(Unk803DE1BC* o, int a, int b, int c, int d);
void fn_803DE300(Unk803DE1BC* o, int a);
int fn_803DB5A0(void* state);
void fn_803DDCD4(void);
void GXCallDisplayList(void* list, int nbytes);
void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);
int fn_803DBE2C(void* state, int a);
void fn_803DBBE0(void* tex);
}

struct Unk803DBE08Out {
    int m0;
    int m4;
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


extern "C" {
void GXSetNumChans(int n);
void fn_803DB9C0(int a, int b);
void fn_803DDBF0(void);
void fn_803DDE7C(void);
void GXSetChanAmbColor(int chan, GXColor c);
void GXSetChanMatColor(int chan, GXColor c);
void GXClearVtxDesc(void);
void GXSetVtxDesc(int attr, int type);
void fn_803DBF78(int a, int b, int c, int d, int e);
void fn_803DDB5C(int n);
void fn_803DA468(int attr, void* base, int stride);
void fn_803E3C08(Unk803E3C08* self, void* x, int n);
}

extern char lbl_80623BE8[];
extern char lbl_80623C08[];
extern char lbl_80604A1C[];

extern int lbl_80627290;
extern void (*lbl_806279C8)(void);
extern "C" void DisableCoPlanarZMode(void);
extern int lbl_80623C24;
extern char lbl_80623C28[];
extern Unk80623C2C lbl_80623C2C;
extern int lbl_80623C60;
extern int lbl_806EFB70;
extern int lbl_806EFB74;
extern int lbl_806EFB78;
extern int lbl_806EFB7C;
extern int lbl_806EFB80;

extern "C" void fn_80371D9C(Unk803E3C08* a, Unk80623C2C* b)
{
    Unk803DE1BC o;
    unsigned i1;
    unsigned i2;
    if (a->mB0) {
        b->m4 = a->m0[6].m4;
        if (b->m70) {
            GXSetChanCtrl(4, 0, 1, 1, 0, 2, 2);
            GXSetChanCtrl(5, 0, 1, 1, 0, 0, 2);
        } else {
            GXSetNumChans(1);
            GXSetChanCtrl(4, 0, 0, 0, 0, 2, 2);
            GXSetChanCtrl(5, 0, 0, 0, 0, 0, 2);
        }
        fn_803DDB5C(0);
        fn_803DE1BC(&o);
        fn_803DE2E8(&o, 0xff, 0xff);
        fn_803DE27C(&o, 0xf, 0xf, 0xf, 0xa);
        fn_803DE25C(&o, 7, 7, 7, 5);
        fn_803DE300(&o, 0);
        GXColor c1 = { 0xff, 0xff, 0xff, 0xff };
        GXColor c2;
        Unk80371D9CParam* p = a->m0;
        c2.r = (unsigned char)(((float*)p[5].m4)[0] * 255.0f);
        c2.g = (unsigned char)(((float*)p[5].m4)[1] * 255.0f);
        c2.b = (unsigned char)(((float*)p[5].m4)[2] * 255.0f);
        c2.a = (unsigned char)(((float*)p[5].m4)[3] * 255.0f);
        GXSetChanAmbColor(4, c1);
        GXSetChanMatColor(4, c2);
        GXSetChanCtrl(4, 0, 0, 0, 0, 2, 2);
        fn_803DBE2C(b->m4, 1);
        fn_803DB5A0(b->m4);
        if (b->m6C == 0) {
            fn_803DB9C0(b->m8, 3);
        }
        Unk80371D9CMtx mtx = *(Unk80371D9CMtx*)a->m0[2].m4;
        fn_803DFBAC(&mtx, &mtx);
        GXLoadPosMtxImm(&mtx, 0);
        GXSetCurrentMtx(0);
        fn_803DBBE0(a->m0[4].m4);
        lbl_806279C8 = DisableCoPlanarZMode;
        mtx = *(Unk80371D9CMtx*)a->m0[1].m4;
        fn_803DFBAC(&mtx, &mtx);
        GXLoadPosMtxImm(&mtx, 0);
        GXSetCurrentMtx(0);
    }
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

extern "C" void fn_80372638(void* x)
{
    Unk803E3C08 a;
    a.m8 = &lbl_806EFB70;
    a.m24 = (unsigned char*)lbl_80623C28;
    a.m18 = lbl_80623C24;
    a.m14 = &lbl_806EFB74;
    a.m10 = &lbl_806EFB78;
    a.m1C = &lbl_806EFB7C;
    a.m20 = &lbl_806EFB80;
    fn_803E3C08(&a, x, -1);
    if (a.mB0) {
        GXColor c = { 0xff, 0xff, 0xff, 0xff };
        GXSetChanAmbColor(4, c);
        GXSetChanMatColor(4, c);
        fn_803DDE7C();
        fn_803DDBF0();
    }
    if (a.mB4 != 0 || a.m50[10] != lbl_80623C2C.m34) {
        a.mB8 = 0;
        GXClearVtxDesc();
        lbl_80623C60 = a.m50[10];
        if (lbl_80623C60) {
            GXSetVtxDesc(9, 2);
        } else {
            GXSetVtxDesc(9, 3);
        }
    }
    if (a.mB4) {
        fn_803DBF78(0, 9, 1, 4, 0);
        fn_803DDB5C(0);
        GXSetNumChans(0);
        lbl_80623C2C.m8 = 0x200;
        lbl_80623C2C.m0 = 1;
        lbl_80623C2C.m6C = 1;
        fn_803DB9C0(0x200, -1);
    }
    fn_803DA468(9, a.m0[3].m4, 0xc);
    fn_80371D9C(&a, &lbl_80623C2C);
    if (a.m4) {
        while (a.m4[0] == 2) {
            a.m0[a.m4[1]].m4 = (void*)((int)a.m0[a.m4[1]].m4 - *(int*)(a.m4 + 2) * lbl_80627290);
            a.m4 += 6;
        }
    }
}

Unk803DA440 lbl_80693DEC((void (*)())fn_80372638, lbl_80623BE8, lbl_80623C08, lbl_80604A1C);
