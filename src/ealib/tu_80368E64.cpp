/*
 * EA library TU, .text 0x80368E64-0x80369878 (TU start is a guess: the previous
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

struct Unk80368E64Param {
    void* m0;
    void* m4;
};

struct Unk80373AD8Mtx {
    float f[16];
};

/*
 * Object built by fn_803E3C08 (x, -1). Offsets seen in RenderSingleDrawGouraud (stack
 * object, 0xC0 bytes) and fn_80368E64.
 */
struct Unk803E3C08 {
    Unk80368E64Param* m0;
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

struct Unk8062326C {
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

struct Unk803DE1BC {
    int m[26];
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
void fn_803DFBAC(Unk80373AD8Mtx* in, Unk80373AD8Mtx* out);
void GXLoadPosMtxImm(Unk80373AD8Mtx* m, int id);
void GXSetCurrentMtx(int id);
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
void fn_803DE1BC(Unk803DE1BC* o);
void fn_803DE2E8(Unk803DE1BC* o, int a, int b);
void fn_803DE27C(Unk803DE1BC* o, int a, int b, int c, int d);
void fn_803DE25C(Unk803DE1BC* o, int a, int b, int c, int d);
void fn_803DE300(Unk803DE1BC* o, int a);
void fn_803DD794(void* tex);
void GXCallDisplayList(void* list, int nbytes);
}

extern const char lbl_80623230[];
extern const char lbl_80623248[];
extern const char lbl_80603718[];

extern int lbl_80627290;
extern int lbl_8062325C;
extern char lbl_80623260[];
extern Unk8062326C lbl_8062326C;
extern int lbl_806232A0;
extern int lbl_806232B0;
extern int lbl_806EFA74;
extern int lbl_806EFA78;
extern void* lbl_806EFA80[2];
extern int lbl_806EFA88;
extern int lbl_806EFA8C;

/*
 * Display-list builder (same family as the 0x704 function in tu_80373AD8).
 * Register allocation tells: the index loops of one branch share ONE counter
 * (i1 / i2, declared at function scope). With a counter per loop the default
 * loops get i and j in swapped registers (gcc 2.95 allocates by
 * refs*log2(refs)/live-length, and a per-loop i loses to the inner counter j);
 * a shared i has enough weighted references to be allocated first (r9, j r11).
 * "first = 1" goes after the GXCallDisplayList call (li r25,1 after mr r4,r28).
 */
extern "C" void fn_80368E64(Unk803E3C08* a, Unk8062326C* b)
{
    Unk80373AD8Mtx mtx;
    Unk803DE1BC o;
    unsigned i1;
    unsigned i2;
    if (a->mB0) {
        b->m4 = a->m0[4].m4;
        mtx = *(Unk80373AD8Mtx*)a->m0[1].m4;
        fn_803DFBAC(&mtx, &mtx);
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

extern "C" void RenderSingleDrawGouraud(void* x)
{
    Unk803E3C08 a;
    a.m8 = &lbl_806EFA74;
    a.m24 = (unsigned char*)lbl_80623260;
    a.m18 = lbl_8062325C;
    a.m14 = &lbl_806EFA78;
    a.m10 = (int*)lbl_806EFA80;
    a.m1C = &lbl_806EFA88;
    a.m20 = &lbl_806EFA8C;
    fn_803E3C08(&a, x, -1);
    if (a.mB0) {
        GXColor c = { 0xff, 0xff, 0xff, 0xff };
        GXSetChanAmbColor(4, c);
        GXSetChanMatColor(4, c);
        fn_803DDE7C();
        fn_803DDBF0();
    }
    if (a.mB4 != 0 || a.m50[10] != lbl_8062326C.m34 || a.m50[14] != lbl_8062326C.m44) {
        a.mB8 = 0;
        GXClearVtxDesc();
        lbl_806232A0 = a.m50[10];
        if (lbl_806232A0) {
            GXSetVtxDesc(9, 2);
        } else {
            GXSetVtxDesc(9, 3);
        }
        lbl_806232B0 = a.m50[14];
        if (lbl_806232B0) {
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
        lbl_8062326C.m0 = 2;
        lbl_8062326C.m8 = 0x1200;
        lbl_8062326C.m6C = 1;
        fn_803DB9C0(0x1200, -1);
        lbl_8062326C.m70 = 1;
    }
    fn_803DA468(9, lbl_806EFA80[0], 0xc);
    fn_803DA468(0xb, lbl_806EFA80[1], 4);
    fn_80368E64(&a, &lbl_8062326C);
    if (a.m4) {
        while (a.m4[0] == 2) {
            a.m0[a.m4[1]].m4 = (void*)((int)a.m0[a.m4[1]].m4 - *(int*)(a.m4 + 2) * lbl_80627290);
            a.m4 += 6;
        }
    }
}

Unk803DA440 lbl_80693CFC((void (*)())RenderSingleDrawGouraud, (void*)lbl_80623230, (void*)lbl_80623248, (void*)lbl_80603718);
