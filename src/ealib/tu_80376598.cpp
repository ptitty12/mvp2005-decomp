/*
 * EA library TU, .text 0x80376598-0x80377048 (static-init anchored; issue #76).
 * Registers one static node (5 words, intrusive list, ctor 0x803DA440) whose
 * arguments are a creator function and three data tables.
 *
 * FIFO accesses use the typed page-relative volatile view in ealib/gx_fifo.h.
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

struct Unk80373AD8Param {
    void* m0;
    void* m4;
};

struct Unk80373AD8Mtx {
    float f[16];
};

/*
 * Object built by fn_803E3C08 (x, -1). Offsets seen in fn_80376C9C (stack
 * object, 0xC0 bytes) and fn_80376598.
 */
struct Unk803E3C08 {
    Unk80373AD8Param* m0;
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

struct Unk80623E94 {
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

extern unsigned char* lbl_8062C660;
extern int lbl_8062C664;

struct Unk803DBE08Out {
    int m0;
    int m4;
    inline Unk803DBE08Out(void* state);
    inline void Begin(int n);
    inline void StoreHdr(int per);
};

extern "C" {
void fn_803DFBAC(Unk80373AD8Mtx* in, Unk80373AD8Mtx* out);
void GXLoadPosMtxImm(Unk80373AD8Mtx* m, int id);
void GXSetCurrentMtx(int id);
void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);
void GXSetNumChans(int n);
void fn_803DD794(void* tex);
void PSMTXTrans(Unk80373AD8Mtx* m, float x, float y, float z);
void GXLoadTexMtxImm(Unk80373AD8Mtx* m, int id, int type);
void fn_803DDB88(int a, int b, int c, int d, int e, int f);
int fn_803DB5A0(void* state);
void fn_803DB9C0(int a, int b);
void fn_803DDCD4(void);
void GXCallDisplayList(void* list, int nbytes);
int fn_803DBE08(void* state, Unk803DBE08Out* out);
void DCStoreRange(void* p, int n);
void GXBegin(int a, int b, int c);
void fn_803DA188(int n);
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

extern void (*lbl_806279C8)(void);
extern int lbl_80627290;
extern char lbl_80624148[];
extern char lbl_8062416C[];
extern char lbl_80605460[];
extern int lbl_8062418C;
extern char lbl_80624190[];
extern Unk80623E94 lbl_80624194;
extern int lbl_806241C8;
extern int lbl_806241D8;
extern int lbl_806241E0;
extern int lbl_806EFBFC;
extern int lbl_806EFC00;
extern int lbl_806EFC04;
extern int lbl_806EFC08;
extern int lbl_806EFC0C;

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

/*
 * What mattered: Unk803DBE08Out must be 8 bytes (frame layout), inline ctor and
 * members for the &v register pair, "per = a / b; per = per / 12 * 12;" as two
 * statements, "n *= m; n += 3;" in the tails, the if/else chain (not a switch),
 * unsigned char i with an int "next" temp in the fill loop, "first = 1" after
 * the first-chunk call, and one "unsigned i" per branch shared by its five
 * index loops (global-alloc priority: a per-loop i is 3*11/34 against the
 * inner j's 3*13/20, so j takes r9; the shared i is 5*53/130 and goes first).
 */
extern "C" void fn_80376598(Unk803E3C08* a, Unk80623E94* b)
{
    Unk80373AD8Mtx mtx;
    if (a->mB0) {
        b->m4 = a->m0[7].m4;
        mtx = *(Unk80373AD8Mtx*)a->m0[1].m4;
        fn_803DFBAC(&mtx, &mtx);
        GXLoadPosMtxImm(&mtx, 0);
        GXSetCurrentMtx(0);
        if (b->m70) {
            GXSetChanCtrl(4, 0, 1, 1, 0, 2, 2);
            GXSetChanCtrl(5, 0, 1, 1, 0, 0, 2);
        } else {
            GXSetNumChans(1);
            GXSetChanCtrl(4, 0, 0, 0, 0, 2, 2);
            GXSetChanCtrl(5, 0, 0, 0, 0, 0, 2);
        }
        lbl_806279C8 = fn_803DDBF0;
    }
    if (a->m30[0]) {
        fn_803DD794(a->m0[5].m4);
    }
    float* uv = (float*)a->m0[6].m4;
    PSMTXTrans(&mtx, uv[0], uv[1], 0.0f);
    GXLoadTexMtxImm(&mtx, 0x1e, 1);
    fn_803DDB88(0, 1, 4, 0x1e, 0, 0x7d);
    fn_803DB5A0(b->m4);
    if (b->m6C == 0) {
        fn_803DB9C0(b->m8, 3);
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

extern "C" void fn_80376C9C(void* x)
{
    Unk803E3C08 a;
    a.m8 = &lbl_806EFBFC;
    a.m24 = (unsigned char*)lbl_80624190;
    a.m18 = lbl_8062418C;
    a.m14 = &lbl_806EFC00;
    a.m10 = &lbl_806EFC04;
    a.m1C = &lbl_806EFC08;
    a.m20 = &lbl_806EFC0C;
    fn_803E3C08(&a, x, -1);
    if (a.mB0) {
        GXColor c = { 0xff, 0xff, 0xff, 0xff };
        GXSetChanAmbColor(4, c);
        GXSetChanMatColor(4, c);
        fn_803DDE7C();
        fn_803DDBF0();
    }
    if (a.mB4 != 0 || a.m50[10] != lbl_80624194.m34 || a.m50[14] != lbl_80624194.m44
        || a.m50[16] != lbl_80624194.m4C) {
        a.mB8 = 0;
        GXClearVtxDesc();
        lbl_806241C8 = a.m50[10];
        if (lbl_806241C8) {
            GXSetVtxDesc(9, 2);
        } else {
            GXSetVtxDesc(9, 3);
        }
        lbl_806241D8 = a.m50[14];
        if (lbl_806241D8) {
            GXSetVtxDesc(0xb, 2);
        } else {
            GXSetVtxDesc(0xb, 3);
        }
        lbl_806241E0 = a.m50[16];
        if (lbl_806241E0) {
            GXSetVtxDesc(0xd, 2);
        } else {
            GXSetVtxDesc(0xd, 3);
        }
    }
    if (a.mB4) {
        fn_803DBF78(0, 9, 1, 4, 0);
        fn_803DBF78(0, 0xb, 1, 5, 0);
        fn_803DBF78(0, 0xd, 1, 4, 0);
        fn_803DDB88(0, 1, 4, 0x3c, 0, 0x7d);
        fn_803DDB5C(1);
        GXSetNumChans(1);
        lbl_80624194.m0 = 3;
        lbl_80624194.m8 = 0x5200;
        lbl_80624194.m6C = 1;
        fn_803DB9C0(0x5200, -1);
        lbl_80624194.m70 = 1;
    }
    fn_803DA468(9, a.m0[2].m4, 0xc);
    fn_803DA468(0xb, a.m0[3].m4, 4);
    fn_803DA468(0xd, a.m0[4].m4, 8);
    fn_80376598(&a, &lbl_80624194);
    if (a.m4) {
        while (a.m4[0] == 2) {
            a.m0[a.m4[1]].m4 = (void*)((int)a.m0[a.m4[1]].m4 - *(int*)(a.m4 + 2) * lbl_80627290);
            a.m4 += 6;
        }
    }
}

Unk803DA440 lbl_80693E78((void (*)())fn_80376C9C, lbl_80624148, lbl_8062416C, lbl_80605460);
