/*
 * Game code, .text 0x800D4D0C-0x800D833C (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_800D4D0C.h"
struct Unk800CFEEC_Raw {
    char pad0[0x28];
    Unk800CFEEC_Raw* fn_800CFEEC(int flag);
};
extern char lbl_8065A0A8[];
extern char lbl_8065A2B0[];
extern "C" void fn_800D4B9C(void* self, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int s0, int s1, int s2, int s3, int s4,
                            int s5, int s6, int s7, int s8, int s9, int s10, int s11, int s12, int s13, int s14, int s15, int s16);
extern "C" int strcmp(const char* a, const char* b);
extern "C" int sprintf(char*, const char*, ...);
extern const char lbl_805BCA08[];
extern "C" char* fn_800DF254(char* dst, int a, int b, float f);
extern "C" void fn_800D4000(Unk800D4D0C_D* t, char* s);
extern "C" void fn_800D4208(Unk800D4D0C_D* t, const char* s);
/* "ml" tweaker object registered by fn_800D69E8 (same object as Unk800D697C_Obj):
 * ints at +4..+0x54 (range-checked), floats at +0x58..+0xA8, flag at +0xAC,
 * two spline lists at +0xB0 and +0xBC (fn_8033CCE0 in the ctor). */
struct Unk800D69E8_Obj {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
    int m30;
    int m34;
    int m38;
    int m3C;
    int m40;
    int m44;
    int m48;
    int m4C;
    int m50;
    int m54;
    float m58;
    float m5C;
    float m60;
    float m64[6];
    float m7C[6];
    float m94;
    float m98;
    float m9C;
    float mA0;
    float mA4;
    float mA8;
    int mAC;
    char mB0[0xC];
    char mBC[0xC];
};
extern "C" {
void fn_8037B7E4(void* s, const char* name);
void fn_8037B7E8(void* s, const char* group);
void fn_8037B830(void* s, const char* group);
void fn_8037B494(void* s, const char* name, int* field, const char* range, const char* def);
void fn_8037B50C(void* s, const char* name, float* field, const char* range, const char* def);
void fn_8037B458(void* s, const char* name, int* field, const char* range, const char* def);
}
extern const char lbl_805B7090[];
extern const char lbl_805B8710[];
extern const char lbl_805BCA18[];
extern const char lbl_805BCA1C[];
extern const char lbl_805BCA30[];
extern const char lbl_805BCA3C[];
extern const char lbl_805BCA48[];
extern const char lbl_805BCA4C[];
extern const char lbl_805BCA5C[];
extern const char lbl_805BCA6C[];
extern const char lbl_805BCA80[];
extern const char lbl_805BCA90[];
extern const char lbl_805BCA94[];
extern const char lbl_805BCAA8[];
extern const char lbl_805BCAC0[];
extern const char lbl_805BCAC4[];
extern const char lbl_805BCAD4[];
extern const char lbl_805BCAE4[];
extern const char lbl_805BCAF8[];
extern const char lbl_805BCB08[];
extern const char lbl_805BCB0C[];
extern const char lbl_805BCB1C[];
extern const char lbl_805BCB20[];
extern const char lbl_805BCB30[];
extern const char lbl_805BCB34[];
extern const char lbl_805BCB4C[];
extern const char lbl_805BCB64[];
extern const char lbl_805BCB80[];
extern const char lbl_805BCB90[];
extern const char lbl_805BCB9C[];
extern const char lbl_805BCBB0[];
extern const char lbl_805BCBD0[];
extern const char lbl_805BCBE0[];
extern const char lbl_805BCBF0[];
extern const char lbl_805BCC00[];
extern const char lbl_805BCC18[];
extern const char lbl_805BCC24[];
extern const char lbl_805BCC28[];
extern const char lbl_805BCC34[];
extern const char lbl_805BCC44[];
extern const char lbl_805BCC60[];
extern const char lbl_805BCC6C[];
extern const char lbl_805BCC70[];
extern const char lbl_805BCC90[];
extern const char lbl_805BCC94[];
extern const char lbl_805BCCB0[];
extern const char lbl_805BCCB4[];
extern const char lbl_805BCCD0[];
extern const char lbl_805BCCF4[];
extern const char lbl_805BCD10[];
extern const char lbl_805BCD1C[];
extern const char lbl_805BCD28[];
extern const char lbl_805BCD38[];
extern const char lbl_805BCD44[];
extern const char lbl_805BCD54[];
extern const char lbl_805BCD78[];
extern const char lbl_805BCD88[];
extern const char lbl_805BCD9C[];
extern const char lbl_805BCDA8[];
extern const char lbl_805BCDBC[];
extern const char lbl_805BCDC8[];
extern const char lbl_805BCDD0[];
class Unk800D53F8_V {
public:
    char pad0[0x3C];
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual void s17();
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual void s23();
    virtual void s24();
    virtual void s25();
    virtual void s26();
    virtual void s27();
    virtual void s28();
    virtual void s29(int a);
};

void fn_800D4D0C(Unk800D4D0C_D* t, int v)
{
    t->fn_800C7E38(v);
    t->vslot49();
}

void fn_800D4D50(Unk800D4D0C_D* t, int v)
{
    t->fn_800C7E90(v);
    t->vslot49();
}

void fn_800D4D94(Unk800D4D0C_D* t, int v)
{
    t->fn_800C7E64(v);
    t->vslot49();
}

void fn_800D4DD8(Unk800D4D0C_D* t, int v)
{
    t->fn_800C7EBC(v);
    t->vslot49();
}

void fn_800D4E1C(Unk800D4D0C_D* t)
{
    t->vslot24(0);
    if (!(t->m24 & 0x40000000)) {
        t->vslot35(t->mBC);
    } else if (t->m24 & 0x20000000) {
        t->vslot35(t->mCC);
    } else if (t->m24 & 0x10000000) {
        t->vslot24(1);
        t->vslot35(t->m9C);
    } else if (t->m24 & 0x08000000) {
        t->vslot35(t->mAC);
    } else {
        t->vslot35(t->m8C);
    }
}

void fn_800D5288(Unk800D4D0C_D* t, int a)
{
    if (fn_800C80D4() != 0) {
        fn_800C8070(t);
        fn_802D7EE4();
    }
    fn_800C8070(t);
    fn_802D7EE4();
    fn_800D4538(t, a);
}

extern "C" Unk800D4D0C_D* fn_800D52E4(Unk800D4D0C_D* self, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int s0, int s1,
                                      int s2, int s3, int s4, int s5, int s6, int s7, int s8, int s9, int s10, int s11, int s12,
                                      int s13, int s14, int s15)
{
    fn_800D4B9C(self, p1, p2, p3, p4, p5, p6, p7, s0, s1, s2, s3, 0xBBC, s4, s10, s11, s5, s6, s7, s8, s9, 0, s13, s14, s15);
    ((Unk800CFEEC_Raw*)((char*)self + 0xDC))->fn_800CFEEC(s12);
    *(const void**)((char*)self + 0x100) = lbl_8065A2B0;
    *(const void**)((char*)self + 0x3C) = lbl_8065A0A8;
    self->m104 = -1;
    self->m318 = 0;
    self->m31C = 0.0f;
    self->m324 = 1.0f;
    self->m320 = 0.0f;
    self->m328 = 0.0f;
    *(int*)((char*)self + 0x1C) = 10;
    return self;
}

void fn_800D53F8(Unk800D4D0C_D* t)
{
    char b8[0x68];
    char b70[200];
    if (t->m318 != 0) {
        char* s = t->m32C;
        if (strcmp(s, lbl_805BCA08) == 0) {
            fn_800DF254(b70, 0, 3, t->m328);
            fn_800D4000(t, b70);
        } else {
            sprintf(b8, s, t->m328);
            fn_800D4208(t, b8);
        }
    } else if (t->bslot13() > 0) {
        ((Unk800D53F8_V*)t)->s29(t->m108[t->bslot5()]);
    }
}

int fn_800D54FC(Unk800D4D0C_D* t)
{
    if (!(t->m24 & 0x40000000)) {
        return 0;
    }
    if (t->m318 != 0) {
        float v = t->m328 + t->m324;
        if (!(v <= t->m320)) {
            if (t->bm8 < 0) {
                v = t->m31C;
            } else {
                v = t->m320;
            }
        }
        t->vslot59(v);
    } else {
        t->fn_800CFF7C();
    }
    t->vslot63();
    return 1;
}

int fn_800D55C8(Unk800D4D0C_D* t)
{
    if (!(t->m24 & 0x40000000)) {
        return 0;
    }
    if (t->m318 != 0) {
        float v = t->m328 - t->m324;
        if (!(v >= t->m31C)) {
            if (t->bm8 < 0) {
                v = t->m320;
            } else {
                v = t->m31C;
            }
        }
        t->vslot59(v);
    } else {
        t->fn_800CFFB4();
    }
    t->vslot63();
    return 1;
}

void fn_800D5694(Unk800D4D0C_D* t, int idx, int a, int b)
{
    int n = t->bslot13() - 1;
    while (n >= idx) {
        t->m108[n + 1] = t->m108[n];
        t->m210[n + 1] = t->m210[n];
        n--;
    }
    t->m108[idx] = a;
    t->m210[idx] = b;
    t->bslot14(t->bslot13() + 1);
}

void fn_800D5770(Unk800D4D0C_D* t, int a, int b)
{
    t->m108[t->bslot13()] = a;
    t->m210[t->bslot13()] = b;
    t->bslot14(t->bslot13() + 1);
    t->vslot63();
}

void fn_800D583C(Unk800D4D0C_D* t, int idx)
{
    t->bslot14(t->bslot13() - 1);
    while (idx < t->bslot13()) {
        t->m108[idx] = t->m108[idx + 1];
        t->m210[idx] = t->m210[idx + 1];
        idx++;
    }
    t->vslot63();
}

void fn_800D5910(Unk800D4D0C_D* t)
{
    int i = t->bslot7();
    if (i != -1) {
        t->bslot3(i);
    }
}

void fn_800D5978(Unk800D4D0C_D* t)
{
    t->fn_800CFF48();
    t->vslot63();
}

void fn_800D59C0(Unk800D4D0C_D* t)
{
    int i = t->bslot7();
    if (i != -1) {
        t->bslot6(i);
    }
}

void fn_800D5A28(Unk800D4D0C_D* t, int x)
{
    int i = 0;
    while (i < t->bslot13()) {
        t->bslot6(i);
        if (t->vslot54(t->bslot5()) == x) {
            t->vslot63();
            return;
        }
        i++;
    }
}

void fn_800D5B00(Unk800D4D0C_D* t, int idx)
{
    char s[0x64];
    Unicode2Ascii(s, fn_80090880(lbl_8061977C, t->m108[idx]));
    fn_80409DAC(s);
}

void fn_800D5B48(Unk800D4D0C_D* t, int idx, int v)
{
    t->m210[idx] = v;
}

int fn_800D5B58(Unk800D4D0C_D* t, int idx)
{
    return t->m210[idx];
}

void* fn_800D5B68(Unk800D4D0C_D* t, int idx)
{
    return fn_80090880(lbl_8061977C, t->m108[idx]);
}

/* near-miss: loop/check layout differs (retail places the checks for 2..4 before the loop increment, 29 slots; tried while/for/do-while and an if-else v= chain) */
int fn_800D5B9C(Unk800D4D0C_D* t)
{
    if (t->m104 == -1) {
        int c = t->vslot32();
        for (int i = 0; i <= 0x7B; i++) {
            int a = fn_800E18F8(i);
            int b = fn_800E1930(i);
            int d = fn_800E1968(i);
            int e = fn_800E18B0(i);
            int f = fn_800E197C(i);
            if (c == a) {
                t->m104 = 0;
                break;
            }
            if (c == b) {
                t->m104 = 1;
                break;
            }
            if (c == d) {
                t->m104 = 2;
                break;
            }
            if (c == e) {
                t->m104 = 3;
                break;
            }
            if (c == f) {
                t->m104 = 4;
                break;
            }
        }
    }
    return t->m104;
}

int fn_800D5C8C(Unk800D4D0C_D* t, int k, int x)
{
    switch (k) {
    case 0:
        return fn_800E18F8(x);
    case 1:
        return fn_800E1930(x);
    case 3:
        return fn_800E18B0(x);
    case 4:
        return fn_800E197C(x);
    case 2:
    default:
        return fn_800E1968(x);
    }
}

void fn_800D5D04(Unk800D4D0C_D* t, int x)
{
    int k = fn_800D5B9C(t);
    int y = fn_800B548C(x);
    int z = fn_800D5C8C(t, k, x);
    t->vslot51(z, y);
}

void fn_800D5D74(Unk800D4D0C_D* t, int x)
{
    int k = fn_800D5B9C(t);
    int y = fn_800B5520(x);
    int z = fn_800D5C8C(t, k, y);
    t->vslot51(z, x);
}

void fn_800D627C(Unk800D4D0C_D* t, float a, float b, float c)
{
    t->m320 = b;
    t->m324 = c;
    t->m31C = a;
    t->m328 = a;
    t->m318 = 1;
    fn_8040B61C(t->m32C);
    t->bslot14(t->bslot13() + (int)((b - a) / c) + 1);
    t->vslot63();
}

void fn_800D6354(Unk800D4D0C_D* t, float f)
{
    t->m328 = f;
    t->vslot63();
}

int fn_800D6394(Unk800D4D0C_D* t)
{
    return t->bm0;
}

int fn_800D639C(Unk800D4D0C_D* t, int x)
{
    int i;
    for (i = 0; i < t->bslot13(); i++) {
        if (t->m108[i] == x) {
            return i;
        }
    }
    return -1;
}

int fn_800D641C(Unk800D4D0C_D* t, int x)
{
    int i;
    for (i = 0; i < t->bslot13(); i++) {
        if (t->m210[i] == x) {
            return i;
        }
    }
    return -1;
}

UiScrRaw2* fn_800D649C(UiScrRaw2* self, int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k)
{
    self->fn_800D0D9C(a, 7, b, c, d, e, f, g, h, i, j, k);
    self->m3C = lbl_806578D8;
    return self;
}

void fn_800D6528(Unk800D4D0C_D* t)
{
    Unk800D4D0C_A* a = t->vslot31();
    Unk800D4D0C_A* b = t->vslot29();
    if (b != 0 && a != 0) {
        short s28 = b->m28;
        short s2C = b->m2C;
        short a2C = a->m2C;
        float f;
        if (t->vslot32() == 1) {
            f = 0.0f;
        } else {
            f = (float)(s2C - a2C) / (float)(t->vslot32() - 1);
        }
        int v = (int)((float)s28 + (float)t->vslot34() * f);
        a->vslot10(v);
    }
}

void fn_800D66C0()
{
    fn_800D1714();
}

void fn_800D66E0(Unk800D4D0C_D* t)
{
    fn_800D1614(t);
    if (t->vslot36() != 0) {
        if (t->vslot37() != 0) {
            if (t->vslot29() != 0) {
                fn_800D689C(t);
            }
        }
    }
}

void fn_800D6774(Unk800D4D0C_D* t)
{
    fn_800D1654(t);
    if (t->vslot36() != 0) {
        if (t->vslot37() != 0) {
            if (t->vslot29() != 0) {
                fn_800D689C(t);
            }
        }
    }
}

void fn_800D6808(Unk800D4D0C_D* t)
{
    fn_800D1694(t);
    if (t->vslot36() != 0) {
        if (t->vslot37() != 0) {
            if (t->vslot29() != 0) {
                fn_800D689C(t);
            }
        }
    }
}

void fn_800D689C(Unk800D4D0C_D* t)
{
    Unk800D4D0C_A* a = t->vslot36();
    Unk800D4D0C_A* b = t->vslot37();
    Unk800D4D0C_A* c = t->vslot29();
    if (a != 0 && b != 0 && c != 0) {
        int s = a->m28 + a->m2C;
        c->vslot10(s);
        c->vslot13(b->m28 - s - 1);
    }
}

void* fn_800D697C(Unk800D697C_Obj* p)
{
    new (p) Unk8037B3E8(0x6D896082);
    *(const void**)p = lbl_8065BDF8;
    fn_8033CCE0((char*)p + 0xB0);
    fn_8033CCE0((char*)p + 0xBC);
    p->mC8 = 1;
    p->mCC = 0;
    p->mD0 = p->m60;
    return p;
}

void fn_800D69E8(Unk800D69E8_Obj* t)
{
    char buf[0x68];
    int i;
    fn_8037B7E4(t, lbl_805BCA18);
    fn_8037B7E8(t, lbl_805BCA1C);
    fn_8037B494(t, lbl_805BCA30, &t->m4, 0, lbl_805B7090);
    fn_8037B494(t, lbl_805BCA3C, &t->m8, 0, lbl_805BCA48);
    fn_8037B494(t, lbl_805BCA4C, &t->mC, 0, lbl_805BCA48);
    fn_8037B494(t, lbl_805BCA5C, &t->m10, 0, lbl_805B7090);
    fn_8037B494(t, lbl_805BCA6C, &t->m14, 0, lbl_805B7090);
    fn_8037B494(t, lbl_805BCA80, &t->m18, 0, lbl_805BCA90);
    fn_8037B494(t, lbl_805BCA94, &t->m1C, 0, lbl_805BCA48);
    fn_8037B494(t, lbl_805BCAA8, &t->m20, 0, lbl_805BCAC0);
    fn_8037B494(t, lbl_805BCAC4, &t->m24, 0, lbl_805BCA48);
    fn_8037B494(t, lbl_805BCAD4, &t->m28, 0, lbl_805BCA90);
    fn_8037B7E8(t, lbl_805BCAE4);
    fn_8037B494(t, lbl_805BCAF8, &t->m2C, 0, lbl_805BCB08);
    fn_8037B494(t, lbl_805BCB0C, &t->m34, 0, lbl_805BCB1C);
    fn_8037B494(t, lbl_805BCB20, &t->m3C, 0, lbl_805BCB30);
    fn_8037B494(t, lbl_805BCB34, &t->m40, 0, lbl_805BCA90);
    fn_8037B494(t, lbl_805BCB4C, &t->m44, 0, lbl_805B7090);
    fn_8037B830(t, lbl_805BCAE4);
    fn_8037B7E8(t, lbl_805BCB64);
    fn_8037B494(t, lbl_805BCB80, &t->m48, 0, lbl_805BCB30);
    fn_8037B494(t, lbl_805BCB90, &t->m4C, 0, lbl_805BCA90);
    fn_8037B494(t, lbl_805BCB9C, &t->m50, 0, lbl_805B7090);
    fn_8037B830(t, lbl_805BCB64);
    fn_8037B7E8(t, lbl_805BCBB0);
    fn_8037B494(t, lbl_805BCBD0, &t->m30, 0, lbl_805BCB30);
    fn_8037B494(t, lbl_805BCBE0, &t->m38, 0, lbl_805BCA90);
    fn_8037B494(t, lbl_805BCBF0, &t->m54, 0, lbl_805B7090);
    fn_8037B830(t, lbl_805BCBB0);
    fn_8037B830(t, lbl_805BCA1C);
    fn_8037B7E8(t, lbl_805BCC00);
    fn_8037B50C(t, lbl_805BCC18, &t->m58, 0, lbl_805BCC24);
    fn_8037B50C(t, lbl_805BCC28, &t->m5C, 0, lbl_805B7090);
    fn_8037B50C(t, lbl_805BCC34, &t->m60, 0, lbl_805B8710);
    fn_8037B50C(t, lbl_805BCC44, &t->m94, lbl_805BCC60, lbl_805BCC6C);
    fn_8037B50C(t, lbl_805BCC70, &t->m98, lbl_805BCC60, lbl_805BCC90);
    fn_8037B50C(t, lbl_805BCC94, &t->m9C, lbl_805BCC60, lbl_805BCCB0);
    fn_8037B50C(t, lbl_805BCCB4, &t->mA0, lbl_805BCC60, lbl_805BCCB0);
    fn_8037B50C(t, lbl_805BCCD0, &t->mA4, lbl_805BCC60, lbl_805BCC90);
    fn_8037B50C(t, lbl_805BCCF4, &t->mA8, lbl_805BCC60, lbl_805BCC6C);
    fn_8037B7E8(t, lbl_805BCD10);
    fn_8037B458(t, lbl_805BCD1C, &t->mAC, 0, lbl_805B8710);
    fn_8037B7E8(t, lbl_805BCD28);
    ((Unk8037B3E8*)t)->fn_8037B728(lbl_805BCD38, lbl_805BCD44, t->mB0, (int)lbl_805BCD54);
    fn_8037B830(t, lbl_805BCD28);
    fn_8037B7E8(t, lbl_805BCD78);
    ((Unk8037B3E8*)t)->fn_8037B728(lbl_805BCD38, lbl_805BCD88, t->mBC, (int)lbl_805BCD54);
    fn_8037B830(t, lbl_805BCD78);
    fn_8037B830(t, lbl_805BCD10);
    fn_8037B7E8(t, lbl_805BCD9C);
    for (i = 0; i < 6; i++) {
        sprintf(buf, lbl_805BCDA8, i + 1);
        fn_8037B50C(t, buf, &t->m64[i], lbl_805BCDBC, lbl_805BCDC8);
    }
    for (i = 0; i < 6; i++) {
        sprintf(buf, lbl_805BCDD0, i + 1);
        fn_8037B50C(t, buf, &t->m7C[i], lbl_805BCDBC, lbl_805BCDC8);
    }
    fn_8037B830(t, lbl_805BCD9C);
    fn_8037B830(t, lbl_805BCA1C);
}

void fn_800D6F9C()
{
}

void fn_800D6FA0()
{
    fn_8037C7FC()->fn_8037CE98(0x6D896082, lbl_80687C5C, 0);
}

Unk800D697C* fn_800D6FD8()
{
    return new (lbl_806EF9B4) Unk800D697C;
}

float fn_800D7008(Unk800D7008_Obj* t, int a)
{
    float s = t->m60 - t->m58;
    s = s / (t->m5C - t->m58);
    if ((int)fn_8007BB28(lbl_8061930C, 2, a, 1) > 0) {
        if (!(fn_8007BB28(lbl_8061930C, 5, a, 1) <= lbl_805BCDE4)) {
            s = s + lbl_805BCDE8;
        } else {
            s = s - lbl_805BCDE8;
        }
        s = s + (float)fn_800768E0(lbl_8061930C, a) * lbl_805BCDF8;
        if (!(s >= lbl_805BCDFC)) {
            s = lbl_805BCE00;
        } else if (!(s <= lbl_805BCE04)) {
            s = lbl_805BCE08;
        }
    }
    return s;
}

Unk800D7624::Unk800D7624()
{
    new (this) Unk8037B3E8(0x6BC2DEDF);
    *(const void**)this = lbl_806557A8;
}

int fn_800D7898(int n)
{
    char buf[0x20];
    if (fn_8037C7FC() != 0) {
        fn_804089B4(buf, lbl_805BD024, n + 1);
        return fn_8037C7FC()->fn_8037CF5C(0x6BC2DEDF, buf);
    }
    return 0;
}

Unk800D7624* fn_800D7900()
{
    return new (lbl_806EF9B4) Unk800D7624;
}

int fn_800D7C10(char* t, int idx, int a)
{
    Unk800D8304_Row* row = &lbl_8061AC5C[idx];
    unsigned int f = row->m4;
    if (f & 0x80) {
        char* b = t + 8;
        return a >= *(int*)(b + idx * 0xC);
    }
    if (f & 0x200) {
        char* b = t + 0xC;
        return a >= *(int*)(b + idx * 0xC);
    }
    if (f & 0x100) {
        char* b = t + 8;
        return a >= (int)(*(float*)(b + idx * 0xC) * lbl_805BD03C);
    }
    return a > 0;
}

float fn_800D7EC8(float a, float b)
{
    if (b == lbl_805BD050) {
        b = lbl_805BD054;
    }
    float t = b * lbl_805BD058;
    if (a < t) {
        return lbl_805BD050;
    }
    float r = (a - t) / (b - t) + lbl_805BD05C;
    return r;
}

float fn_800D7F48(int a, int b)
{
    if (a >= b) {
        return lbl_805BD060;
    }
    float r = (float)a / (float)b;
    return r;
}

static inline unsigned int Flags(int idx)
{
    Unk800D8304_Row* row = &lbl_8061AC5C[idx];
    return row->m4;
}

/* near-miss: 85/85 insns, flag-test blocks and row-pointer register differ (25 slots) */
float fn_800D7FB4(char* t, int idx, float x)
{
    char* p;
    if (Flags(idx) & 0x10000) {
        p = t;
        if ((Flags(idx) & 2) || (Flags(idx) & 4)) {
            int off = idx * 0xC; p += 8;
            return fn_800D7EC8(x, *(float*)(p + off));
        }
        int off = idx * 0xC; p += 8;
        return fn_800D7F48((int)x, *(int*)(p + off));
    }
    if (Flags(idx) & 0x20000) {
        p = t;
        if ((Flags(idx) & 0x20) || (Flags(idx) & 0x40)) {
            int off = idx * 0xC; p += 0xC;
            return fn_800D7EC8(x, *(float*)(p + off));
        }
        int off = idx * 0xC; p += 0xC;
        return fn_800D7F48((int)x, *(int*)(p + off));
    }
    if (Flags(idx) & 0x40000) {
        char* q = t + 8;
        int off = idx * 0xC;
        float b = *(float*)(q + off);
        float r1;
        if (!(x <= lbl_805BD070)) {
            r1 = lbl_805BD074 / x;
        } else {
            r1 = lbl_805BD078;
        }
        float r2;
        if (!(b <= lbl_805BD070)) {
            r2 = lbl_805BD074 / b;
        } else {
            r2 = lbl_805BD078;
        }
        return fn_800D7EC8(r1, r2);
    }
    return x;
}

inline int KeyAt8108(char* base, int off)
{
    return *(int*)(base + off);
}

void fn_800D8108(int tbl, int idx, int a, char* out, float f)
{
    int off = idx * 12;
    Unk800D8304_Row* row = &lbl_8061AC5C[idx];
    unsigned fl = row->m4;
    int v0 = KeyAt8108((char*)tbl + 8, off);
    int v1 = KeyAt8108((char*)tbl + 0xC, off);
    char b1[0x28];
    char b2[0x3C];
    if (fl & 0x80000) {
        fn_804031BC(b1, lbl_805BD07C);
        fn_80403DE4(out, b1, a, v0);
        return;
    }
    if (fl & 0x100000) {
        fn_804031BC(b1, lbl_805BD07C);
        fn_80403DE4(out, b1, a, v1);
        return;
    }
    if (fl & 0x200000) {
        fn_804031BC(b1, lbl_805BD07C);
        fn_80403DE4(out, b1, (int)f, v0);
        return;
    }
    if (fl & 0x400000) {
        fn_804031BC(b1, lbl_805BD07C);
        fn_80403DE4(out, b1, (int)f, v1);
        return;
    }
    if (fl & 0x800000) {
        fn_804031BC(b1, lbl_805BC1A8);
        fn_80403DE4(out, b1, (int)f);
        return;
    }
    if (fl & 0x1000000) {
        fn_804089B4(b2, lbl_805BD034, f);
        fn_804031BC(out, b2);
        return;
    }
    if (fl & 0x2000000) {
        fn_804089B4(b2, lbl_805BD02C, f);
        if (b2[0] == 0x30)
            b2[0] = 0x20;
        fn_804031BC(out, b2);
        return;
    }
    fn_804031BC(out, lbl_805BC9DC);
}

int fn_800D8304(void* t, int idx)
{
    Unk800D8304_Row* row = &lbl_8061AC5C[idx];
    return (row->m4 >> 10) & 1;
}

int fn_800D8320(void* t, int idx)
{
    Unk800D8304_Row* row = &lbl_8061AC5C[idx];
    return (row->m4 >> 11) & 1;
}
