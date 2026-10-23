/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803BE8BC-0x803C1DD0 (code-only unit, 43 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

/* EAGLAnim resource base classes (vtable slot N is at byte offset 8*N). */
struct EAGLHeap {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void* v6(int);
    virtual void v7(void*);
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void* v11(int);
    virtual void v12(void*);
    virtual void v13(void*);
};

struct EAGLSelfA {
    void* m0;
    int m4;
    int m8;
    int mC;
};

struct EAGLInfo {
    unsigned short m0;
    unsigned short m2;
    unsigned short m4;
    unsigned short m6;
    unsigned short* m8;
};

struct EAGLSelfB {
    char pad0[0xC];
    EAGLInfo* mC;
};

struct EAGLInfoB {
    int pad0;
    int m4;
};

struct EAGLSelfF {
    char pad0[0xC];
    EAGLInfoB* mC;
};

struct EAGLSelfD {
    char pad0[8];
    unsigned short m8;
    unsigned short padA;
    int mC[1];
};

struct EAGLSelfE {
    void* m0;
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
};

struct EAGLSelfV7 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7(int, float, int);
};

extern EAGLHeap* lbl_80626DD0;
extern char lbl_80685860[];
extern char lbl_806859D0[];

struct EAGLInfoA {
    char pad0[0xA];
    unsigned short mA;
};

struct EAGLSelfRatio {
    char pad0[0xC];
    EAGLInfoA* mC;
    int m10;
    int m14;
    unsigned char m18;
};

struct EAGLRecCnt {
    char pad0[8];
    unsigned short m8;
};

struct EAGLRecArr {
    char pad0[8];
    unsigned short m8;
    char padA[2];
    int mC[1];
};

struct EAGLSelfFree {
    const void* m0;
    char pad4[8];
    EAGLRecCnt* mC;
    int* m10;
};

extern void (*lbl_8062C66C)(void*, int);

extern "C" int fn_803BE8BC() { return 0; }

extern "C" int fn_803BE8C4() { return 0; }

extern "C" int fn_803BE8CC() { return 0; }

extern "C" int fn_803BE8D4() { return 0; }

extern "C" int fn_803BE8DC() { return 0; }

extern "C" int fn_803BE8E4() { return 0; }

struct DtorAt0 { const void* m; };

extern char lbl_80685268[];

extern "C" void fn_80377408(void* p);

struct EAGLChild7 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual int v7(float, int, int);
};

struct EAGLSelfArr7 {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild7** m10;
    int m14;
    unsigned char m18;
};

struct EAGLChild13 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual int v13(float, int, int);
};

struct EAGLSelfArr13 {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild13** m10;
    int m14;
    unsigned char m18;
};

struct EAGLChild11 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual int v11(float, int);
};

struct EAGLSelfArr {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild11** m10;
    int m14;
    unsigned char m18;
};

struct EAGLChild9 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual int v9(float, int);
};

struct EAGLSelfArr9 {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild9** m10;
    int m14;
    unsigned char m18;
};

struct EAGLChild12 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual int v12(float, int);
};

struct EAGLSelfArr12 {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild12** m10;
    int m14;
    unsigned char m18;
};

struct EAGLChild15 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual int v15();
};

struct EAGLSelfArr15 {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild15** m10;
};

struct EAGLChild18 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual int v18(int, int, int, int);
};

struct EAGLSelfArr18 {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild18** m10;
};

extern char lbl_80685918[];

struct EAGLChild4 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4(float, float, int);
};

struct EAGLSelfTwo {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild4** m10;
    int m14;
    unsigned char m18;
};

struct EAGLChild14 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual int v14(int, float, float*);
};

struct EAGLSelfFind {
    char pad0[0xC];
    EAGLRecCnt* mC;
    EAGLChild14** m10;
    int m14;
    unsigned char m18;
};

struct EAGLChild8 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual int v8(float, int);
};

struct EAGLSelfTick {
    char pad0[0xC];
    EAGLRecCnt* mC;
    EAGLChild8** m10;
    int m14;
    unsigned char m18;
};

class Unk803D2B20 {
public:
    virtual ~Unk803D2B20();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7(int a, float f, int z);
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void* v16();
    virtual void v17();
    virtual int v18(int a, int* b, int* c, int* d);
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
};

extern "C" void Invisible(int a, int b);

struct EAGLRecA {
    char pad0[6];
    unsigned char m6;
    char pad7[5];
    unsigned char* mC;
};

struct EAGLSelfRA {
    char pad0[0xC];
    EAGLRecA* mC;
};

inline int EAGLTestBit(unsigned* bits, int idx)
{
    unsigned* q = &bits[idx >> 5];
    return (*q & (1 << (idx & 31))) != 0 ? 1 : 0;
}

extern "C" void SetBone(unsigned* set, int k, int v);

struct EAGLChild10 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual int v10(float, float, int, int);
};

struct EAGLSelfArr10 {
    char pad0[0xC];
    EAGLRecArr* mC;
    EAGLChild10** m10;
    int m14;
    unsigned char m18;
};

extern "C" EAGLSelfA* fn_803BE8EC(EAGLSelfA* s)
{
    s->m0 = lbl_80685860;
    s->m4 = 0;
    s->mC = 0;
    return s;
}

extern "C" void fn_803BE90C(DtorAt0* self, int chrg)
{
    self->m = lbl_80685268;
    if (chrg & 1)
        fn_80377408(self);
}

extern "C" void fn_803BE940(EAGLSelfA* s, int v)
{
    s->mC = v;
}

extern "C" int fn_803BE948(EAGLSelfA* s)
{
    return s->mC;
}

extern "C" int fn_803BE950(EAGLSelfA* s)
{
    return s->mC;
}

extern "C" int fn_803BE958(EAGLSelfB* s)
{
    return s->mC->m2;
}

extern "C" int EvalEvent(EAGLSelfArr10* s, float x, float y, int a, int b)
{
    EAGLRecArr* rec = s->mC;
    int r = 0;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild10**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild10*)lbl_80626DD0->v11(rec->mC[i]);
    }
    if (s->m14 != 0) {
        float k = (float)(int)s->m18;
        x *= k;
        y *= k;
    }
    for (i = rec->m8 - 1; i >= 0; i--)
        r = (r != 0) | (s->m10[i]->v10(x, y, a, b) != 0);
    return r;
}

extern "C" int fn_803BEAF4(EAGLSelfArr7* s, float t, int a, int b)
{
    EAGLRecArr* rec = s->mC;
    int r = 0;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild7**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild7*)lbl_80626DD0->v11(rec->mC[i]);
    }
    if (s->m14 != 0)
        t *= (float)(int)s->m18;
    for (i = rec->m8 - 1; i >= 0; i--)
        r = (r != 0) | (s->m10[i]->v7(t, a, b) != 0);
    return r;
}

extern "C" int fn_803BEC70(EAGLSelfArr13* s, float t, int a, int b)
{
    EAGLRecArr* rec = s->mC;
    int r = 0;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild13**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild13*)lbl_80626DD0->v11(rec->mC[i]);
    }
    if (s->m14 != 0)
        t *= (float)(int)s->m18;
    for (i = rec->m8 - 1; i >= 0; i--)
        r = (r != 0) | (s->m10[i]->v13(t, a, b) != 0);
    return r;
}

extern "C" int fn_803BEDEC(EAGLSelfArr* s, float t, int a)
{
    EAGLRecArr* rec = s->mC;
    int r = 0;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild11**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild11*)lbl_80626DD0->v11(rec->mC[i]);
    }
    if (s->m14 != 0)
        t *= (float)(int)s->m18;
    for (i = rec->m8 - 1; i >= 0; i--)
        r = (r != 0) | (s->m10[i]->v11(t, a) != 0);
    return r;
}

extern "C" int fn_803BEF60(EAGLSelfArr9* s, float t, int a)
{
    EAGLRecArr* rec = s->mC;
    int r = 0;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild9**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild9*)lbl_80626DD0->v11(rec->mC[i]);
    }
    if (s->m14 != 0)
        t *= (float)(int)s->m18;
    for (i = rec->m8 - 1; i >= 0; i--)
        r = (r != 0) | (s->m10[i]->v9(t, a) != 0);
    return r;
}

extern "C" int fn_803BF0D4(EAGLSelfArr12* s, float t, int a)
{
    EAGLRecArr* rec = s->mC;
    int r = 0;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild12**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild12*)lbl_80626DD0->v11(rec->mC[i]);
    }
    if (s->m14 != 0)
        t *= (float)(int)s->m18;
    for (i = rec->m8 - 1; i >= 0; i--)
        r = (r != 0) | (s->m10[i]->v12(t, a) != 0);
    return r;
}

extern "C" int GetPhaseChan(EAGLSelfArr15* s)
{
    EAGLRecArr* rec = s->mC;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild15**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild15*)lbl_80626DD0->v11(rec->mC[i]);
    }
    for (i = rec->m8 - 1; i >= 0; i--) {
        int r = s->m10[i]->v15();
        if (r != 0)
            return r;
    }
    return 0;
}

extern "C" int fn_803BF34C(EAGLSelfArr18* s, int a, int b, int c, int d)
{
    EAGLRecArr* rec = s->mC;
    int r = 0;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild18**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild18*)lbl_80626DD0->v11(rec->mC[i]);
    }
    for (i = rec->m8 - 1; i >= 0; i--)
        r = (r != 0) | (s->m10[i]->v18(a, b, c, d) != 0);
    return r;
}

extern "C" int fn_803BF490(EAGLSelfB* s)
{
    return s->mC->m2;
}

extern "C" int fn_803BF49C(EAGLSelfRatio* s, float* out)
{
    float v = (float)s->mC->mA;
    *out = v;
    if (s->m14 != 0)
        *out = v / (float)s->m18;
    return 1;
}

extern "C" void fn_803BF504(EAGLSelfFree* s, int chrg)
{
    s->m0 = lbl_80685918;
    if (s->m10 != 0) {
        int i;
        for (i = s->mC->m8 - 1; i >= 0; i--)
            lbl_80626DD0->v12((void*)s->m10[i]);
        lbl_80626DD0->v7(s->m10);
    }
    fn_803BE90C((DtorAt0*)s, 0);
    if (chrg & 1)
        lbl_8062C66C(s, 0x1c);
}

extern "C" void fn_803BF5E0(EAGLSelfFree* s, EAGLRecCnt* r)
{
    s->mC = r;
    if (s->m10 != 0) {
        int i;
        for (i = r->m8 - 1; i >= 0; i--)
            lbl_80626DD0->v12((void*)s->m10[i]);
        lbl_80626DD0->v7(s->m10);
    }
    s->m10 = 0;
}

extern "C" int fn_803BF688(EAGLSelfF* s)
{
    return s->mC->m4;
}

extern "C" void fn_803BF694(EAGLSelfTwo* s, float x, float y, int a)
{
    EAGLRecArr* rec = s->mC;
    int i;
    if (s->m10 == 0) {
        s->m10 = (EAGLChild4**)lbl_80626DD0->v6(rec->m8 * 4);
        for (i = rec->m8 - 1; i >= 0; i--)
            s->m10[i] = (EAGLChild4*)lbl_80626DD0->v11(rec->mC[i]);
    }
    if (s->m14 != 0) {
        float k = (float)(int)s->m18;
        x *= k;
        y *= k;
    }
    for (i = rec->m8 - 1; i >= 0; i--)
        s->m10[i]->v4(x, y, a);
}

extern "C" int FindTime(EAGLSelfFind* s, int a, float t, float* out)
{
    EAGLRecCnt* rec = s->mC;
    int i;
    if (s->m14 != 0)
        t *= (float)(int)s->m18;
    for (i = rec->m8 - 1; i >= 0; i--) {
        if (s->m10[i]->v14(a, t, out) != 0) {
            if (s->m14 != 0)
                *out /= (float)(int)s->m18;
            return 1;
        }
    }
    return 0;
}

extern "C" int EvalPhase(EAGLSelfTick* s, float t, int a)
{
    int r = 0;
    EAGLRecCnt* rec = s->mC;
    if (s->m14 != 0)
        t *= (float)(int)s->m18;
    int i;
    for (i = rec->m8 - 1; i >= 0; i--)
        r = (r != 0) | (s->m10[i]->v8(t, a) != 0);
    return r;
}

extern "C" void fn_803BF9F0(EAGLSelfD* s)
{
    int i;
    for (i = s->m8 - 1; i >= 0; i--)
        lbl_80626DD0->v13((void*)s->mC[i]);
}

#include "../../include/game/eagl_key_803d2d74.h"

struct EAGLSelfScale {
    const void* m0;
    int m4;
    int m8;
    void* mC;
    int m10;
    int m14;
    unsigned char m18;
};

extern "C" int fn_803D2D74(void* table, Unk803D2B20Key* key, int* out);

extern "C" void UseFPS(EAGLSelfScale* s, int on)
{
    s->m14 = on;
    if (s->m18 == 0 && on != 0) {
        Unk803D2B20Key key(1);
        void* tbl = ((Unk803D2B20*)s)->v16();
        if (tbl) {
            Unk803D2B20Key k2(key);
            Unk803D2B20Key k3(k2);
            int res;
            if (fn_803D2D74(tbl, &k3, &res))
                s->m18 = ((unsigned char*)&res)[0];
        }
    }
}

extern "C" int fn_803BFAEC(Unk803D2B20* self, int a, int* b, int* c, int* d)
{
    Invisible(a, 0);
    *b = 0;
    return self->v18(a, b, c, d);
}

/* near-miss: constants land in swapped registers (-1 in r10, 18 in r11; retail r11/r10) */
extern "C" EAGLSelfE* fn_803C1B50(EAGLSelfE* s)
{
    fn_803BE8EC((EAGLSelfA*)s);
    s->m0 = lbl_806859D0;
    s->m28 = -1;
    s->m3C = 0;
    s->m8 = 0x12;
    s->m10 = 0;
    s->m14 = 0;
    s->m18 = -1;
    s->m1C = -1;
    s->m20 = 0;
    s->m24 = 0;
    s->m2C = 0;
    s->m30 = 0;
    s->m34 = 0;
    s->m38 = 0;
    return s;
}

extern "C" void fn_803C1BCC(EAGLSelfE* s, int chrg)
{
    s->m0 = lbl_806859D0;
    if (s->m10 != 0)
        lbl_80626DD0->v7((void*)s->m10);
    fn_803BE90C((DtorAt0*)s, 0);
    if (chrg & 1)
        lbl_8062C66C(s, 0x40);
}

extern "C" int fn_803C1C60(EAGLSelfB* s, float* out)
{
    EAGLInfo* p = s->mC;
    int v;
    if (p->m8 == 0)
        v = p->m4;
    else
        v = p->m8[p->m4 - 2] + 1;
    *out = (float)v;
    return 1;
}

extern "C" void fn_803C1CC4(EAGLSelfV7* s, int a, float x, float y)
{
    s->v7(a, y, 0);
}

extern "C" int fn_803C1D00(EAGLSelfRA* s, unsigned* bits, int* cnt, int* lo, int* hi)
{
    EAGLRecA* r = s->mC;
    int i;
    for (i = r->m6 - 1; i >= 0; i--) {
        int b = r->mC[i];
        if (!EAGLTestBit(bits, b)) {
            if (*cnt == 0) {
                *lo = b;
                *hi = b;
            } else {
                if (b < *lo)
                    *lo = b;
                if (b > *hi)
                    *hi = b;
            }
            *cnt += 1;
            SetBone(bits, b, 1);
        }
    }
    return 1;
}
