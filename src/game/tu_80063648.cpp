/*
 * Part of the game TU 0x8000F598-0x8006902C (2,110 functions; it ends with
 * the static-initializer wrapper at 0x80069000, so it is one large, likely
 * unity-built TU). Original file boundary unknown; placeholder.
 *
 * .text 0x80063648-0x80064D48 only: every data object is extern by its lbl_ name.
 */
#include "game/tu_80063648.h"
#include "game/tu_80060F94.h"

typedef Unk8005DF2C_Ctx Ctx;
typedef Unk8005DF2C_A A;
typedef Unk80060250_O O;
typedef Unk8005F4E4_Id16 Id16;
typedef Unk8005EDCC_Ent Ent;

extern Unk80060F94_G0* lbl_806192E0;
extern int lbl_80617804;
extern int lbl_80617808;
extern int lbl_80618F94;
extern int lbl_80618F98;
extern int lbl_80618F9C;
extern int lbl_80618FA0;
extern int lbl_80618FA8;
extern int lbl_80618FAC;
extern int lbl_80618FB0;
extern int lbl_80618FB4;
extern const char lbl_805AE330[];
extern const char lbl_805AE378[];
extern const char lbl_805AE380[];
extern const char lbl_805AE388[];
extern const char lbl_805AE390[];
extern const char lbl_805AE398[];

typedef Unk800642F0_Rec Rec;
typedef Unk800642F0_Str Str;

void* operator new[](unsigned int n, int heap);

extern "C" int fn_8008CFD8(char* p, int a, unsigned char b);
void fn_8040A208(void* base, int n, int size, int (*cmp)(int*, int*));
int fn_802B96C4(Str* s, int a);
int fn_802B9C1C(Str* s, int* sz, int a);
int fn_802B9804(int a, int b);
extern "C" int SPCH_AddBank(int v);
extern "C" int fn_803FC438(int h, Str* s);
Str* fn_8040B61C(Str* s, const char* name); /* strcpy: returns s */
Str* fn_8040B448(Str* s, const char* t);    /* strcat: returns s */
extern "C" int fn_803FED48(int h, int a, int b, int* out);
extern "C" int fn_803FEDB8(int h, int a);
extern "C" void MEM_free(int h);
int fn_804089B4(char* buf, const char* fmt, ...);
extern "C" int fn_8009E1D4(char* p, int a, int b);
extern "C" void fn_803EE7BC();
void fn_8033E958(const char* fmt, ...);
extern int lbl_80686CCC[5];
extern int lbl_80686CE0[5];

Unk8005EDCC_T* fn_801A6D84(Unk8005DF2C_O44C* p, int a);
void fn_80063500(A* a, Ctx* ctx, Ent* e);
void fn_80061B38(A* a, O* out, Ctx* ctx);
void fn_80063728(A* a, O* out, Ctx* ctx);
int fn_8004FF78(Unk8005DF2C_P* p, int a, int b);
extern "C" void fn_8004CC8C(Unk8005DF2C_P* p, int a, Ctx* c, int b);
float fn_800DBA74(Unk8005DF2C_Y* y, int a, int b, int c);
int fn_800DB9A8(Unk8005DF2C_Y* y, int a, int b, int c);
int fn_800E01C8(float f);
extern "C" int fn_80097210(Unk8005DF2C_Y* y, int a);
int fn_800475A4(int v);
void fn_80016090(int a, int b);
void fn_800160F4(int a, int b, int c);
void fn_80016160(int, int, int, int, int, int);
int fn_8001D218(Ctx* c);
extern "C" int fn_8001ED80(Ctx* c);
extern "C" int fn_8001E854(Ctx* c);
void fn_80015CE0(int a, int b);
int fn_8040ABA0();
Unk8005F4E4_G* fn_800B555C();
extern "C" int fn_800B57EC();
float fn_80253E38(Unk8005DF2C_Stat* s, int a, int b);
void fn_8001A6B8(int v);
int fn_8001D42C(Ctx* c);
void fn_80019C8C(int v);
void fn_80019C30(int v);
int fn_80253F0C(Unk8005DF2C_Stat* s, int a, int b, int c);
float fn_80253F54(Unk8005DF2C_Stat* s, int a, int b, int c);
int fn_80253DB0(Unk8005DF2C_Stat* s, int a, int b);
void fn_80016ADC(int a, int b);
void fn_80016B40(int a, int b);
void fn_80016BA4(int a, int b);
void fn_80016C08(int a, int b);
void fn_80016C6C(int a, int b);
void fn_80016A68(int a, int b, int c, int d);

void fn_80063648(A* a, O* out, Ctx* ctx)
{
    Unk8005EDCC_T* t = fn_801A6D84(ctx->m130->m44C, 1);
    Ent* p = &t->m3D48[2];
    int ok = 1;
    if (p->m0 == 0xc7)
        ok = 0;
    if (ok) {
        Ent e = *p;
        fn_80063500(a, ctx, &e);
    }
}

void fn_800636E4(A* a, O* out, Ctx* ctx)
{
    fn_80061B38(a, out, ctx);
    fn_80063728(a, out, ctx);
}

void fn_80063728(A* a, O* out, Ctx* ctx)
{
    Ent* e = &fn_801A6D84(ctx->m130->m44C, 1)->m3D48[0];
    int i = fn_8004FF78(a->m0, a->m0->m9D8, 0x20);
    while (i != -1 && a->m0->m4[i].m0->m458 != e->m8)
        i = fn_8004FF78(a->m0, i, 0x20);
    fn_8004CC8C(a->m0, 0x20, ctx, i);
}

int fn_800637F0(int v)
{
    int r = 0;
    switch (v) {
    case 0:
        r = 0x20002;
        break;
    case 1:
        r = 0x80;
        break;
    case 2:
        r = 0x201;
        break;
    case 3:
        r = 0x10;
        break;
    case 4:
        r = 0x208;
        break;
    case 5:
        r = 0x240;
        break;
    case 6:
        r = 0x204;
        break;
    case 7:
        r = 0x220;
        break;
    case 8:
        r = 0x40002;
        break;
    case 9:
        r = 0x400;
        break;
    case 10:
        r = 0x1000;
        break;
    case 11:
        r = 0x300;
        break;
    case 12:
        r = 0x10000;
        break;
    case 13:
        r = 0x8000;
        break;
    case 14:
        r = 0xa00;
        break;
    }
    return r;
}

int fn_800638F0(Unk8005DF2C_O6F8* q, Unk8005DF2C_Y* y, int k, int c, int** o1, int** o2)
{
    float f;
    for (int i = 0; i < 5; i++) {
        lbl_80686CCC[i] = 0;
        lbl_80686CE0[i] = -1;
    }
    int n = 0;
    f = fn_800DBA74(y, 0, k, c);
    lbl_80686CCC[n] = 0;
    lbl_80686CE0[n] = fn_800E01C8(f);
    n = 1;
    int v = fn_800DB9A8(y, 6, k, c);
    if (v) {
        f = fn_800DBA74(y, 1, k, c);
        lbl_80686CCC[n] = v;
        lbl_80686CE0[n] = fn_800E01C8(f);
        n++;
    }
    v = fn_800DB9A8(y, 0xb, k, c);
    if (v) {
        f = fn_800DBA74(y, 2, k, c);
        lbl_80686CCC[n] = v;
        lbl_80686CE0[n] = fn_800E01C8(f);
        n++;
    }
    v = fn_800DB9A8(y, 0x10, k, c);
    if (v) {
        f = fn_800DBA74(y, 3, k, c);
        lbl_80686CCC[n] = v;
        lbl_80686CE0[n] = fn_800E01C8(f);
        n++;
    }
    v = fn_800DB9A8(y, 0x15, k, c);
    if (v) {
        f = fn_800DBA74(y, 4, k, c);
        lbl_80686CCC[n] = v;
        lbl_80686CE0[n] = fn_800E01C8(f);
        n++;
    }
    *o1 = lbl_80686CCC;
    *o2 = lbl_80686CE0;
    return n;
}

void fn_80063AD0(A* a, O* out, Ctx* ctx)
{
    out->m4 = 0x111;
    int i = 0;
    int off = 0;
    Unk8005DF2C_O6F8* q = ctx->m130->m44C->m6F8;
    Unk8005DF2C_Y* y = q->m14;
    int k = ctx->m130->m708;
    int* p1;
    int* p2;
    int n = fn_800638F0(q, y, k, fn_80097210(y + k, 0), &p1, &p2);
    int want = fn_8001D218(ctx);
    for (; i < n; i++, off += 4) {
        if (want == fn_800637F0(p1[i])) {
            fn_80016090(fn_800637F0(p1[i]), fn_800475A4(p2[i]));
            break;
        }
    }
}

void fn_80063BAC(A* a, O* out, Ctx* ctx)
{
    out->m4 = 0x109;
    int best = -1;
    Unk8005DF2C_O6F8* q = ctx->m130->m44C->m6F8;
    Unk8005DF2C_Y* y = q->m14;
    int k = ctx->m130->m708;
    int* p1;
    int* p2;
    int n = fn_800638F0(q, y, k, fn_80097210(y + k, 0), &p1, &p2);
    int i = 0;
    int mx = -1;
    for (i = 0; i < n; i++) {
        if (p2[i] > mx) {
            mx = p2[i];
            best = i;
        }
    }
    if (best != -1)
        fn_800160F4(fn_8001ED80(ctx), fn_800637F0(p1[best]), 1);
}

inline int Pick(int base, int m, int n)
{
    return m > 0 ? (base + 1 + fn_8040ABA0() % m) % n : (base + 1) % n;
}

/* near-miss: bodies and the Pick() sequences are the same; retail's loop keeps
 * i and a byte offset in two registers, puts the increments before the loop
 * test, and reuses one saved compare (mfcr/mtcrf) for the m > 0 tests; none of
 * seven loop shapes reproduced that. */
void fn_80063C90(A* a, O* out, Ctx* ctx)
{
    int i = 0;
    out->m4 = 0x110;
    int off = 0;
    Unk8005DF2C_O6F8* q = ctx->m130->m44C->m6F8;
    Unk8005DF2C_Y* y = q->m14;
    int k = ctx->m130->m708;
    int* p1;
    int* p2;
    int n = fn_800638F0(q, y, k, fn_80097210(y + k, 0), &p1, &p2);
    for (; i < n; i++, off += 4) {
        if (fn_800637F0(p1[i]) & 0x20000) {
            if (n > 1) {
                int m = n - 1;
                int sel[3] = { -1, -1, -1 };
                sel[0] = i;
                sel[1] = Pick(sel[0], m, n);
                if (n > 2) {
                    do {
                        sel[2] = Pick(sel[0], m, n);
                    } while (sel[2] == sel[1]);
                }
                fn_80016160(fn_800637F0(p1[sel[0]]), fn_800475A4(p2[sel[0]]), fn_800637F0(p1[sel[1]]),
                            fn_800475A4(p2[sel[1]]), fn_800637F0(p1[sel[2]]), fn_800475A4(p2[sel[2]]));
            }
            break;
        }
    }
}

void fn_80063EA0(A* a, O* out, Ctx* ctx)
{
    out->m4 = 0xff;
    int f = fn_8001E854(ctx);
    if (ctx->m290[ctx->m130->m708] == 0 && (f & 1))
        f &= ~1;
    fn_80015CE0(fn_8001ED80(ctx), f);
}

void fn_80063F14(A* a, O* out, Ctx* ctx)
{
    out->m4 = 0x10a;
    fn_8001A6B8((int)((fn_80253E38(&ctx->m130->m588, 1, 1) + 0.05f) * 10.0f));
}

void fn_80063F74(A* a, O* out, Ctx* ctx)
{
    out->m4 = 0x75;
    fn_80019C8C(fn_8001D42C(ctx));
}

void fn_80063FA4(A* a, O* out, Ctx* ctx)
{
    out->m4 = 0x76;
    fn_80019C30(fn_8001D42C(ctx));
}

void fn_80063FD4(A* a, O* out, Ctx* ctx)
{
    int v = -1;
    out->m4 = 0x39;
    Unk8005DF2C_O3B0* t = ctx->m3B0;
    fn_800B555C();
    if (fn_800B57EC())
        v = fn_80253F0C(&t->m588, 3, 2, 1);
    fn_80016ADC(4, v - 10);
}

void fn_8006403C(A* a, O* out, Ctx* ctx)
{
    int v = -1;
    out->m4 = 0x3a;
    Unk8005DF2C_O3B0* t = ctx->m3B0;
    fn_800B555C();
    if (fn_800B57EC())
        v = fn_80253F0C(&t->m588, 4, 2, 1);
    fn_80016B40(8, v - 5);
}

void fn_800640A4(A* a, O* out, Ctx* ctx)
{
    int v = -1;
    out->m4 = 0x3c;
    Unk8005DF2C_O3B0* t = ctx->m3B0;
    fn_800B555C();
    if (fn_800B57EC())
        v = (int)fn_80253F54(&t->m588, 0, 2, 1);
    fn_80016BA4(1, v - 0x4b);
}

void fn_80064114(A* a, O* out, Ctx* ctx)
{
    int v = -1;
    out->m4 = 0x3d;
    Unk8005DF2C_O3B0* t = ctx->m3B0;
    fn_800B555C();
    if (fn_800B57EC())
        v = fn_80253DB0(&t->m588, 3, 1);
    fn_80016C08(0x10, v - 0x1e);
}

void fn_80064178(A* a, O* out, Ctx* ctx)
{
    int v = -1;
    out->m4 = 0x3e;
    Unk8005DF2C_O3B0* t = ctx->m3B0;
    fn_800B555C();
    if (fn_800B57EC())
        v = fn_80253DB0(&t->m588, 1, 1);
    fn_80016C6C(2, v);
}

void fn_800641DC(A* a, O* out, Ctx* ctx)
{
    int x = -1;
    out->m4 = 0x3b;
    int y = -1;
    int z = -1;
    Unk8005DF2C_O3B0* t = ctx->m3B0;
    fn_800B555C();
    if (fn_800B57EC()) {
        Unk8005DF2C_Stat* s = &t->m588;
        float p = fn_80253F54(s, 0, 2, 1);
        float q = fn_80253F54(s, 1, 2, 1);
        x = (int)(((p / q) + 0.0005f) * 1000.0f);
        y = fn_80253F0C(s, 1, 2, 1);
        z = fn_80253F0C(s, 0, 2, 1);
    }
    fn_80016A68(0x260, x - 0xc8, y, z);
}

int fn_800642D0(int* a, int* b)
{
    if (*a >= *b)
        return *a > *b;
    return -1;
}

Unk800642F0::Unk800642F0()
{
    m8 = 0;
    m10 = 0;
    m18 = 0;
    m20 = 0;
    m38 = 0;
    m1C = new (0) Rec[50];
    mC = new (0) Rec[200];
    m24 = new (0) Rec[2];
    m14 = new (0) Rec[2];
    m2C = new (0) Rec[2];
    m4 = new (0) int[50];
    fn_803EE7BC();
}

Unk800642F0::~Unk800642F0()
{
    if (m38)
        fn_80064F78();
    if (m4)
        delete[] m4;
    fn_803EE7BC();
}

int Unk800642F0::fn_800644CC()
{
    return 0x100;
}

void Unk800642F0::fn_800644D4()
{
    fn_803EE7BC();
    if (m38)
        fn_80064F78();
    m3C = 0;
    fn_80064594();
    fn_800649C0();
    fn_80064858();
    fn_80064BFC();
    fn_80064AB0();
    fn_80064D48();
    int total = m18 + m20 + m10 + m8 + m28;
    fn_803EE7BC();
    fn_8033E958(lbl_805AE330, total, m3C);
    m38 = 1;
}

void Unk800642F0::fn_80064594()
{
    int tier = 2;
    Unk8005DF2C_Y* y = lbl_806192E0->m4->m14;
    Unk8005DF2C_Y* y2 = y + 1;
    if (y->m198 == y[1].m198)
        tier = 1;
    m0 = tier * 25;
    for (int i = 0; i <= 0x18; i++) {
        m4[i] = fn_8008CFD8(&y->mC9C, 0, (unsigned char)i);
        if (m4[i] > 0x4e1f)
            m4[i] -= 0x4e20;
    }
    if (tier > 1) {
        for (int i = 0x19; i <= 0x31; i++) {
            m4[i] = fn_8008CFD8(&y2->mC9C, 0, (unsigned char)(i - 0x19));
            if (m4[i] > 0x4e1f)
                m4[i] -= 0x4e20;
        }
    }
    fn_8040A208(m4, m0, 4, fn_800642D0);
    for (int j = m0 - 1; j > 0; j--) {
        if (m4[j] == m4[j - 1]) {
            for (int k = j; k < m0 - 1; k++)
                m4[k] = m4[k + 1];
            m0 = m0 - 1;
            m4[m0] = 0;
        }
    }
}

int Unk800642F0::fn_8006473C(const char* name, int h, Rec* r)
{
    Str s1;
    fn_8040B61C(&s1, name);
    int ok = 0;
    fn_8040B448(&s1, lbl_805AE378);
    Str s2;
    fn_8040B61C(&s2, name);
    fn_8040B448(&s2, lbl_805AE380);
    if (fn_802B96C4(&s1, 1)) {
        int sz;
        r->m8 = fn_802B9C1C(&s1, &sz, 0);
        m3C = m3C + sz;
        if (lbl_80617808 < lbl_80617804 - 1) {
            r->m0 = SPCH_AddBank(r->m8);
            lbl_80617808 += 1;
        }
        ok = 1;
        r->m4 = fn_803FC438(h, &s2);
    }
    return ok;
}

int Unk800642F0::fn_80064830(int a)
{
    return fn_802B9804(a, 0);
}

void Unk800642F0::fn_80064858()
{
    int h = fn_80064830(lbl_80618FB0);
    if (h) {
        int f;
        if (fn_803FED48(lbl_80618F9C, 0, 0, &f)) {
            m8 = 0;
            int i = 0;
            if (i < m0) {
                do {
                    char buf[16];
                    fn_804089B4(buf, lbl_805AE388, m4[i]);
                    if (fn_8006473C(buf, h, mC + m8))
                        m8 = m8 + 1;
                    buf[4] = 0x63;
                    if (fn_8006473C(buf, h, mC + m8))
                        m8 = m8 + 1;
                    buf[4] = 0x64;
                    if (fn_8006473C(buf, h, mC + m8))
                        m8 = m8 + 1;
                    i++;
                } while (i < m0);
            }
            fn_803FEDB8(f, 0);
        }
        MEM_free(h);
    }
}

void Unk800642F0::fn_800649C0()
{
    int h = fn_80064830(lbl_80618FA8);
    if (h) {
        int f;
        if (fn_803FED48(lbl_80618F94, 0, 0, &f)) {
            m18 = 0;
            int i = 0;
            if (i < m0) {
                do {
                    char buf[16];
                    fn_804089B4(buf, lbl_805AE390, m4[i]);
                    if (fn_8006473C(buf, h, m1C + m18))
                        m18 = m18 + 1;
                    i++;
                } while (i < m0);
            }
            fn_803FEDB8(f, 0);
        }
        MEM_free(h);
    }
}

void Unk800642F0::fn_80064AB0()
{
    int h = fn_80064830(lbl_80618FB4);
    if (h) {
        int f;
        if (fn_803FED48(lbl_80618FA0, 0, 0, &f)) {
            Unk8005DF2C_Y* y = lbl_806192E0->m4->m14;
            int a = fn_8009E1D4(&y->m8C, 2, 0);
            Unk8005DF2C_Y* y2 = y + 1;
            int b = fn_8009E1D4(&y2->m8C, 2, 0);
            m10 = 0;
            char buf[16];
            fn_804089B4(buf, lbl_805AE398, a);
            if (fn_8006473C(buf, h, m14 + m10))
                m10 = m10 + 1;
            if (a != b) {
                fn_804089B4(buf, lbl_805AE398, b);
                if (fn_8006473C(buf, h, m14 + m10))
                    m10 = m10 + 1;
            }
            fn_803FEDB8(f, 0);
        }
        MEM_free(h);
    }
}

void Unk800642F0::fn_80064BFC()
{
    int h = fn_80064830(lbl_80618FAC);
    if (h) {
        int f;
        if (fn_803FED48(lbl_80618F98, 0, 0, &f)) {
            Unk8005DF2C_Y* y = lbl_806192E0->m4->m14;
            int a = fn_8009E1D4(&y->m8C, 2, 0);
            Unk8005DF2C_Y* y2 = y + 1;
            int b = fn_8009E1D4(&y2->m8C, 2, 0);
            m20 = 0;
            char buf[16];
            fn_804089B4(buf, lbl_805AE398, a);
            if (fn_8006473C(buf, h, m24 + m20))
                m20 = m20 + 1;
            if (a != b) {
                fn_804089B4(buf, lbl_805AE398, b);
                if (fn_8006473C(buf, h, m24 + m20))
                    m20 = m20 + 1;
            }
            fn_803FEDB8(f, 0);
        }
        MEM_free(h);
    }
}
