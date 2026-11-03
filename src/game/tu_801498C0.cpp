/*
 * Game code, .text 0x801498C0-0x8014BEA4 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_80144CF0.h"

#include "game/tu_8001FDA8.h"
#include "game/tu_802ACD88.h"
extern const char lbl_805C507C[];
extern const char lbl_805C508C[];
extern const char lbl_805C50F0[];
extern const char lbl_805C5100[];
struct Unk8014A468_Obj {
    int m0;
    char pad4[4];
    int m8;
    int mC;
    char pad10[0xC];
    int m1C;
    int m20;
    int m24;
};
extern const char lbl_805C4DD4[];
extern const char lbl_805C5030[];
extern const char lbl_805C5044[];
extern const char lbl_805C5054[];
extern const char lbl_805C506C[];
extern "C" int fn_803FEE14(char*, int);
extern "C" int FILESYS_sizesync(int, int);
char* fn_8040B448(char*, const char*);
struct Unk8014AD2C_V2 {
    float x;
    float y;
};
struct Unk8014AD2C_Pair {
    Unk8014AD2C_V2 a;
    Unk8014AD2C_V2 b;
};
struct Unk8014AD2C_Col {
    int a;
    int b;
    int c;
    int d;
};
struct Unk8014AD2C_Rect {
    float x0;
    float y0;
    float x1;
    float y1;
};
extern "C" Unk8014AD2C_Col* fn_8014DBA0(Unk8014AD2C_Col*, int, int, int, int);
extern "C" void fn_80314034(int, Unk8014AD2C_V2*, Unk8014AD2C_V2*, Unk8014AD2C_Col*);
extern float lbl_8068B4CC[];
struct Unk8014AAA8_Tri {
    Unk8014AD2C_V2 a;
    Unk8014AD2C_V2 b;
    Unk8014AD2C_V2 c;
};
struct Unk8014AAA8_Obj {
    char pad0[0x18];
    float m18;
    char pad1C[0x10];
    float m2C;
    char pad30[4];
    float m34;
    char pad38[4];
    Unk8014AD2C_V2 p3C;
    Unk8014AD2C_V2 p44;
    Unk8014AD2C_V2 p4C;
    Unk8014AD2C_V2 p54;
    Unk8014AD2C_V2 p5C;
    Unk8014AD2C_V2 p64;
};
extern "C" void fn_803141AC(int, Unk8014AD2C_V2*, Unk8014AD2C_V2*, Unk8014AD2C_V2*, Unk8014AD2C_Col*);
struct Unk8014BC64_Va {
    virtual void fn_vslot1();
    virtual void fn_vslot2(int a);
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5();
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8();
    virtual void fn_vslot9();
    virtual void fn_vslot10();
    virtual void fn_vslot11();
    virtual void fn_vslot12();
    virtual void fn_vslot13();
    virtual void fn_vslot14();
    virtual void fn_vslot15();
    virtual void fn_vslot16();
    virtual void fn_vslot17();
    virtual void fn_vslot18();
    virtual void fn_vslot19(int a);
};
struct Unk8014BC64_VbD {
    char pad0[0x4];
};
struct Unk8014BC64_Vb : Unk8014BC64_VbD {
    virtual void fn_vslot1(int a);
};
struct Unk8014BC64_VcD {
    char pad0[0x18];
};
struct Unk8014BC64_Vc : Unk8014BC64_VcD {
    virtual void fn_vslot1(int a);
};
struct Unk800642F0 {
    ~Unk800642F0();
};
struct Unk802BB4FC;
extern void* lbl_8068F1F8;
extern void* lbl_80618FBC;
extern void* lbl_80619948;
void fn_802B8698(Unk802ACD88_Mgr*);
void fn_802B832C(Unk802ACD88_Mgr*);
void fn_802BB518(Unk802BB4FC*, int);
void fn_8001BC58();
void fn_800AA144();
void fn_802B1D04();
void fn_80312544();
void fn_80070E80();
void fn_80090700();
void fn_80071F48();
extern "C" void fn_800B8150();
extern "C" void fn_805210EC();
extern "C" void fn_803224C8();
struct Unk80310934_P38;

struct Unk8001C314 {
    void fn_8001C89C(int);
};
struct Unk80149DD4_C { char pad0[0x28]; int m28; };
struct Unk80149DD4_D { char pad0[0x74]; int m74; };
struct Unk80149DD4_B { char pad0[0xC]; Unk80149DD4_C* mC; Unk80149DD4_D* m10; };
extern "C" void* fn_800C33D0();
extern "C" void fn_800C3A4C(void*);
extern "C" int fn_80400DA4();
extern "C" void fn_8037CDA4(void*);
extern "C" void fn_80148394(void*);
extern "C" void DrawLoadingProgress(int, int, float);
extern "C" void fn_8031618C(int);
extern "C" void fn_801498BC();
void fn_80313BA4(Unk80310934_P38*);
void fn_802AE5D8(Unk802ACD88_Mgr*);
void fn_802AE42C(Unk802ACD88_Mgr*);
void fn_802AE5A0(Unk802ACD88_Mgr*);
void fn_802B865C(Unk802ACD88_Mgr*);
void fn_802B8610(Unk802ACD88_Mgr*);
void fn_802B8A90(Unk802ACD88_Mgr*);
void fn_80317B30();
void fn_80317AB0(int);
extern int lbl_8061B934;
extern int lbl_8061EE14;
struct Unk80149F70_Ent { int m0; int m4; int m8; };
int fn_802B9804(int, int);
extern "C" int fn_803FC410(int, int, int, int, void*);
extern "C" int fn_803FC4C0(int);
extern "C" int fn_80401904(int);
extern "C" void fn_80377488(void*);
struct Unk8014A928_Obj { char pad0[0x18]; float m18; float m1C; float m20; float m24; float m28; float m2C; int m30; float m34; float m38; float m3C; float m40; float m44; float m48; float m4C; float m50; float m54; float m58; float m5C; float m60; float m64; float m68; };
struct Unk8014A310_Obj { int m0; char pad4[4]; int m8; int mC; int m10; int m14; int m18; int m1C; int m20; int m24; };
void* operator new(unsigned int size, int heap);
struct Unk8014A390_Obj { char pad0[0xC]; int mC; char pad10[0xC]; int m1C; };
struct Unk8014A3E8_Obj { char pad0[8]; int m8; int mC; int m10; int m14; int m18; };
struct Unk8014A6D8_Obj { int m0; char pad4[4]; int m8; int mC; };
struct Unk8014A740_Obj { char pad0[8]; int m8; int mC; };
void* operator new[](unsigned int size, int heap);
extern int lbl_806EF9B4;
void fn_8001BC10();
void fn_800AA108();

extern "C" {
void fn_80149F6C();
void fn_8014A0EC();
char* fn_8014A140();
/* near-miss: only the order of the three argument loads of the second fn_803FEBDC call differs */
void fn_8014A1BC(void* self, int a, const char* b);
void fn_8014A290(void* self, int a, int b);
int fn_8014A2B8(void* self, int a, int b, const char* c);
void* fn_8014A310(void* self);
void* fn_8014A344();
float fn_8014A390(Unk8014A390_Obj* p);
int fn_8014A3E8(Unk8014A3E8_Obj* p, int a, int b, int c);
void* fn_8014A6D8(void* self);
void* fn_8014A6F4();
int fn_8014A740(Unk8014A740_Obj* p, int a, int b);
void* fn_8014AA18();
void fn_8014AA64(int a);
void fn_8014AF14();
void fn_8014AF4C();
void fn_8014AFA8();
void fn_8014AFE0();
void fn_8014B018();
void fn_8014B044();
void fn_8014B070();
void fn_8014B09C();
void fn_8014B0C8();
void fn_8014B0F4();
void fn_8014B120();
void fn_8014B14C();
void fn_8014B178();
void fn_8014B1A4();
void fn_8014B1D0();
void fn_8014B1FC();
void fn_8014B228();
void fn_8014B254();
void fn_8014B280();
void fn_8014B2AC();
void fn_8014B2D8();
void fn_8014B304();
void fn_8014B330();
void fn_8014B35C();
void fn_8014B388();
void fn_8014B3B4();
void fn_8014B3E0();
void fn_8014B40C();
void fn_8014B438();
void fn_8014B464();
void fn_8014B490();
void fn_8014B4BC();
void fn_8014B4E8();
void fn_8014BB70();
void fn_8014BE28();
extern int lbl_8061B938;
extern int lbl_8061B93C;
extern int lbl_8061B940;
extern int lbl_8061B944;
extern int lbl_8061B8BC;
extern char lbl_8061B948[];
extern const char lbl_805C4FEC[];
extern char lbl_805C5008[];
int fn_804089B4(char*, const char*, ...);
extern const char lbl_805C5020;
extern const char lbl_805C5024[];
void fn_8040B320(char*, int, int, ...);
int fn_803FEBDC(char*, int, int, int*);
int fn_803FEC9C(int, int);
int fn_803FEC74(int, int, const char*, int, int);
int fn_8040B6A0(const char*);
void fn_8018466C(int, int);
extern void* lbl_8061B978;
extern void* lbl_8061B97C;
extern void* lbl_8061B980;
void* fn_8014A928(void*);
extern int lbl_80620298;
struct Unk8014AAA8_Obj;
void fn_8014AAA8(Unk8014AAA8_Obj*);
void fn_8014AD2C(int);
extern const char lbl_806557A8[];
extern const void* lbl_80687CE4;
extern const void* lbl_80687E6C;
extern const char lbl_8065BDF8[];
void fn_8033CD28(void*, int);
extern const char lbl_8065F878[];
extern const void* lbl_80687F44;
extern const char lbl_8065F8D8[];
extern const void* lbl_80687F58;
extern char lbl_80687F90[];
void fn_8018102C(void*, int);
extern Unk8001FE34 lbl_80688020;
extern char lbl_806881F8[];
void fn_801421D4(void*, int);
extern char lbl_806882F8[];
void fn_8017F384(void*, int);
extern char lbl_8068836C[];
void fn_8035F6C0(void*, int);
extern char lbl_80688390[];
void fn_80358484(void*, int);
extern char lbl_80688398[];
void fn_8035F6E0(void*, int);
extern char lbl_806883F8[];
void fn_8035F700(void*, int);
extern char lbl_80689558[];
void fn_80151910(void*, int);
extern Unk8002118C lbl_806896BC;
extern char lbl_8068977C[];
void fn_80311BEC(void*, int);
extern char lbl_80689968[];
void fn_8003A9E4(void*, int);
extern char lbl_80689B68[];
void fn_8035F61C(void*, int);
extern char lbl_80689BD0[];
void fn_8035B16C(void*, int);
extern char lbl_80689DB4[];
void fn_8035A5A4(void*, int);
extern char lbl_80689DD4[];
void fn_8035D9F8(void*, int);
extern char lbl_80689E50[];
void fn_8035A344(void*, int);
extern char lbl_80689F08[];
void fn_8035A324(void*, int);
extern char lbl_80689F94[];
void fn_8035A304(void*, int);
extern char lbl_80689FA8[];
void fn_801E5EBC(void*, int);
extern char lbl_80689FF8[];
void fn_801E6330(void*, int);
extern char lbl_8068A060[];
void fn_801E6870(void*, int);
extern char lbl_8068A0BC[];
void fn_8035F67C(void*, int);
extern char lbl_8068A0C4[];
void fn_8035F63C(void*, int);
extern char lbl_8068A0CC[];
void fn_8035F65C(void*, int);
extern char lbl_8068A0D4[];
void fn_80359604(void*, int);
extern char lbl_8068A0F4[];
void fn_8030BFB0(void*, int);
extern char lbl_8068A1A0[];
void fn_8030C728(void*, int);
extern char lbl_8068A380[];
void fn_80311D08(void*, int);
extern int lbl_8061930C;
extern void* lbl_8068B618;
extern void* lbl_8061E978;
extern void* lbl_8068DA4C;
extern void* lbl_8062013C;
extern void* lbl_80620140;
extern int lbl_8061FEDC;
void fn_802DC9C0();
void fn_80071F00();
void fn_800906B8();
void fn_8007295C(int);
void fn_800766B8(int, int);
void fn_80070E38();
void fn_800A152C();
void* fn_80148280(void*);
void* fn_802AD720(void*);
void fn_802B85D8(Unk802ACD88_Mgr*);
void fn_80313BCC(int);
void* fn_802BB4FC(void*);
void fn_800B8108();
void fn_805210A0();
void* fn_8031C62C(void*);
void* fn_8031CD88(void*);
void fn_80322450();
extern void* lbl_806EF9C0;
extern const char lbl_805C5260[];
int fn_803ED9D4();
void MEMCLASS_create(int, const char*, void*, int, int, int, int, int, int, int);

void fn_80149DD4(void* a, Unk80149DD4_B* b)
{
    fn_800C3A4C(fn_800C33D0());
    lbl_8061B934 = fn_80400DA4();
    Unk802ACD88_Mgr* m = (Unk802ACD88_Mgr*)lbl_8061E978;
    int kind = b->m10->m74;
    fn_80313BA4((Unk80310934_P38*)lbl_8061FEDC);
    fn_8037CDA4(fn_8037C7FC());
    fn_802AE5D8(m);
    fn_802AE42C(m);
    fn_802AE5A0(m);
    fn_802B865C(m);
    fn_802B8610(m);
    fn_802B8A90(m);
    fn_80148394(a);
    if (kind >= 1 && kind <= 3) {
        ((Unk8001C314*)fn_8001C394())->fn_8001C89C(2);
    } else if (kind == 5) {
        DrawLoadingProgress(lbl_8061FEDC, 3, 0.095238097f);
        DrawLoadingProgress(lbl_8061FEDC, 5, 0.088495575f);
        ((Unk8001C314*)fn_8001C394())->fn_8001C89C(1);
    } else if (kind == 6) {
        ((Unk8001C314*)fn_8001C394())->fn_8001C89C(5);
    } else if (kind == 7) {
        ((Unk8001C314*)fn_8001C394())->fn_8001C89C(7);
    } else if (kind == 0 || kind == 10) {
        if (b->mC->m28 == 0x37)
            ((Unk8001C314*)fn_8001C394())->fn_8001C89C(6);
        else
            ((Unk8001C314*)fn_8001C394())->fn_8001C89C(3);
    }
    fn_80317B30();
    fn_80317AB0(1);
    fn_8031618C(lbl_8061FEDC);
    if (lbl_8061EE14 != 0)
        lbl_8061EE14 = 0;
    fn_801498BC();
}

void fn_80149F6C() {}

void fn_80149F70(int list, int kind)
{
    int total = 0;
    char pad[0x20];
    Unk80149F70_Ent e;
    e.m0 = 0;
    switch (kind) {
    case 0:
        list = fn_802B9804(list, 0x100);
    case 1:
    case 2:
    case 3:
        e.m8 = 0;
    }
    for (int i = 0; i < fn_803FC4C0(list); i++) {
        int r = fn_803FC410(list, 0, i, 0, &e);
        e.m4 = i;
        if (e.m8 != 0)
            total += fn_80401904(r);
        else
            total += e.m0;
    }
    switch (kind) {
    case 0:
        lbl_8061B938 = total / 1024;
        if (list != 0)
            fn_80377488((void*)list);
        break;
    case 1:
        lbl_8061B93C = total / 1024;
        break;
    case 3:
        lbl_8061B944 = total / 1024;
        break;
    case 2:
        lbl_8061B940 = total / 1024;
        break;
    }
}

void fn_8014A0EC()
{
    if (lbl_8061B938 + lbl_8061B93C + lbl_8061B940 + lbl_8061B944 > 0xFA0) {
        lbl_8061B8BC = 1;
        return;
    }
    lbl_8061B8BC = 0;
}

char* fn_8014A140()
{
    fn_804089B4(lbl_8061B948, lbl_805C4FEC, lbl_8061B938 + lbl_8061B93C + lbl_8061B940 + lbl_8061B944);
    if (lbl_8061B948) {
        return lbl_8061B948;
    }
    return lbl_805C5008;
}

/* near-miss: only the order of the three argument loads of the second fn_803FEBDC call differs */
void fn_8014A1BC(void* self, int a, const char* b)
{
    char buf[0x80];
    int h = 0;
    buf[0] = lbl_805C5020;
    fn_8040B320(buf + 1, 0, 0x7F);
    fn_804089B4(buf, lbl_805C5024, a);
    fn_803FEBDC(buf, 6, 0x64, &h);
    fn_803FEC9C(h, 0x64);
    fn_803FEBDC(buf, 0, 0x64, &h);
    fn_803FEC74(h, 0, b, fn_8040B6A0(b), 0x64);
    fn_803FEC9C(h, 0x64);
}

void fn_8014A290(void* self, int a, int b)
{
    fn_8018466C(a, b);
}

int fn_8014A2B8(void* self, int a, int b, const char* c)
{
    if (b < 0) {
        return -1;
    }
    return fn_803FEC74(a, b, c, fn_8040B6A0(c), 0x64);
}

void* fn_8014A310(void* self)
{
    Unk8014A310_Obj* p = (Unk8014A310_Obj*)self;
    p->m0 = -1;
    p->m8 = 0;
    p->mC = 0;
    p->m1C = 0;
    p->m20 = 0;
    p->m24 = 0;
    p->m10 = 0;
    p->m14 = 0;
    p->m18 = 0;
    return p;
}

void* fn_8014A344() { if (!lbl_8061B978) lbl_8061B978 = fn_8014A310(operator new(0x28, 0)); return lbl_8061B978; }

float fn_8014A390(Unk8014A390_Obj* p)
{
    return (float)p->m1C / (float)p->mC;
}

int fn_8014A3E8(Unk8014A3E8_Obj* p, int a, int b, int c)
{
    if (p->m10 != 0 && a % p->m10 <= 2 && b != 0) {
        int t = p->mC;
        if (t != 0 && a > p->m14 - 2 && p->m8 < t) {
            if (p->m18 != 0) {
                if (c != 0) {
                    return 1;
                }
                return 0;
            }
            return 1;
        }
    }
    return 0;
}

void fn_8014A468(Unk8014A468_Obj* self, int a, int b, int c, int d, int e)
{
    if (fn_8014A3E8((Unk8014A3E8_Obj*)self, c, d, e) != 0) {
        if (b != self->m0) {
            self->m8 = 1;
            self->m1C = a;
            self->m24 = a;
            self->m20 = a;
            self->m0 = b;
        } else {
            self->m1C += a;
            self->m8 += 1;
            if (a < self->m24)
                self->m24 = a;
            else if (a > self->m20)
                self->m20 = a;
        }
        if (self->m8 == self->mC) {
            char s1[0x40];
            char s2[0x100];
            char s3[0x40];
            char s4[0x80];
            int h;
            s1[0] = lbl_805C5020;
            fn_8040B320(s1 + 1, 0, 0x3F);
            s2[0] = lbl_805C5020;
            fn_8040B320(s2 + 1, 0, 0xFF);
            s3[0] = lbl_805C5020;
            fn_8040B320(s3 + 1, 0, 0x3F);
            h = 0;
            fn_804089B4(s1, lbl_805C5030, lbl_805C4DD4);
            fn_8014A290(self, (int)s2, self->m0);
            fn_804089B4(s3, lbl_805C5044, fn_8014A390((Unk8014A390_Obj*)self), self->m24, self->m20);
            fn_8040B448(s2, s3);
            if (fn_803FEE14(s1, 0x64) != 0) {
                if (fn_803FEBDC(s1, 0, 0x64, &h) == 0)
                    return;
            } else {
                if (fn_803FEBDC(s1, 2, 0x64, &h) == 0)
                    return;
                fn_804089B4(s4, lbl_805C5054);
                fn_803FEC74(h, 0, s4, fn_8040B6A0(s4), 0x64);
            }
            int pos = FILESYS_sizesync(h, 0x64);
            fn_803FEC74(h, pos, s2, fn_8040B6A0(s2), 0x64);
            fn_803FEC9C(h, 0x64);
            s2[0] = 0;
            fn_8014A290(self, (int)s2, self->m0);
            fn_8014A1BC(self, (int)lbl_805C506C, s2);
        }
    }
}

void* fn_8014A6D8(void* self)
{
    Unk8014A6D8_Obj* p = (Unk8014A6D8_Obj*)self;
    p->m0 = -1;
    p->m8 = 0;
    p->mC = 0;
    return p;
}

void* fn_8014A6F4() { if (!lbl_8061B97C) lbl_8061B97C = fn_8014A6D8(operator new(0x10, 0)); return lbl_8061B97C; }

int fn_8014A740(Unk8014A740_Obj* p, int a, int b)
{
    int t = p->m8;
    if (t != 0 && b != 0 && p->mC == 0 && t - a <= 0) {
        p->mC = 1;
        return 1;
    }
    return 0;
}

void fn_8014A784(void* self, int a, int b, int c, int d, int e)
{
    char s1[0x100] = "";
    int h;
    *(int*)self = a;
    fn_8014A290(self, (int)s1, a);
    char s2[0x100] = "";
    fn_804089B4(s2, lbl_805C507C, lbl_805C4DD4);
    char s3[0x100] = "";
    if (fn_803FEE14(s2, 0x64) == 0) {
        if (fn_803FEBDC(s2, 6, 0x64, &h) == 0)
            return;
        fn_804089B4(s3, lbl_805C508C, s1, b, c, d, e);
    } else {
        if (fn_803FEBDC(s2, 0, 0x64, &h) == 0)
            return;
        fn_804089B4(s3, lbl_805C50F0, s1, b, c, d, e);
    }
    fn_8014A2B8(self, h, FILESYS_sizesync(h, 0x64), s3);
    fn_803FEC9C(h, 0x64);
    fn_8014A1BC(self, (int)lbl_805C5100, s1);
}

void* fn_8014A928(void* self)
{
    Unk8014A928_Obj* p = (Unk8014A928_Obj*)self;
    int n = 100;
    p->m1C = 0.2f;
    p->m18 = 0.4f;
    p->m30 = n;
    p->m20 = p->m1C / (float)n;
    p->m24 = p->m18 * 0.16666667f;
    p->m28 = 0.0f;
    p->m38 = 0.3f;
    p->m34 = 0.1f;
    p->m40 = p->m38;
    p->m48 = p->m38;
    p->m5C = p->m34;
    p->m3C = p->m34;
    p->m4C = p->m34;
    p->m58 = p->m50 = p->m38 + p->m1C;
    p->m44 = p->m34 + p->m18;
    p->m2C = p->m50;
    p->m54 = p->m34 + p->m18;
    p->m64 = p->m34 + p->m18;
    p->m68 = p->m60 = p->m50 - p->m1C / (float)n * 60.0f;
    return p;
}

void* fn_8014AA18() { if (!lbl_8061B980) lbl_8061B980 = fn_8014A928(operator new(0x6C, 0)); return lbl_8061B980; }

void fn_8014AA64(int a)
{
    fn_8014AAA8((Unk8014AAA8_Obj*)a);
    if (lbl_80620298 == 0) {
        fn_8014AD2C(a);
    }
}

void fn_8014AAA8(Unk8014AAA8_Obj* p)
{
    Unk8014AAA8_Tri v;
    v.a = p->p3C;
    v.b = p->p54;
    v.c = p->p4C;
    Unk8014AD2C_Col col;
    fn_8014DBA0(&col, 0, 0, 0, 100);
    fn_803141AC(lbl_8061FEDC, &v.a, &v.b, &v.c, &col);
    v.c = p->p3C;
    v.b = p->p54;
    v.a = p->p44;
    fn_8014DBA0(&col, 0, 0, 0, 100);
    fn_803141AC(lbl_8061FEDC, &v.c, &v.b, &v.a, &col);
    v.c = p->p4C;
    v.b = p->p3C;
    fn_8014DBA0(&col, 255, 0, 0, 255);
    fn_80314034(lbl_8061FEDC, &v.c, &v.b, &col);
    v.c = p->p4C;
    v.b = p->p54;
    fn_8014DBA0(&col, 255, 0, 0, 255);
    fn_80314034(lbl_8061FEDC, &v.c, &v.b, &col);
    v.c = p->p5C;
    v.b = p->p64;
    fn_8014DBA0(&col, 0, 255, 0, 255);
    fn_80314034(lbl_8061FEDC, &v.c, &v.b, &col);
    float t = p->m18 * 0.5f + p->m34;
    Unk8014AD2C_V2 lo = { 0.0f, 0.0f };
    lo.x = t - 0.01f;
    lo.y = p->m2C;
    Unk8014AD2C_V2 hi = { 0.0f, 0.0f };
    hi.x = t + 0.01f;
    hi.y = p->m2C;
    v.b = hi;
    v.c = lo;
    fn_8014DBA0(&col, 255, 0, 255, 255);
    fn_80314034(lbl_8061FEDC, &v.c, &v.b, &col);
}

void fn_8014AD2C(int self)
{
    Unk8014A928_Obj* p = (Unk8014A928_Obj*)self;
    Unk8014AD2C_Rect r;
    r.x1 = p->m34 + p->m24;
    r.y0 = p->m28;
    r.y1 = 0.0f;
    r.x0 = p->m34;
    int i = 5;
    do {
        float t = lbl_8068B4CC[i];
        r.y1 = p->m50 - t * (p->m1C / (float)p->m30);
        if (!(r.y1 >= p->m2C))
            p->m2C = r.y1;
        Unk8014AD2C_Pair pr;
        Unk8014AD2C_Col col;
        Unk8014AD2C_V2 lo = { 0.0f, 0.0f };
        Unk8014AD2C_V2 hi = { 0.0f, 0.0f };
        lo.x = r.x0;
        lo.y = r.y0;
        hi.x = r.x1;
        hi.y = r.y1;
        if (!(t >= 60.0f)) {
            pr.a = lo;
            pr.b = hi;
            fn_8014DBA0(&col, 255, 0, 0, 255);
            fn_80314034(lbl_8061FEDC, &pr.a, &pr.b, &col);
        } else {
            pr.b = lo;
            pr.a = hi;
            fn_8014DBA0(&col, 0, 0, 255, 255);
            fn_80314034(lbl_8061FEDC, &pr.b, &pr.a, &col);
        }
        r.x0 = r.x1;
        r.x1 = r.x1 + p->m24;
        r.y0 = r.y1;
    } while (--i >= 0);
    p->m28 = r.y1;
}

void fn_8014AF14() { lbl_80687CE4 = lbl_806557A8; ((Unk8037B3E8*)&lbl_80687CE4)->Unk8037B3E8::~Unk8037B3E8(); }

void fn_8014AF4C()
{
    lbl_80687E6C = lbl_8065BDF8;
    fn_8033CD28((char*)&lbl_80687E6C + 0xBC, 2);
    fn_8033CD28((char*)&lbl_80687E6C + 0xB0, 2);
    ((Unk8037B3E8*)&lbl_80687E6C)->Unk8037B3E8::~Unk8037B3E8();
}

void fn_8014AFA8() { lbl_80687F44 = lbl_8065F878; ((Unk8037B3E8*)&lbl_80687F44)->Unk8037B3E8::~Unk8037B3E8(); }

void fn_8014AFE0() { lbl_80687F58 = lbl_8065F8D8; ((Unk8037B3E8*)&lbl_80687F58)->Unk8037B3E8::~Unk8037B3E8(); }

void fn_8014B018() { fn_8018102C(lbl_80687F90, 0x2); }

void fn_8014B044()
{
    lbl_80688020.Unk8001FE34::~Unk8001FE34();
}

void fn_8014B070() { fn_801421D4(lbl_806881F8, 0x2); }

void fn_8014B09C() { fn_8017F384(lbl_806882F8, 0x2); }

void fn_8014B0C8() { fn_8035F6C0(lbl_8068836C, 0x2); }

void fn_8014B0F4() { fn_80358484(lbl_80688390, 0x2); }

void fn_8014B120() { fn_8035F6E0(lbl_80688398, 0x2); }

void fn_8014B14C() { fn_8035F700(lbl_806883F8, 0x2); }

void fn_8014B178() { fn_80151910(lbl_80689558, 0x2); }

void fn_8014B1A4()
{
    lbl_806896BC.Unk8002118C::~Unk8002118C();
}

void fn_8014B1D0() { fn_80311BEC(lbl_8068977C, 0x2); }

void fn_8014B1FC() { fn_8003A9E4(lbl_80689968, 0x2); }

void fn_8014B228() { fn_8035F61C(lbl_80689B68, 0x2); }

void fn_8014B254() { fn_8035B16C(lbl_80689BD0, 0x2); }

void fn_8014B280() { fn_8035A5A4(lbl_80689DB4, 0x2); }

void fn_8014B2AC() { fn_8035D9F8(lbl_80689DD4, 0x2); }

void fn_8014B2D8() { fn_8035A344(lbl_80689E50, 0x2); }

void fn_8014B304() { fn_8035A324(lbl_80689F08, 0x2); }

void fn_8014B330() { fn_8035A304(lbl_80689F94, 0x2); }

void fn_8014B35C() { fn_801E5EBC(lbl_80689FA8, 0x2); }

void fn_8014B388() { fn_801E6330(lbl_80689FF8, 0x2); }

void fn_8014B3B4() { fn_801E6870(lbl_8068A060, 0x2); }

void fn_8014B3E0() { fn_8035F67C(lbl_8068A0BC, 0x2); }

void fn_8014B40C() { fn_8035F63C(lbl_8068A0C4, 0x2); }

void fn_8014B438() { fn_8035F65C(lbl_8068A0CC, 0x2); }

void fn_8014B464() { fn_80359604(lbl_8068A0D4, 0x2); }

void fn_8014B490() { fn_8030BFB0(lbl_8068A0F4, 0x2); }

void fn_8014B4BC() { fn_8030C728(lbl_8068A1A0, 0x2); }

void fn_8014B4E8() { fn_80311D08(lbl_8068A380, 0x2); }

/* near-miss (aligned diff 3): retail stores lbl_8061E978 through 'mr r0,r3'; the address-caching
   pointer locals below reproduce the callee-saved lis registers, which plain globals do not */
/* Constructs the global managers. fn_802B85D8 takes the new manager (r3 stays live across the store); the
   address-caching pointer locals below reproduce the callee-saved lis registers, which plain globals do not */
void fn_8014BB70()
{
    void** a = &lbl_8068B618;
    void** b = &lbl_8061E978;
    void** c = &lbl_8068DA4C;
    void** d = &lbl_8062013C;
    void** e = &lbl_80620140;
    fn_802DC9C0();
    fn_80071F00();
    fn_800906B8();
    fn_8007295C(lbl_8061930C);
    fn_800766B8(lbl_8061930C, 0);
    fn_80070E38();
    fn_800A152C();
    *a = fn_80148280(operator new(0x38, 0));
    void* m = fn_802AD720(operator new(0x18720, 0));
    *b = m;
    fn_802B85D8((Unk802ACD88_Mgr*)m);
    fn_80313BCC(lbl_8061FEDC);
    *c = fn_802BB4FC(operator new(8, 0));
    fn_800B8108();
    fn_8001BC10();
    fn_800AA108();
    fn_805210A0();
    *d = fn_8031C62C(operator new(0x90, 0));
    *e = fn_8031CD88(operator new(0x28, 0));
    fn_80322450();
}

void fn_8014BC64()
{
    fn_802B8698((Unk802ACD88_Mgr*)lbl_8061E978);
    fn_802B832C((Unk802ACD88_Mgr*)lbl_8061E978);
    if (lbl_8061E978 != 0) {
        ((Unk8014BC64_Va*)lbl_8061E978)->fn_vslot19(3);
        lbl_8061E978 = 0;
    }
    if (lbl_8068F1F8 != 0) {
        ((Unk8014BC64_Vb*)lbl_8068F1F8)->fn_vslot1(3);
        lbl_8068F1F8 = 0;
    }
    if (lbl_8068DA4C != 0) {
        fn_802BB518((Unk802BB4FC*)lbl_8068DA4C, 3);
        lbl_8068DA4C = 0;
    }
    if (lbl_80618FBC != 0)
        delete (Unk800642F0*)lbl_80618FBC;
    lbl_80618FBC = 0;
    fn_800B8150();
    fn_8001BC58();
    fn_800AA144();
    fn_805210EC();
    fn_803224C8();
    fn_802B1D04();
    if (lbl_8062013C != 0) {
        ((Unk8014BC64_Vc*)lbl_8062013C)->fn_vslot1(3);
        lbl_8062013C = 0;
    }
    if (lbl_80620140 != 0) {
        ((Unk8014BC64_Vc*)lbl_80620140)->fn_vslot1(3);
        lbl_80620140 = 0;
    }
    fn_80312544();
    if (lbl_80619948 != 0)
        ((Unk8014BC64_Vc*)lbl_80619948)->fn_vslot1(3);
    lbl_80619948 = 0;
    fn_80070E80();
    fn_80090700();
    fn_80071F48();
    if (lbl_8068B618 != 0) {
        ((Unk8014BC64_Va*)lbl_8068B618)->fn_vslot2(3);
        lbl_8068B618 = 0;
    }
}

void fn_8014BE28()
{
    lbl_806EF9C0 = operator new[](0x181BE0, 0);
    int h = fn_803ED9D4();
    lbl_806EF9B4 = h;
    MEMCLASS_create(h, lbl_805C5260, lbl_806EF9C0, 0x181BE0, 4, 4, 0, 0, 0, 0);
}

}
