/*
 * EA library TU, .text 0x80367140-0x80367658 (TU start is a guess: the previous
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

struct Unk80367140Param {
    void* m0;
    void* m4;
};

/*
 * Object built by fn_803E3C08 (x, -1). Offsets seen in fn_803672A0 (stack
 * object, 0xC0 bytes) and fn_80367140.
 */
struct Unk803E3C08 {
    Unk80367140Param* m0;
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

struct Unk8062300C {
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

extern const char lbl_80622FB0[];
extern const char lbl_80622FD8[];
extern const char lbl_806032DC[];

extern int lbl_80627290;
extern int lbl_80622FFC;
extern char lbl_80623000[];
extern Unk8062300C lbl_8062300C;
extern int lbl_80623040;
extern int lbl_80623050;
extern int lbl_80623058;
extern int lbl_806EFA34;
extern int lbl_806EFA38;
extern int lbl_806EFA3C;
extern int lbl_806EFA40;
extern int lbl_806EFA44;

extern "C" void fn_80367140(Unk803E3C08* a, Unk8062300C* b)
{
    fn_803EA338(a->m0[2].m4, a->m0[2].m0, a->m0[1].m4, a->m0[3].m4);
    if (a->mB0) {
        b->m4 = a->m0[8].m4;
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
        fn_803DD794(a->m0[7].m4);
    }
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

extern "C" void fn_803672A0(void* x)
{
    Unk803E3C08 a;
    a.m8 = &lbl_806EFA34;
    a.m24 = (unsigned char*)lbl_80623000;
    a.m18 = lbl_80622FFC;
    a.m14 = &lbl_806EFA38;
    a.m10 = &lbl_806EFA3C;
    a.m1C = &lbl_806EFA40;
    a.m20 = &lbl_806EFA44;
    fn_803E3C08(&a, x, -1);
    if (a.mB0) {
        GXColor c = { 0xff, 0xff, 0xff, 0xff };
        GXSetChanAmbColor(4, c);
        GXSetChanMatColor(4, c);
        fn_803DDE7C();
        fn_803DDBF0();
    }
    if (a.mB4 != 0 || a.m50[10] != lbl_8062300C.m34 || a.m50[14] != lbl_8062300C.m44
        || a.m50[16] != lbl_8062300C.m4C) {
        a.mB8 = 0;
        GXClearVtxDesc();
        GXSetVtxDesc(0, 1);
        lbl_80623040 = a.m50[10];
        if (lbl_80623040) {
            GXSetVtxDesc(9, 2);
        } else {
            GXSetVtxDesc(9, 3);
        }
        lbl_80623050 = a.m50[14];
        if (lbl_80623050) {
            GXSetVtxDesc(0xb, 2);
        } else {
            GXSetVtxDesc(0xb, 3);
        }
        lbl_80623058 = a.m50[16];
        if (lbl_80623058) {
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
        lbl_8062300C.m0 = 3;
        lbl_8062300C.m8 = 0x5201;
        lbl_8062300C.m6C = 1;
        fn_803DB9C0(0x5201, -1);
        lbl_8062300C.m70 = 1;
    }
    fn_803DA468(9, a.m0[4].m4, 0xc);
    fn_803DA468(0xb, a.m0[5].m4, 4);
    fn_803DA468(0xd, a.m0[6].m4, 8);
    fn_80367140(&a, &lbl_8062300C);
    if (a.m4) {
        while (a.m4[0] == 2) {
            a.m0[a.m4[1]].m4 = (void*)((int)a.m0[a.m4[1]].m4 - *(int*)(a.m4 + 2) * lbl_80627290);
            a.m4 += 6;
        }
    }
}

Unk803DA440 lbl_80693CC0((void (*)())fn_803672A0, (void*)lbl_80622FB0, (void*)lbl_80622FD8, (void*)lbl_806032DC);
