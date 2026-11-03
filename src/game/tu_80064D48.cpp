/*
 * Part of the game TU 0x8000F598-0x8006902C (2,110 functions; it ends with
 * the static-initializer wrapper at 0x80069000, so it is one large, likely
 * unity-built TU). Original file boundary unknown; placeholder.
 *
 * .text 0x80064D48-0x80067A20 only: every data object is extern by its lbl_ name.
 */
#include "game/tu_80064D48.h"
#include "game/tu_80060F94.h"

typedef Unk8005DF2C_Ctx Ctx;
typedef Unk800642F0_Rec Rec;

struct Unk80064D48_T16 {
    int v[16];
};

extern Unk80060F94_G0* lbl_806192E0;
extern Unk80064D48_T16 lbl_805A928C;
extern int lbl_805B1634[];
extern const char lbl_805AE3A0[];
extern int lbl_80617808;
extern int lbl_80618F90;
extern int lbl_80618FA4;
extern int lbl_80618FA8;
extern int lbl_80618FAC;
extern int lbl_80618FB0;
extern int lbl_80618FB4;
extern int lbl_80618FB8;
extern int lbl_80687608;
extern int lbl_8068760C;
extern char lbl_80649780[];
extern char lbl_805A4984[];
extern char lbl_805A50CC[];
extern char lbl_805A6030[];
extern char lbl_806872AC[];
extern char lbl_806872B4[];
extern char lbl_806872BC[];
extern char lbl_806872C4[];
extern char lbl_806872CC[];
extern char lbl_806872D4[];
extern char lbl_806872DC[];
extern char lbl_806872E4[];

extern "C" int fn_803FED48(int h, int a, int b, int* out);
extern "C" int fn_803FEDB8(int h, int a);
extern "C" void MEM_free(int h);
int fn_804089B4(char* buf, const char* fmt, ...);
extern "C" void fn_803917D0(int v);
extern "C" void fn_803EE7BC();
int fn_8040ABA0();
extern "C" int fn_8001FA90(Ctx* c);
extern "C" int fn_8001FAC4(Ctx* c);
extern "C" int fn_8001FAF8(Ctx* c);
extern "C" int fn_8001FB2C(Ctx* c);
extern "C" int fn_8001FB60(Ctx* c);
extern "C" int fn_8001FB94(Ctx* c);
extern "C" int fn_8001FBC4(Ctx* c);
extern "C" int fn_8001FBF4(Ctx* c);
extern "C" int fn_8001FC28(Ctx* c);
extern "C" void fn_803BA7C4(char* p, int* v);
extern "C" int fn_8001D280(Ctx* c);
int fn_8001D62C(Ctx* c);
int fn_8001DA7C(Ctx* c);
int fn_8001DB70(Ctx* c);
extern "C" int fn_8001F2A0(Ctx* c);
extern "C" int fn_8001F33C(Ctx* c);
extern "C" int fn_8001F3B8(Ctx* c, Unk8005DF2C_O3B0* t);
extern "C" int fn_8001FA20(Ctx* c);
extern "C" int fn_8001FA58(Ctx* c);
float fn_80253F54(Unk8005DF2C_Stat* s, int a, int b, int c);
void fn_8001475C(int, int, int, int, int);
int fn_80016CD0(int, int, int, int);
void fn_80016D44(int, int, int, int, int);
void fn_800190A4(int, int);
void fn_80019108(int, int);
void* fn_8006771C();
void* fn_800677A8();
void* fn_800677E0();

void Unk800642F0::fn_80064D48()
{
    int h = fn_80064830(lbl_80618FB8);
    if (h) {
        char buf[16];
        int f;
        if (fn_803FED48(lbl_80618FA4, 0, 0, &f)) {
            Unk8005DF2C_O6F8* g = lbl_806192E0->m4;
            int t = g->mC->m28;
            int v;
            if (t == 0x37) {
                Unk80064D48_T16 tbl = lbl_805A928C;
                unsigned int idx = g->mC->m78 - 1;
                if (idx <= 15)
                    v = tbl.v[idx];
                else
                    v = 0;
            } else
                v = lbl_805B1634[t];
            m28 = 0;
            fn_804089B4(buf, lbl_805AE3A0, v);
            if (fn_8006473C(buf, h, m2C + m28))
                m28 = m28 + 1;
            buf[2] = 0x62;
            if (fn_8006473C(buf, h, m2C + m28))
                m28 = m28 + 1;
            fn_803FEDB8(f, 0);
        }
        MEM_free(h);
    }
}

void Unk800642F0::fn_80064F0C(Rec* r)
{
    if (r->m0 != -1) {
        fn_803917D0(r->m0);
        lbl_80617808 -= 1;
        r->m0 = -1;
    }
    r->m4 = 0;
    if (r->m8) {
        MEM_free(r->m8);
        r->m8 = 0;
    }
}

void Unk800642F0::fn_80064F78()
{
    int i = 0;
    fn_803EE7BC();
    for (; i < m18; i++)
        fn_80064F0C(m1C + i);
    for (i = 0; i < m8; i++)
        fn_80064F0C(mC + i);
    for (i = 0; i < m10; i++)
        fn_80064F0C(m14 + i);
    for (i = 0; i < m20; i++)
        fn_80064F0C(m24 + i);
    for (i = 0; i < m28; i++)
        fn_80064F0C(m2C + i);
    fn_803EE7BC();
    m38 = 0;
}

int Unk800642F0::fn_800650C8(int key, int* out1, int* out2)
{
    int r = -1;
    int i;
    if (lbl_80687608) {
        for (i = 0; i < m8; i++) {
            if (mC[i].m0 == key) {
                r = 0;
                *out1 = mC[i].m4;
                *out2 = lbl_80618FB0;
                break;
            }
        }
        if (r == -1) {
            for (i = 0; i < m10; i++) {
                if (m14[i].m0 == key) {
                    r = 0;
                    *out1 = m14[i].m4;
                    *out2 = lbl_80618FB4;
                    break;
                }
            }
            if (r == -1) {
                for (i = 0; i < m28; i++) {
                    if (m2C[i].m0 == key) {
                        r = 0;
                        *out1 = m2C[i].m4;
                        *out2 = lbl_80618FB8;
                        break;
                    }
                }
            }
        }
    }
    if (lbl_8068760C && r == -1) {
        for (i = 0; i < m18; i++) {
            if (m1C[i].m0 == key) {
                r = 1;
                *out1 = m1C[i].m4;
                *out2 = lbl_80618FA8;
                break;
            }
        }
        if (r == -1) {
            for (i = 0; i < m20; i++) {
                if (m24[i].m0 == key) {
                    r = 1;
                    *out1 = m24[i].m4;
                    *out2 = lbl_80618FAC;
                    break;
                }
            }
        }
    }
    return r;
}

Unk800652F4::Unk800652F4()
{
    m7E0 = lbl_80649780;
    m0 = 0x32;
    fn_80065338();
}

void Unk800652F4::fn_80065338()
{
    for (int i = 0; i < m0; i++) {
        m10[i].id = -1;
        for (int j = 0; j < 9; j++)
            m10[i].f[j] = 0;
    }
    m4 = -1;
    m8 = -1;
    mC = -1;
}

void Unk800652F4::fn_800653A4()
{
    if (m4 != -1 && m8 != -1) {
        m10[m4].f[m8] = 1;
        if (mC != -1)
            m10[m4].f[mC] = 1;
    }
}

/* near-miss: two stores. Retail stores the constant 1 (m8) from the saved
 * `ok` register (r22); here CSE picks the register that was just compared with
 * 1 (p, r25), and its death moves that store ahead of m4. The argument moves of
 * fn_80016CD0 match once it is declared int. Tried: m8 = ok/p/1 in both orders,
 * p == ok, ok = 1 inside the branch (becomes mr r22,r25), store after the call. */
void Unk800652F4::fn_800653F8(Ctx* c)
{
    int i = 0;
    int t = fn_8001DB70(c);
    for (i = 0; i < m0; i++) {
        if (m10[i].id == -1 || m10[i].id == t)
            break;
    }
    int lvl = fn_8001D62C(c);
    m4 = -1;
    mC = -1;
    m8 = -1;
    if (i < m0) {
        int ok = 1;
        int picked = 0;
        m10[i].id = t;
        int p = fn_800656FC(i, c);
        int q = (int)fn_80253F54(&c->m3B0->m588, 1, 2, 0);
        if (p == 1 && q > 1) {
            picked = 1;
            fn_80016CD0(fn_8001FA58(c), fn_8001FA20(c), fn_8001DB70(c), fn_8001F33C(c));
            m8 = ok;
            m4 = i;
            p = fn_800656FC(i, c);
        } else if (p != -1 && lvl > 1 && fn_8040ABA0() % 100 > 0x3c) {
            int out;
            int kind = fn_800657C8(p, &out, c);
            int v = 0;
            if (kind == 1)
                v = out;
            picked = 1;
            fn_80016D44(out, kind, fn_8001DB70(c), fn_8001F33C(c), v);
            m4 = i;
            m8 = p;
        } else {
            ok = 0;
            fn_8001475C(fn_8001F3B8(c, c->m3B0), fn_8001DB70(c), fn_8001D280(c), fn_8001F2A0(c), fn_8001DA7C(c));
        }
        if (ok && !picked) {
            int lv2 = fn_8001FC28(c);
            if (p == 1 && lv2 > 0) {
                if (fn_8040ABA0() % 100 > 0x32) {
                    fn_80019108(fn_8001DB70(c), lv2);
                } else {
                    fn_800190A4(fn_8001FA58(c), fn_8001FA20(c));
                }
                m4 = i;
                mC = p;
                fn_8040ABA0();
            }
        }
    }
}

int Unk800652F4::fn_800656FC(int idx, Ctx* c)
{
    int cnt = 0;
    int pick = -1;
    int sel[9];
    int v;
    for (int i = 0; i <= 8; i++) {
        if (m10[idx].f[i] == 0 && fn_800657C8(i, &v, c) != 0 && v > 0) {
            sel[cnt] = i;
            cnt++;
        }
    }
    if (cnt > 0)
        pick = sel[fn_8040ABA0() % cnt];
    return pick;
}

int Unk800652F4::fn_800657C8(int k, int* out, Ctx* c)
{
    int r = 0;
    switch (k) {
    case 0:
        r = 2;
        *out = fn_8001FB94(c);
        break;
    case 1: {
        int v = fn_8001FC28(c);
        if (v != fn_8001FA90(c)) {
            *out = v;
            r = 1;
        } else
            *out = -1;
        break;
    }
    case 2:
        r = 0x10000;
        *out = fn_8001FAF8(c);
        break;
    case 3:
        r = 4;
        *out = fn_8001FB2C(c);
        break;
    case 4:
        r = 8;
        *out = fn_8001FB60(c);
        break;
    case 5: {
        int v = fn_8001FBC4(c);
        int w = fn_8001FA90(c);
        if (v != w) {
            *out = w;
            r = 0x20;
        } else
            *out = -1;
        break;
    }
    case 6:
        r = 0x20;
        *out = fn_8001FA90(c);
        break;
    case 7: {
        int v = fn_8001FAC4(c);
        if (v != fn_8001FA90(c)) {
            *out = v;
            r = 0x40;
        } else
            *out = -1;
        break;
    }
    case 8:
        r = 0x80;
        *out = fn_8001FBF4(c);
        break;
    case 9:
    case -1:
        *out = -1;
        break;
    }
    return r;
}

void fn_800676F0()
{
    fn_8006771C();
}

int fn_80067710()
{
    return 0x871AE7A9;
}

void* fn_8006771C()
{
    return lbl_805A4984;
}

void fn_80067728()
{
}

void fn_8006772C(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7fff)
        v = 0x7fff;
    fn_803BA7C4(lbl_806872AC, &v);
}

void fn_8006777C()
{
    fn_800677A8();
}

int fn_8006779C()
{
    return 0x9A0B3B52;
}

void* fn_800677A8()
{
    return lbl_805A50CC;
}

void fn_800677B4()
{
    fn_800677E0();
}

int fn_800677D4()
{
    return 0xAB6EE03F;
}

void* fn_800677E0()
{
    return lbl_805A6030;
}

void fn_800677EC()
{
}

void fn_800677F0(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7fff)
        v = 0x7fff;
    fn_803BA7C4(lbl_806872B4, &v);
}

void fn_80067840(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7fff)
        v = 0x7fff;
    fn_803BA7C4(lbl_806872BC, &v);
}

void fn_80067890(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7fff)
        v = 0x7fff;
    fn_803BA7C4(lbl_806872C4, &v);
}

void fn_800678E0(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7fff)
        v = 0x7fff;
    fn_803BA7C4(lbl_806872CC, &v);
}

void fn_80067930(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7fff)
        v = 0x7fff;
    fn_803BA7C4(lbl_806872D4, &v);
}

void fn_80067980(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7fff)
        v = 0x7fff;
    fn_803BA7C4(lbl_806872DC, &v);
}

void fn_800679D0(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7fff)
        v = 0x7fff;
    fn_803BA7C4(lbl_806872E4, &v);
}
