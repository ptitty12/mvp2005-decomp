/*
 * Part of the game TU 0x8000F598-0x8006902C (2,110 functions; it ends with
 * the static-initializer wrapper at 0x80069000, so it is one large, likely
 * unity-built TU). Original file boundary unknown; placeholder.
 *
 * .text 0x8001CACC-0x8001D7F8 only: every data object is extern by its lbl_ name.
 */
#include "game/tu_8001B190.h"
#include "game/tu_8001CACC.h"
#include "game/tu_8022FE34.h"

extern "C" void* fn_803774A8(unsigned n, int heap);
extern "C" int fn_803AC11C();
extern "C" int SNDSYS_vectortoreal();
extern "C" int fn_803AC164(int a, char* buf, int size);
extern "C" int fn_803B9D50(Unk8001BA78* a);
extern "C" int Init();
extern "C" int fn_803AC944();
extern "C" int fn_803ABEE4(int a);
extern "C" void fn_803ABDD0(int a, int b);
extern "C" void fn_803ABEDC(int a);
extern "C" void SetOutputMode(int a);

/* near-miss: r30/r31 are swapped (retail keeps `this` in r30 and the ok flag in r31); everything else is identical */
int Unk8001C314::fn_8001CACC()
{
    int ok = 1;
    if (m6C == 0) {
        m64 = new (0x400) char[0x32000];
        if (fn_803AC11C() == 0) {
            if (fn_8001CC7C() == 0 || SNDSYS_vectortoreal() < 0) {
                ok = 0;
            }
            if (ok) {
                if (fn_803AC164(0, m64, 0x32000) < 0) {
                    ok = 0;
                }
            }
            if (ok) {
                if (fn_803B9D50(fn_8001BB7C()) < 0) {
                    ok = 0;
                }
            }
            if (ok) {
                if (Init() < 0) {
                    ok = 0;
                }
            }
            if (ok) {
                if (fn_803AC944() < 0) {
                    ok = 0;
                }
            }
            if (ok) {
                if (fn_803ABEE4(0x90204) < 0) {
                    ok = 0;
                }
            }
            if (ok) {
                m6C = 1;
            }
        }
    }
    if (ok == 0) {
        if (m64) {
            delete[] m64;
        }
    }
    return m6C;
}

void Unk8001C314::fn_8001CC04(int* result, int v)
{
    if (*result == -1) {
        *result = v;
        return;
    }
    if (*result < 0) {
        if (v > 0) {
            *result = 0;
        }
    } else {
        if (v <= 0) {
            *result = 0;
        }
    }
}

int Unk8001C314::fn_8001CC40(int idx)
{
    int r = 0;
    if (m68 == 1) {
        if (m24[idx].m4 != 0) {
            r = m24[idx].m0 >= 0;
        }
    }
    return r;
}

int Unk8001C314::fn_8001CC7C()
{
    return fn_8001CC9C();
}

int Unk8001C314::fn_8001CC9C()
{
    fn_803ABDD0(0, 0x20);
    fn_803ABEDC(0);
    SetOutputMode(8);
    return 1;
}

int Unk8001C314::fn_8001CCD8()
{
    return m68;
}

extern const float lbl_805A514C;
extern const float lbl_805A5150;
extern const Unk8001CE80_Vec2 lbl_8060558C;

/* near-miss: the two `x + k` fadds have their operands swapped (f11,f13 in retail) */
void Unk8001CE80::fn_8001CCE0(Unk8001CE80* o)
{
    Unk8001CE80_Vec2 v = o->m3B8;
    m288 = lbl_8060558C;
    if (!(v.x <= lbl_805A514C)) {
        m288.x = v.x - lbl_805A514C;
    }
    if (!(v.x >= lbl_805A5150)) {
        m288.x = lbl_805A514C + v.x;
    }
    if (!(v.y <= lbl_805A514C)) {
        m288.y = v.y - lbl_805A514C;
    }
    if (!(v.y >= lbl_805A5150)) {
        m288.y = lbl_805A514C + v.y;
    }
}

int Unk8001CE80::fn_8001CD94(int a, int flag)
{
    int r = 0x1b;
    switch (a) {
    case 0:
        r = 7;
        break;
    case 1:
        r = flag ? 10 : 4;
        break;
    case 2:
        r = flag ? 4 : 10;
        break;
    case 3:
        r = 6;
        break;
    case 4:
        r = flag ? 9 : 3;
        break;
    case 5:
        r = flag ? 3 : 9;
        break;
    case 6:
        r = 5;
        break;
    case 7:
        r = flag ? 8 : 2;
        break;
    case 8:
        r = flag ? 2 : 8;
        break;
    }
    return r;
}

void Unk8001CE80::fn_8001CE80()
{
    int v = m274;
    if (v <= 8) {
        if (v < -1) {
            v = -1;
        }
    } else {
        v = 8;
    }
    m274 = v;
}

int fn_80253678(char* p);
float fn_80254474(char* p, int idx, int h);

void Unk8001CE80::fn_8001CEA8(float* b, float* a)
{
    int h = fn_80253678((char*)m130 + 0x480);
    *a = fn_80254474((char*)m3B0 + 0x588, 0, h);
    *b = fn_80254474((char*)m3B0 + 0x588, 1, h);
}

void fn_8040A208(void* arr, int n, int sz, void (*f)(void*));
extern "C" void fn_8022F2C0(void* p);

extern "C" int fn_8001CF18(Unk8001CE80* x, Unk8022FE34_Ent* arr)
{
    int i = 0;
    Unk8001D47C_Mid* w = x->m130->m44C;
    int side = *(int*)(w->m6F8->m14 + 0x1cd0) == 0;
    Unk8022FE34_Pai* pai = ((Unk8022FE34_U*)w->m700->m3B0)->fn_80234E38();
    int n = pai->fn_80230024(side);
    for (i = 0; i < n; i++) {
        pai->fn_8023003C(side, i, &arr[i], 0);
    }
    fn_8040A208(arr, n, 0x2c, fn_8022F2C0);
    return n;
}

struct Unk806192E0_B {
    char pad0[0x14];
    char* m14;
};
struct Unk806192E0 {
    int m0;
    Unk806192E0_B* m4;
};
extern Unk806192E0* lbl_806192E0;
extern const float lbl_805A5154;
extern const float lbl_805A5158;
extern "C" int fn_800968C0(char* e, int a, int b);
int fn_80089AA0(char* e, int k, int v);
float fn_8007189C(char* e, int k, int v);

int Unk8001CE80::fn_8001CFCC(int a, int b)
{
    char* e = lbl_806192E0->m4->m14 + m130->m708 * 0xe64;
    char* p = e + 0xc74;
    int v = fn_800968C0(e, b, 1);
    int r = 0x1c;
    if (fn_80089AA0(p, 6, v) == a) {
        r = 6;
    } else if (fn_80089AA0(p, 0xb, v) == a) {
        r = 0xb;
    } else if (fn_80089AA0(p, 0x10, v) == a) {
        r = 0x10;
    } else if (fn_80089AA0(p, 0x15, v) == a) {
        r = 0x15;
    }
    return r;
}

float Unk8001CE80::fn_8001D0AC(int a, int k)
{
    float res = lbl_805A5154;
    if (k != 0xf && k != 8) {
        int idx = fn_8001CFCC(k, a);
        if (idx != 0x1c) {
            char* e = lbl_806192E0->m4->m14 + m130->m708 * 0xe64;
            int v = fn_800968C0(e, a, 1);
            switch (idx) {
            case 6:
                res = fn_8007189C(e + 0xc74, 7, v);
                break;
            case 0xb:
                res = fn_8007189C(e + 0xc74, 0xc, v);
                break;
            case 0x10:
                res = fn_8007189C(e + 0xc74, 0x11, v);
                break;
            case 0x15:
                res = fn_8007189C(e + 0xc74, 0x16, v);
                break;
            }
        }
    }
    return res;
}

int Unk8001CE80::fn_8001D1A4(int a, int k, int x)
{
    int r = 1;
    if (k == 0xf || k == 8) {
        if (x > 0x5c) {
            r = 0;
        }
    } else {
        if (!(fn_8001D0AC(a, k) < lbl_805A5158)) {
            r = 0;
        }
    }
    return r;
}

int Unk8001CE80::fn_8001D204(int v)
{
    return v <= 1;
}

void fn_80043690(int a);
void fn_80045934(int a, int b);
void fn_80042E68(int a, int b);
void fn_80042EA8(int a, int b);
int fn_8040ABA0();
int fn_80044D90(int a, int b);
void fn_800450A8(int a, int b, int c, int d, int e, Unk8001D47C_Mid* f);
extern "C" void fn_80045388(int a, int b, int c, int d, int e, Unk8001D47C_Mid* f, int* g, int* h, int* i);
void fn_80043628(int a);
void fn_80046024(int a, float b);

void Unk8001CE80::fn_8001D218()
{
    fn_80043690(m154[m274].m0);
}

void Unk8001CE80::fn_8001D248()
{
    fn_80045934(m154[m274].m0, m154[m274].m4);
}

extern "C" int fn_8001FA20(Unk8001CE80* x);
extern "C" int fn_8001EDBC(Unk8001CE80* x);

extern "C" int fn_8001D280(Unk8001CE80* x)
{
    int r = 0;
    unsigned char* t = x->m130->m44C->m6F8->m14;
    int o0 = x->m3B4 * 0xE64;
    int a = Unk8001D47C_SumRow((Unk8001D47C_Row*)(t + o0));
    int z = x->m3B4 == 0;
    int o1 = z * 0xE64;
    int b = Unk8001D47C_SumRow((Unk8001D47C_Row*)(t + o1));
    int ok = 0;
    if (*(int*)&x->m3B8 != 0 && x->m0 == 1 && x->m3B4 == 1 && x->m3C == 1 && x->m4D4 == 0 && x->m104 == 0 && a == 0) {
        ok = b == 0;
    }
    if (ok) {
        r |= 2;
    }
    if (*(int*)&x->m3B8 != 0 && x->m1 == 0 && a == 0 && b == 0 && x->m104 == 0 && x->m4D4 == 0) {
        r |= 1;
    }
    if (fn_8001FA20(x) == 0x4000) {
        r |= 4;
    }
    if (fn_8001EDBC(x) == 4) {
        r |= 8;
    }
    float va;
    float vb;
    x->fn_8001CEA8(&vb, &va);
    const float th = 1.6f;
    if (!(vb + va < th)) {
        r |= 0x10;
    }
    return r;
}

void Unk8001CE80::fn_8001D42C()
{
    fn_80042E68(m3CC, m3D0);
}

void Unk8001CE80::fn_8001D454()
{
    fn_80042EA8(m3CC, m3D0);
}

int fn_8004705C(int a);
int fn_80047118(int a);
int fn_80046FD4(int a);

int Unk8001CE80::fn_8001D47C()
{
    int a = Unk8001D47C_SumRow((Unk8001D47C_Row*)m130->m44C->m6F8->m14);
    int b = Unk8001D47C_SumRow((Unk8001D47C_Row*)(m130->m44C->m6F8->m14 + 0xe64));
    int sum = a + b;
    int d = a - b;
    int r = fn_8004705C(d < 0 ? -d : d);
    return r | fn_80047118(sum);
}

int Unk8001CE80::fn_8001D538()
{
    int a = Unk8001D47C_SumRow((Unk8001D47C_Row*)m130->m44C->m6F8->m14);
    int b = Unk8001D47C_SumRow((Unk8001D47C_Row*)(m130->m44C->m6F8->m14 + 0xe64));
    int d = a - b;
    return fn_80046FD4(d < 0 ? -d : d);
}

int Unk8001CE80::fn_8001D5D8()
{
    return fn_8040ABA0() % 100 > 50 ? 2 : 1;
}

int Unk8001CE80::fn_8001D62C()
{
    int v = fn_80044D90(m130->m44C->m6F8->m10->m1C, m0);
    if (v == 1) {
        return 1;
    }
    if (v != 2) {
        return v == 4 ? 4 : 0;
    }
    return 2;
}

int Unk8001CE80::fn_8001D694()
{
    return fn_80044D90(m130->m44C->m6F8->m10->m1C, m0);
}

void Unk8001CE80::fn_8001D6CC()
{
    fn_800450A8(m44C, m458, m454, m3E0, m3E4, m130->m44C);
}

void Unk8001CE80::fn_8001D70C()
{
    fn_80045388(m44C, m450, m458, m448, m444, m130->m44C, &m468, &m474, &m480);
}

void Unk8001CE80::fn_8001D75C()
{
    fn_80043628(m45C);
}

int fn_800435E8(int a, int b);

void Unk8001CE80::fn_8001D780()
{
    fn_800435E8(m454, ((Unk8001D780_VT*)m3B0)->s24());
}

void Unk8001CE80::fn_8001D7D0()
{
    fn_80046024(m3C4, m3C8);
}
