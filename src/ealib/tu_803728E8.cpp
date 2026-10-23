/*
 * EA library TU, .text 0x803728E8-0x80372F60 (static-init anchored; issue #70).
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

struct Unk803728E8Mtx {
    float f[16];
};

struct Unk803DE1BC {
    int m[26];
};

struct Unk803728E8Param {
    void* m0;
    void* m4;
};

/*
 * Object built by fn_803E3C08 (x, -1). Offsets seen in fn_80372C30 (stack
 * object, 0xC0 bytes) and fn_803728E8.
 */
struct Unk803E3C08 {
    Unk803728E8Param* m0;
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

struct Unk80623D04 {
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
void fn_803DFBAC(Unk803728E8Mtx* in, Unk803728E8Mtx* out);
void GXLoadPosMtxImm(Unk803728E8Mtx* m, int id);
void GXSetCurrentMtx(int id);
void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);
void fn_803DB5A0(void* state);
void fn_803DDCD4(void);
void GXCallDisplayList(void* list, int nbytes);
void fn_803EA338(void* a, void* b, void* c, void* d);
void fn_803DBBE0(void* tex);
void fn_803DE1BC(Unk803DE1BC* o);
void fn_803DE2E8(Unk803DE1BC* o, int a, int b);
void fn_803DE27C(Unk803DE1BC* o, int a, int b, int c, int d);
void fn_803DE25C(Unk803DE1BC* o, int a, int b, int c, int d);
void fn_803DE300(Unk803DE1BC* o, int a);
}

extern char lbl_80623CA0[];
extern char lbl_80623CCC[];
extern char lbl_80604BB8[];

extern int lbl_80627290;
extern void (*lbl_806279C8)(void);
extern "C" void DisableCoPlanarZMode(void);
extern int lbl_80623CF4;
extern char lbl_80623CF8[];
extern Unk80623D04 lbl_80623D04;
extern int lbl_80623D38;
extern int lbl_80623D48;
extern int lbl_806EFB84;
extern int lbl_806EFB88;
extern int lbl_806EFB8C;
extern int lbl_806EFB90;
extern int lbl_806EFB94;

extern "C" void fn_803728E8(Unk803E3C08* a, Unk80623D04* b)
{
    Unk803DE1BC o;
    if (a->mB0) {
        b->m4 = a->m0[9].m4;
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
        Unk803728E8Param* p = a->m0;
        c2.r = (unsigned char)(((float*)p[8].m4)[0] * 255.0f);
        c2.g = (unsigned char)(((float*)p[8].m4)[1] * 255.0f);
        c2.b = (unsigned char)(((float*)p[8].m4)[2] * 255.0f);
        c2.a = (unsigned char)(((float*)p[8].m4)[3] * 255.0f);
        GXSetChanAmbColor(4, c1);
        GXSetChanMatColor(4, c2);
        GXSetChanCtrl(4, 0, 0, 0, 0, 2, 2);
        fn_803DB5A0(b->m4);
        if (b->m6C == 0) {
            fn_803DB9C0(b->m8, 3);
        }
        Unk803728E8Mtx mtx = *(Unk803728E8Mtx*)a->m0[4].m4;
        fn_803DFBAC(&mtx, &mtx);
        GXLoadPosMtxImm(&mtx, 0);
        GXSetCurrentMtx(0);
        fn_803DBBE0(a->m0[7].m4);
        lbl_806279C8 = DisableCoPlanarZMode;
    }
    fn_803EA338(a->m0[2].m4, a->m0[2].m0, a->m0[1].m4, a->m0[3].m4);
    fn_803DDCD4();
    void* dl = a->m28;
    int dlsz = a->m2C;
    if (a->mB8) {
        EALIB_GX_FIFO(PPCWGPipe).u8 = 0x40;
        EALIB_GX_FIFO(PPCWGPipe).u32 = (unsigned int)dl;
        EALIB_GX_FIFO(PPCWGPipe).u32 = dlsz;
    } else {
        GXCallDisplayList(dl, dlsz);
    }
}

extern "C" void fn_80372C30(void* x)
{
    Unk803E3C08 a;
    a.m8 = &lbl_806EFB84;
    a.m24 = (unsigned char*)lbl_80623CF8;
    a.m18 = lbl_80623CF4;
    a.m14 = &lbl_806EFB88;
    a.m10 = &lbl_806EFB8C;
    a.m1C = &lbl_806EFB90;
    a.m20 = &lbl_806EFB94;
    fn_803E3C08(&a, x, -1);
    if (a.mB0) {
        GXColor c = { 0xff, 0xff, 0xff, 0xff };
        GXSetChanAmbColor(4, c);
        GXSetChanMatColor(4, c);
        fn_803DDE7C();
        fn_803DDBF0();
    }
    if (a.mB4 != 0 || a.m50[10] != lbl_80623D04.m34 || a.m50[14] != lbl_80623D04.m44) {
        a.mB8 = 0;
        GXClearVtxDesc();
        GXSetVtxDesc(0, 1);
        lbl_80623D38 = a.m50[10];
        if (lbl_80623D38) {
            GXSetVtxDesc(9, 2);
        } else {
            GXSetVtxDesc(9, 3);
        }
        lbl_80623D48 = a.m50[14];
        if (lbl_80623D48) {
            GXSetVtxDesc(0xb, 2);
        } else {
            GXSetVtxDesc(0xb, 3);
        }
    }
    if (a.mB4) {
        fn_803DBF78(0, 9, 1, 4, 0);
        fn_803DBF78(0, 0xb, 1, 3, 0);
        fn_803DDB5C(0);
        GXSetNumChans(1);
        lbl_80623D04.m0 = 2;
        lbl_80623D04.m8 = 0x1201;
        lbl_80623D04.m6C = 1;
        fn_803DB9C0(0x1201, -1);
        lbl_80623D04.m70 = 1;
    }
    fn_803DA468(9, a.m0[5].m4, 0xc);
    fn_803DA468(0xb, a.m0[6].m4, 2);
    fn_803728E8(&a, &lbl_80623D04);
    if (a.m4) {
        while (a.m4[0] == 2) {
            a.m0[a.m4[1]].m4 = (void*)((int)a.m0[a.m4[1]].m4 - *(int*)(a.m4 + 2) * lbl_80627290);
            a.m4 += 6;
        }
    }
}

Unk803DA440 lbl_80693E00((void (*)())fn_80372C30, lbl_80623CA0, lbl_80623CCC, lbl_80604BB8);
