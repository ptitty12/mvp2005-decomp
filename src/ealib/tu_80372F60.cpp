/*
 * EA library TU, .text 0x80372F60-0x80373AD8 (static-init anchored).
 * Registers one static node (5 words, intrusive list, ctor 0x803DA440) whose
 * arguments are a creator function and three data tables.
 */

class Unk803DA440 {
public:
    Unk803DA440(void (*fn)(), void* a, void* b, void* c);
    int m[5];
};

struct GXColor {
    unsigned char r, g, b, a;
};

struct Unk80372F60Param {
    void* m0;
    void* m4;
};

/*
 * Object built by fn_803E3C08 (x, -1). Offsets seen in fn_80373748 (stack
 * object, 0xC0 bytes) and fn_80372F60.
 */
struct Unk803E3C08 {
    Unk80372F60Param* m0;
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

struct UnkCfg {
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

union PPCWGPipe {
    unsigned char u8;
    unsigned short u16;
    unsigned int u32;
    float f32;
};
#include "../../include/ealib/gx_fifo.h"

struct Unk80372F60Mtx {
    float f[16];
};

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
void fn_803DFBAC(Unk80372F60Mtx* in, Unk80372F60Mtx* out);
void GXLoadPosMtxImm(Unk80372F60Mtx* m, int id);
void GXLoadNrmMtxImm(Unk80372F60Mtx* m, int id);
void GXSetCurrentMtx(int id);
void GXLoadTexMtxImm(Unk80372F60Mtx* m, int id, int type);
void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);
void fn_803DD794(void* tex);
int fn_803DB5A0(void* state);
void fn_803DDCD4(void);
void GXCallDisplayList(void* list, int nbytes);
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
void fn_803DDB88(int a, int b, int c, int d, int e, int f);
void fn_803DDB5C(int n);
void fn_803DA468(int attr, void* base, int stride);
void fn_803E3C08(Unk803E3C08* self, void* x, int n);
void fn_80372F60(Unk803E3C08* a, UnkCfg* b);
}

extern char lbl_80623D78[];
extern char lbl_80623DA0[];
extern char lbl_80604D4C[];

extern int lbl_80627290;
extern UnkCfg lbl_80623DD4;
extern int lbl_80623E08;
extern int lbl_80623E0C;
extern int lbl_80623E18;
extern int lbl_80623DC4;
extern char lbl_80623DC8[];
extern int lbl_806EFB98;
extern int lbl_806EFB9C;
extern int lbl_806EFBA0;
extern int lbl_806EFBA4;
extern int lbl_806EFBA8;

extern "C" void fn_80372F60(Unk803E3C08* a, UnkCfg* b)
{
    unsigned i1;
    unsigned i2;
    if (a->mB0) {
        b->m4 = a->m0[7].m4;
        Unk80372F60Mtx mtx = *(Unk80372F60Mtx*)a->m0[1].m4;
        fn_803DFBAC(&mtx, &mtx);
        GXLoadPosMtxImm(&mtx, 0);
        GXSetCurrentMtx(0);
        Unk80372F60Mtx mtx2 = *(Unk80372F60Mtx*)a->m0[1].m4;
        fn_803DFBAC(&mtx2, &mtx2);
        GXLoadNrmMtxImm(&mtx2, 0);
        Unk80372F60Mtx mtx3 = *(Unk80372F60Mtx*)a->m0[2].m4;
        fn_803DFBAC(&mtx3, &mtx3);
        GXLoadTexMtxImm(&mtx3, 0x1e, 0);
        fn_803DDB5C(2);
        fn_803DDB88(0, 0, 0, 0x1e, 0, 0x7d);
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
    if (a->m30[0]) {
        fn_803DD794((void*)lbl_806EFB98);
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
            unsigned i;
            v.Begin(n);
            if (b->m0 == 4) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 8) | (i << 16) | (i << 24);
                }
            } else if (b->m0 == 3) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i | (i << 8);
                    EALIB_GX_FIFO(PPCWGPipe).u8 = i;
                }
            } else if (b->m0 == 2) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i | (i << 8);
                }
            } else if (b->m0 == 1) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u8 = i;
                }
            } else {
                for (i = 0; i < n; i++) {
                    for (int j = 0; j < b->m0; j++) {
                        EALIB_GX_FIFO(PPCWGPipe).u8 = i;
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
            unsigned i;
            v.Begin(n);
            if (b->m0 == 4) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 16);
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 16);
                }
            } else if (b->m0 == 3) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 16);
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i;
                }
            } else if (b->m0 == 2) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 16);
                }
            } else if (b->m0 == 1) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i;
                }
            } else {
                for (i = 0; i < n; i++) {
                    for (int j = 0; j < b->m0; j++) {
                        EALIB_GX_FIFO(PPCWGPipe).u16 = i;
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

extern "C" void fn_80373748(void* x)
{
    Unk803E3C08 a;
    a.m8 = &lbl_806EFB9C;
    a.m24 = (unsigned char*)lbl_80623DC8;
    a.m18 = lbl_80623DC4;
    a.m14 = &lbl_806EFBA0;
    a.m10 = &lbl_806EFB98;
    a.m1C = &lbl_806EFBA4;
    a.m20 = &lbl_806EFBA8;
    fn_803E3C08(&a, x, -1);
    if (a.mB0) {
        GXColor c = { 0xff, 0xff, 0xff, 0xff };
        GXSetChanAmbColor(4, c);
        GXSetChanMatColor(4, c);
        fn_803DDE7C();
        fn_803DDBF0();
    }
    if (a.mB4 != 0 || a.m50[10] != lbl_80623DD4.m34 || a.m50[11] != lbl_80623DD4.m38 || a.m50[14] != lbl_80623DD4.m44) {
        a.mB8 = 0;
        GXClearVtxDesc();
        lbl_80623E08 = a.m50[10];
        if (lbl_80623E08) {
            GXSetVtxDesc(9, 2);
        } else {
            GXSetVtxDesc(9, 3);
        }
        lbl_80623E0C = a.m50[11];
        if (lbl_80623E0C) {
            GXSetVtxDesc(0xa, 2);
        } else {
            GXSetVtxDesc(0xa, 3);
        }
        lbl_80623E18 = a.m50[14];
        if (lbl_80623E18) {
            GXSetVtxDesc(0xb, 2);
        } else {
            GXSetVtxDesc(0xb, 3);
        }
    }
    if (a.mB4) {
        fn_803DBF78(0, 9, 1, 4, 0);
        fn_803DBF78(0, 0xa, 0, 3, 0xe);
        fn_803DBF78(0, 0xb, 1, 5, 0);
        fn_803DDB5C(0);
        GXSetNumChans(1);
        lbl_80623DD4.m0 = 3;
        lbl_80623DD4.m8 = 0x1600;
        lbl_80623DD4.m6C = 1;
        fn_803DB9C0(0x1600, -1);
        lbl_80623DD4.m70 = 1;
    }
    fn_803DA468(9, a.m0[3].m4, 0xc);
    fn_803DA468(0xa, a.m0[4].m4, 6);
    fn_803DA468(0xb, a.m0[5].m4, 4);
    fn_80372F60(&a, &lbl_80623DD4);
    if (a.m4) {
        while (a.m4[0] == 2) {
            a.m0[a.m4[1]].m4 = (void*)((int)a.m0[a.m4[1]].m4 - *(int*)(a.m4 + 2) * lbl_80627290);
            a.m4 += 6;
        }
    }
}

Unk803DA440 lbl_80693E14((void (*)())fn_80373748, lbl_80623D78, lbl_80623DA0, lbl_80604D4C);
