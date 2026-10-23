/*
 * EA library TU, .text 0x8036E65C-0x8036F44C (static-init anchored).
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

struct Mtx34 {
    float f[12];
};

struct GXColor {
    unsigned char r, g, b, a;
};

struct Unk8036E65CParam {
    void* m0;
    void* m4;
};

/*
 * Object built by fn_803E3C08 (x, -1). Offsets seen in fn_8036F010 (stack
 * object, 0xC0 bytes) and fn_8036E65C.
 */
struct Unk803E3C08 {
    Unk8036E65CParam* m0;
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
    int m50;
    int pad54[6];
    int m6C;
    int m70;
};

extern "C" {
void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);
int fn_803DB5A0(void* state);
void fn_803DDCD4(void);
void GXCallDisplayList(void* list, int nbytes);
void fn_803DD794(void* tex);
void GXSetTevKColor(int id, GXColor c);
void fn_803DE2C0(void* o, int a);
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


extern "C" {
void fn_803EA3EC(void* a, void* b, void* c, void* d);
int fn_803DD7C8(void* tex, int a);
void fn_803DD8E0(void* tex, int a, int b);
void fn_803DD794(void* tex);
void fn_803DE3D0(int a);
void fn_803DE29C(void* o, int a, int b, int c, int d, int e);
void fn_803DE27C(void* o, int a, int b, int c, int d);
void fn_803DE25C(void* o, int a, int b, int c, int d);
void fn_803DE2D4(void* o, int a);
void PSMTXScale(Mtx34* m, float x, float y, float z);
void PSMTXTrans(Mtx34* m, float x, float y, float z);
void PSMTXConcat(Mtx34* a, Mtx34* b, Mtx34* ab);
void GXLoadTexMtxImm(Mtx34* m, int id, int type);
void GXCallDisplayList(void* list, int nbytes);
}

struct Unk803DE29C {
    char pad[0x64];
};

/* Global 0x640-byte state; sub-objects at +0 and +0x64 (fn_803DE2xx). */
struct Unk8062BC30 {
    Unk803DE29C m0;
    Unk803DE29C m64;
    char pad[0x5dc];
};

extern Unk8062BC30 lbl_8062BC30;

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
void fn_8036CC08(Unk803E3C08* a, UnkCfg* b);
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
void fn_8036E65C(Unk803E3C08* a, UnkCfg* b);
}

extern char lbl_80623800[];
extern char lbl_80623840[];
extern char lbl_80604398[];

extern int lbl_80627290;
extern UnkCfg lbl_80623894;
extern int lbl_806238C8;
extern int lbl_806238CC;
extern int lbl_806238D8;
extern int lbl_806238E0;
extern int lbl_8062387C;
extern char lbl_80623880[];
extern int lbl_806EFB10;
extern int lbl_806EFB14;
extern int lbl_806EFB18;
extern int lbl_806E4678;
extern int lbl_806EFB1C;

extern "C" void fn_8036E65C(Unk803E3C08* a, UnkCfg* b)
{
    Mtx34 m1;
    Mtx34 m2;
    Mtx34 m3;
    unsigned i1;
    unsigned i2;
    fn_803EA3EC(a->m0[2].m4, a->m0[2].m0, a->m0[1].m4, a->m0[3].m4);
    if (a->mB0) {
        Unk8062BC30* q = &lbl_8062BC30;
        b->m4 = a->m0[13].m4;
        fn_803DE2D4(q, 0);
        fn_803DE27C(q, 0xf, 0xa, 8, 0xf);
        fn_803DE25C(q, 7, 4, 6, 7);
        fn_803DD7C8(a->m0[9].m4, 0);
        fn_803DDB5C(2);
        fn_803DDB88(1, 0, 1, 0x1e, 1, 0x40);
        fn_803DE3D0(2);
        fn_803DE29C(&q->m64, 0, 0, 0, 1, 0);
        fn_803DE27C(&q->m64, 0xf, 8, 0, 0xf);
        fn_803DE25C(&q->m64, 7, 7, 7, 0);
        if (a->m30[1]) {
            fn_803DD8E0((char*)a->m0[9].m4 + 4, 1, 1);
        }
        m3.f[0] = ((float*)a->m0[14].m4)[0];
        m3.f[1] = 0.0f;
        m3.f[2] = 0.0f;
        m3.f[3] = ((float*)a->m0[14].m4)[2];
        m3.f[4] = 0.0f;
        m3.f[5] = ((float*)a->m0[14].m4)[1];
        m3.f[6] = 0.0f;
        m3.f[7] = ((float*)a->m0[14].m4)[3];
        m3.f[8] = 0.0f;
        m3.f[9] = 0.0f;
        m3.f[10] = 1.0f;
        m3.f[11] = 0.0f;
        PSMTXScale(&m1, 0.5f, -0.5f, 0.0f);
        PSMTXTrans(&m2, 0.5f, 0.5f, 1.0f);
        PSMTXConcat(&m3, &m2, &m2);
        PSMTXConcat(&m2, &m1, &m2);
        GXLoadTexMtxImm(&m2, 0x40, 0);
        float* kf = (float*)a->m0[11].m4;
        fn_803DD7C8(a->m0[10].m4, 0);
        fn_803DDB5C(3);
        fn_803DDB88(2, 1, 1, 0x21, 1, 0x43);
        GXColor kc;
        kc.a = (unsigned char)(kf[3] * 255.0f);
        GXSetTevKColor(0, kc);
        fn_803DE3D0(3);
        fn_803DE2C0((char*)q + 0xc8, 0x1c);
        fn_803DE29C((char*)q + 0xc8, 0, 0, 0, 1, 0);
        fn_803DE27C((char*)q + 0xc8, 0xf, 8, 0xe, 0);
        fn_803DE25C((char*)q + 0xc8, 7, 7, 7, 0);
        if (a->m30[2]) {
            fn_803DD8E0((char*)a->m0[10].m4 + 4, 2, 2);
        }
        m1.f[0] = 0.5f;
        m1.f[1] = 0.0f;
        m1.f[2] = 0.0f;
        m1.f[3] = 0.5f;
        m1.f[4] = 0.0f;
        m1.f[5] = -0.5f;
        m1.f[6] = 0.0f;
        m1.f[7] = 0.5f;
        m1.f[8] = 0.0f;
        m1.f[9] = 0.0f;
        m1.f[10] = 0.0f;
        m1.f[11] = 1.0f;
        GXLoadTexMtxImm(&m1, 0x43, 0);
        GXColor c;
        Unk8036E65CParam* pp = a->m0;
        c.r = (unsigned char)(((float*)pp[12].m4)[0] * 255.0f);
        c.g = (unsigned char)(((float*)pp[12].m4)[1] * 255.0f);
        c.b = (unsigned char)(((float*)pp[12].m4)[2] * 255.0f);
        c.a = (unsigned char)(((float*)pp[12].m4)[3] * 255.0f);
        GXSetChanAmbColor(4, c);
        if (c.a == 0xff) {
            GXSetChanCtrl(0, 1, 0, 1, 0, 0, 2);
            GXSetChanCtrl(2, 0, 1, 1, 0, 0, 2);
        } else {
            GXSetChanCtrl(4, 1, 0, 1, 0, 0, 2);
        }
        fn_803DB5A0(b->m4);
        if (b->m6C == 0) {
            fn_803DB9C0(b->m8, 3);
        }
    }
    if (a->m30[0]) {
        fn_803DD794(a->m0[8].m4);
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

extern "C" void fn_8036F010(void* x)
{
    Unk803E3C08 a;
    a.m8 = &lbl_806EFB10;
    a.m24 = (unsigned char*)lbl_80623880;
    a.m18 = lbl_8062387C;
    a.m14 = &lbl_806EFB14;
    a.m10 = &lbl_806EFB18;
    a.m1C = &lbl_806E4678;
    a.m20 = &lbl_806EFB1C;
    fn_803E3C08(&a, x, -1);
    if (a.mB0) {
        GXColor c = { 0xff, 0xff, 0xff, 0xff };
        GXSetChanAmbColor(4, c);
        GXSetChanMatColor(4, c);
        fn_803DDE7C();
        fn_803DDBF0();
    }
    if (a.mB4 != 0 || a.m50[10] != lbl_80623894.m34 || a.m50[11] != lbl_80623894.m38 || a.m50[16] != lbl_80623894.m4C || a.m50[14] != lbl_80623894.m44) {
        a.mB8 = 0;
        GXClearVtxDesc();
        GXSetVtxDesc(0, 1);
        GXSetVtxDesc(2, 1);
        GXSetVtxDesc(3, 1);
        lbl_806238C8 = a.m50[10];
        if (lbl_806238C8) {
            GXSetVtxDesc(9, 2);
        } else {
            GXSetVtxDesc(9, 3);
        }
        lbl_806238CC = a.m50[11];
        if (lbl_806238CC) {
            GXSetVtxDesc(0xa, 2);
        } else {
            GXSetVtxDesc(0xa, 3);
        }
        lbl_806238E0 = a.m50[16];
        if (lbl_806238E0) {
            GXSetVtxDesc(0xd, 2);
        } else {
            GXSetVtxDesc(0xd, 3);
        }
        lbl_806238D8 = a.m50[14];
        if (lbl_806238D8) {
            GXSetVtxDesc(0xb, 2);
        } else {
            GXSetVtxDesc(0xb, 3);
        }
    }
    if (a.mB4) {
        fn_803DBF78(0, 9, 1, 4, 0);
        fn_803DBF78(0, 0xa, 0, 3, 0xe);
        fn_803DBF78(0, 0xd, 1, 4, 0);
        fn_803DDB88(0, 1, 4, 0x3c, 0, 0x7d);
        fn_803DBF78(0, 0xb, 1, 5, 0);
        fn_803DDB5C(1);
        GXSetNumChans(1);
        lbl_80623894.m0 = 4;
        lbl_80623894.m8 = 0x560d;
        lbl_80623894.m6C = 1;
        fn_803DB9C0(0x560d, -1);
        lbl_80623894.m70 = 1;
    }
    fn_803DA468(9, a.m0[4].m4, 0xc);
    fn_803DA468(0xa, a.m0[5].m4, 6);
    fn_803DA468(0xd, a.m0[6].m4, 8);
    fn_803DA468(0xb, a.m0[7].m4, 4);
    fn_8036E65C(&a, &lbl_80623894);
    if (a.m4) {
        while (a.m4[0] == 2) {
            a.m0[a.m4[1]].m4 = (void*)((int)a.m0[a.m4[1]].m4 - *(int*)(a.m4 + 2) * lbl_80627290);
            a.m4 += 6;
        }
    }
}

Unk803DA440 lbl_80693D88((void (*)())fn_8036F010, lbl_80623800, lbl_80623840, lbl_80604398);
