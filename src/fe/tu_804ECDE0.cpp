/*
 * Front-end overlay code, .frontend_codeoverlay 0x804ECDE0-0x804F81E0 (code-only unit;
 * placeholder TU). Define every function FE_OVERLAY (game/overlay.h); data extern by lbl_ name.
 */

#include "game/fe_804E02E0.h"

/* Q16-DECL-BEGIN */
extern "C" char* strcpy(char* d, const char* s);
extern char lbl_805FB1B8[];
extern char lbl_805FB1C8[];
extern char lbl_805FB1D0[];
extern char lbl_805FB1D4[];
extern char lbl_805FB1E0[];
extern char lbl_805FB1AC[];
struct Unk8019F8C0_Owner;
struct FeOwnRet {
    int m0;
    int m4;
};
FeOwnRet* fn_801A04A8(Unk8019F8C0_Owner* o, int a);
struct Unk802ACD88_Mgr;
void fn_802AE5D8(Unk802ACD88_Mgr* m);
void fn_802B8610(Unk802ACD88_Mgr* m);
#define FE_LO30(X)     X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9) X(10) X(11) X(12) X(13) X(14) X(15) X(16) X(17)     X(18) X(19) X(20) X(21) X(22) X(23) X(24) X(25) X(26) X(27) X(28) X(29) X(30)
class FeObjR31 : public UiBase {
public:
    FE_LO30(FE_V0) virtual const void* v31(); virtual FeObj* v32(const char* a, int b); FE_HI(FE_V0)
};
struct FeNm : FeObj {
    char pad40[0x594 - 0x40];
    Unk8019F8C0_Owner* m594;
    char m598[0x634 - 0x598];
    int m634;
    int m638;
    char pad63C[0x654 - 0x63C];
    FeObjR31* m654;
    FeObjR31* m658;
};
struct FeT9 {
    int v[9];
};
extern FeT9 lbl_805FB188;
extern "C" int strcmp(const char* a, const char* b);
extern "C" void Unicode2Ascii(char* dst, const void* src);
void fn_800E633C(char* s);
void fn_800E6158();
void fn_800E6258(int a, int b);
class FeVt9 {
public:
    char pad0[8];
    virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
};
struct FeM618 {
    char pad0[0xC];
    FeVt9* mC;
};
struct FeOwn618 {
    char pad0[0x618];
    FeM618* m618;
};
class FeVt18 {
public:
    char pad0[0x18];
    virtual void s1(int a);
};
FeOwn618* fn_801A4C20(Unk8019F8C0_Owner* o);
extern "C" void fn_80244A64(FeOwn618* o, int a, int b);
extern "C" void fn_803814A4(FeM618* p, int a, int b, float f);
extern "C" void fn_8037EB3C(FeM618* p, int a);
extern "C" void fn_804F8A58(void* self);
void fn_802B8CC8();
void fn_802AE5A0(Unk802ACD88_Mgr* m);
extern "C" {
void fn_8037C7FC();
void fn_8037CCE0();
void fn_804F6898(FeNm* self, int a, int b);
void fn_804F3560(int flag, FeNm* self);
void fn_804F365C(int flag, FeNm* self);
void fn_804F3718(FeNm* self);
void fn_804F8BC8(FeNm* self);
}
class FeNd {
public:
    char pad0[8];
    FeNd* m8;
    char nm[0x24 - 0xC];
    int m24;
    char pad28[7];
    unsigned char m2F;
    char pad30[0x3C - 0x30];
    virtual void v1();
    virtual void v2();
};
struct FeLst : FeObj {
    FeNd* m40;
    char pad44[0x594 - 0x44];
    Unk8019F8C0_Owner* m594;
};
struct FeX3C {
    char pad0[0x3C];
    int m3C;
};
struct FeQ16 {
    int w[4];
};
extern char lbl_805FB228[];
extern float lbl_805FB238;
extern "C" {
FeQ16* fn_8014D9C4(FeQ16* p);
void fn_80318140(int a, int b, int c, float x1, float x2, float x3, float x4, int d, void* e, int f, float x5, int g, int h, int s0, int s1);
}
extern "C" void fn_8040074C(void* p, int n);
extern const char lbl_8064B298[];
extern const char lbl_8064A550[];
extern const char lbl_8064A4C8[];
extern const char lbl_806816F0[];
struct FeAr {
    int* m0;
    int m4;
    int m8;
    int mC;
    const void* vp;
    int m14;
};

inline void FeArBase(FeAr* a, int* buf, int cap)
{
    a->vp = lbl_8064B298;
    a->m0 = buf;
    a->m8 = cap;
    fn_8040074C(a->m0, cap * 4);
}

inline void FeArMid(FeAr* a)
{
    a->m4 = 0;
    a->mC = 0;
    a->vp = lbl_8064A550;
    fn_8040074C(a->m0, a->m8 * 4);
}

inline void FeArD3(FeAr* a)
{
    a->mC = 0;
    a->vp = lbl_8064A4C8;
    a->m14 = 0;
    a->m4 = 0;
}

struct FeTri {
    int a;
    int b;
    int c;
};
struct FeCapC {
    char pad0[0x3C];
    const void* m3C;
    char pad40[0x130 - 0x40];
    int m130;
    char pad134[0x13C - 0x134];
    FeAr ar;
    int buf[40];
    FeTri e[25];
    float m320;
    float m324;
    float m328;
    float m32C;
    char pad330[0x594 - 0x330];
    int m594;
    char pad598[0x600 - 0x598];
    int m600;
    char pad604[0x60C - 0x604];
    int m60C;
    int m610;
    int m614;
    int m618;
    char pad61C[0x630 - 0x61C];
    int m630;
    int m634;
    int m638;
    char pad63C[0x7F4 - 0x63C];
    int m7F4[14];
    int m82C[9];
    char pad850[0x85C - 0x850];
    int m85C;
    int m860;
    char pad864[0x870 - 0x864];
    int m870;
};
extern "C" int fn_80077BC4(void* g, int a);
class FeSubI8 {
public:
    char pad0[0x24];
    virtual int s1();
    virtual int s2();
    virtual int s3();
    virtual int s4();
    virtual int s5();
    virtual int s6();
    virtual int s7();
    virtual void s8(int a);
};
struct FeNm364 : FeObj {
    char pad40[0x364 - 0x40];
    int m364;
};
extern "C" void fn_804FCBAC();
/* Q16-DECL-END */

extern char lbl_8067F178[];

extern "C" FE_OVERLAY FeScrRaw* fn_804ECDE0(FeScrRaw* self, int a, int b, int c, int d, int e, int f, int g, int h)
{
    fn_800EC648(self, a, b, c, d, e, f, g, h);
    self->m3C = lbl_8067F178;
    self->m130 = 0;
    return self;
}

extern "C" FE_OVERLAY void fn_804ECE2C(FeWf134* self)
{
    fn_800ECF18(self);
    self->m134 = fn_800B7AB8(UiGetMsgMgr(), "NAVBAR");
    fn_80124FA8(self->m134, self);
    fn_80125154(self->m134, 0);
}

extern "C" FE_OVERLAY void fn_804ECEA8(FeObj* self, int a)
{
    UiMsg m;
    UiMsg* p = &m;
    fn_8035682C(p);
    if (a == 0) {
        fn_8040B830(p->name, "BACK", 15);
        p->name[15] = 0;
        fn_800B7B4C(UiGetMsgMgr(), p, self);
    }
}

extern "C" FE_OVERLAY FeCapC* fn_804ECF34(FeCapC* self, int a, int b, int c, int d, int e, int f, int g, int h)
{
    fn_800EC648((FeScrRaw*)self, a, b, c, d, e, f, g, h);
    self->m3C = lbl_806816F0;
    self->m130 = 0;
    FeArBase(&self->ar, self->buf, 0x28);
    FeArMid(&self->ar);
    FeArD3(&self->ar);
    self->m600 = 0;
    self->m60C = 0;
    self->m610 = 0;
    self->m630 = 0;
    self->m870 = 0;
    self->m594 = 0;
    Unk8007A4DC_Screen* sc = (Unk8007A4DC_Screen*)lbl_8061930C;
    for (int i = 0; i <= 0x18; i++) {
        self->e[i].a = fn_800765A8(i);
        self->e[i].b = sc->fn_8007A6A0(0xB, self->e[i].a) == 1;
        self->e[i].c = fn_80077BC4(sc, self->e[i].a) == 0;
    }
    for (int j = 0; j < 9; j++)
        self->m82C[j] = 0;
    for (int j = 0; j < 14; j++)
        self->m7F4[j] = 0;
    self->m320 = 5.555556f;
    self->m324 = 5.0f;
    self->m328 = 160.0f - self->m320 * 66.0f;
    self->m32C = 190.0f - self->m324 * 66.0f;
    self->m634 = 0;
    self->m638 = 0;
    self->m85C = 0;
    self->m860 = 0;
    self->m618 = 0;
    return self;
}

extern "C" FE_OVERLAY void fn_804ED114(FeCapC* self, int chrg)
{
    self->m3C = lbl_806816F0;
    FeOwn618* o = fn_801A4C20((Unk8019F8C0_Owner*)self->m594);
    fn_80244A64(o, 0, 0);
    fn_803814A4(o->m618, 0, 0, 0.0f);
    fn_8037EB3C(o->m618, 0);
    o->m618->mC->s9();
    fn_804F8A58(self);
    fn_802B8CC8();
    fn_802AE5A0((Unk802ACD88_Mgr*)lbl_8061E978);
    FeVt18* p = (FeVt18*)self->m594;
    if (p)
        p->s1(3);
    fn_800E2BCC();
    self->ar.vp = lbl_8064A550;
    fn_8035686C(self, chrg);
}

extern "C" FE_OVERLAY void fn_804EEEFC(FeNm* self)
{
    fn_801A04A8(self->m594, 1)->m4 = 1;
    lbl_8061D384 = lbl_806221F8;
    lbl_8061D388 = lbl_806221FC;
    fn_800ED254(self);
    fn_8037C7FC();
    fn_8037CCE0();
    fn_802AE5D8((Unk802ACD88_Mgr*)lbl_8061E978);
    fn_802B8610((Unk802ACD88_Mgr*)lbl_8061E978);
}

extern "C" FE_OVERLAY void fn_804F32AC(void* self, FeHolder* o, int n)
{
    FeT9 t = lbl_805FB188;
    ((FeSubB*)&o->mDC)->s15();
    ((FeObj2*)o)->v51(0x4B3, 0);
    for (int i = 0; i < n; i++)
        ((FeObj2*)o)->v51(t.v[i], i + 1);
}

extern "C" FE_OVERLAY int fn_804F33C0(FeNm* self, int kind)
{
    FeObj* p = fn_800B7A94(UiGetMsgMgr(), "POPUP");
    switch (kind) {
    case 0:
        fn_80131F34(p, self, (FeCb)fn_804F3560, fn_805283F4, 0x38B8, 1, -1, 1, 0x26DE, 0, 0, 0, 0, 0);
        break;
    case 1: {
        int id = 0x3A70;
        if (fn_800B555C()->m0 == 0x2E)
            id = 0x3A6F;
        fn_80131F34(p, self, (FeCb)fn_804F365C, fn_805283F4, id, 2, -1, 1, 0x3E9, 0x3E8, 0, 0, 0, 0);
        break;
    }
    case 2:
        self->m634 = 0;
        fn_80131F34(p, self, (FeCb)fn_804F3560, fn_805283F4, 0x3958, 1, -1, 1, 0x26DE, 0, 0, 0, 0, 0);
        break;
    }
    p->v86();
    return 0;
}

extern "C" FE_OVERLAY void fn_804F3560(int flag, FeNm* self)
{
    if (self->m634 != 0) {
        strcpy(self->m598, lbl_805FB1AC);
        ((FeObj1*)self)->v80(1);
        fn_800B7ADC(UiGetMsgMgr(), self);
    } else if (self->m638 == 0) {
        strcpy(self->m598, lbl_805FB1B8);
        ((FeObj1*)self)->v80(1);
        fn_800B7ADC(UiGetMsgMgr(), self);
    }
}

extern "C" FE_OVERLAY void fn_804F365C(int flag, FeNm* self)
{
    if (flag != 0) {
        fn_804F3718(self);
        lbl_8061D384 = lbl_806221F8;
        lbl_8061D388 = lbl_806221FC;
        fn_804F8BC8(self);
        strcpy(self->m598, lbl_805FB1B8);
        ((FeObj1*)self)->v80(1);
        fn_800B7ADC(UiGetMsgMgr(), self);
    }
}

extern "C" FE_OVERLAY void fn_804F3718(FeNm* self)
{
    if (fn_800B555C()->m0 != 0x2E) {
        char a[16];
        char b[16];
        char* k1 = lbl_805FB1C8;
        char* k2 = lbl_805FB1D0;
        Unicode2Ascii(a, self->m654->v31());
        Unicode2Ascii(b, self->m658->v31());
        fn_800E633C(a);
        fn_800E633C(b);
        if (strcmp(a, k1) == 0 && strcmp(b, k2) == 0) {
            fn_800E6158();
            return;
        }
        if (strcmp(a, lbl_805FB1D4) == 0 && strcmp(b, lbl_805FB1E0) == 0) {
            fn_800E6258(lbl_8061930C->fn_8007DA28(), 4);
        }
    }
}

extern "C" FE_OVERLAY void fn_804F4030(FeLst* self)
{
    if (self->m24 < 0) {
        FeNd* n = self->m40;
        if (n != 0) {
            do {
                int c = strcmp(n->nm, lbl_805FB228);
                if (c == 0) {
                    FeQ16 q1;
                    FeQ16 q2;
                    fn_8014D9C4(&q2);
                    unsigned char bb = n->m2F;
                    float x0 = lbl_805FB238;
                    float x1 = lbl_805FB238;
                    float x2 = lbl_805FB238;
                    float x3 = lbl_805FB238;
                    float x4 = lbl_805FB238;
                    fn_80318140(((FeX3C*)lbl_8061FEDC)->m3C, 0, (int)&q1, x0, x1, x2, x3, bb, &q2, 0, x4, (int)self->m594, 0, c, c);
                } else if (n->m24 < 0)
                    n->v2();
                n = n->m8;
            } while (n != 0);
        }
    }
}

extern "C" FE_OVERLAY void fn_804F4B2C(void* self, FeHolder* o)
{
    ((FeSubB*)&o->mDC)->s15();
    ((FeObj2*)o)->v51(0x35C, 0);
    ((FeSubB*)&o->mDC)->s6(0);
}

extern "C" FE_OVERLAY void fn_804F67E4(FeNm* self)
{
    fn_804F6898(self, 0, 0);
    fn_804F6898(self, 1, 0);
    fn_804F6898(self, 3, 0);
    fn_804F6898(self, 4, 0);
    fn_804F6898(self, 5, 0);
    fn_804F6898(self, 6, 0);
    fn_804F6898(self, 7, 0);
    fn_804F6898(self, 8, 0);
    fn_804F6898(self, 2, 0);
}

extern "C" FE_OVERLAY int fn_804F7C80(int a, FeObj1* self)
{
    if (a != 0) {
        self->v80(1);
        fn_800B7ADC(UiGetMsgMgr(), self);
    }
}

extern "C" FE_OVERLAY int fn_804F7CFC(FeS17* self)
{
    FeObj* p = fn_800B7A94(UiGetMsgMgr(), "POPUP");
    lbl_8061D384 = lbl_806221F8;
    lbl_8061D388 = lbl_806221FC;
    int id = 0x3A97;
    if (self->m630 != 1)
        id = 0x3A6E;
    fn_80131F34(p, self, (FeCb)fn_804F7C80, fn_805283F4, id, 2, -1, 1, 0x3E9, 0x3E8, 0, 0, 0, 0);
    p->v86();
    return 1;
}

extern "C" FE_OVERLAY int fn_804F7DF8(FeWf604b* self)
{
    self->m604->v40(1);
    return 1;
}

extern "C" FE_OVERLAY int fn_804F7E38(FeWf604* self)
{
    self->m604->v41();
    return 1;
}

extern "C" FE_OVERLAY int fn_804F7E74(FeWf604b* self)
{
    self->m604->v38(1);
    return 1;
}

extern "C" FE_OVERLAY int fn_804F7EB4(FeWf604* self)
{
    self->m604->v39();
    return 1;
}

extern "C" FE_OVERLAY int fn_804F7EF0(FeS17* self)
{
    fn_804FC260(self);
    if (self->m618 != 0)
        self->m618->v50();
    FeSub* sb = (FeSub*)((char*)fn_800C8058(((FeObjQ*)self->m604)->v33(10, 1)) + 0xDC);
    if ((unsigned)(sb->s5() - 6) <= 1)
        fn_804FBF58(self);
    return 1;
}

extern "C" FE_OVERLAY int fn_804F7F9C(FeWf618* self)
{
    if (self->m618 != 0)
        self->m618->v51();
    return 1;
}

extern "C" FE_OVERLAY int fn_804F7FE0(FeS17* self)
{
    fn_804FC260(self);
    if (self->m618 != 0)
        self->m618->v52();
    FeSub* sb = (FeSub*)((char*)fn_800C8058(((FeObjQ*)self->m604)->v33(10, 1)) + 0xDC);
    if ((unsigned)(sb->s5() - 6) <= 1)
        fn_804FBF58(self);
    return 1;
}

extern "C" FE_OVERLAY int fn_804F808C(FeWf618* self)
{
    if (self->m618 != 0)
        self->m618->v53();
    return 1;
}

extern "C" FE_OVERLAY int fn_804F80D0(FeWf618* self)
{
    if (self->m618 != 0)
        self->m618->v56();
    return 1;
}

extern "C" FE_OVERLAY int fn_804F8114(FeWf618* self)
{
    if (self->m618 != 0)
        self->m618->v57();
    return 1;
}

extern "C" FE_OVERLAY int fn_804F8158(FeWf618* self)
{
    if (self->m618 != 0)
        self->m618->v54();
    return 1;
}

extern "C" FE_OVERLAY int fn_804F819C(FeWf618* self)
{
    if (self->m618 != 0)
        self->m618->v55();
    return 1;
}
