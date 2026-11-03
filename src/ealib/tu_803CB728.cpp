/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803CB728-0x803CECA4 (code-only unit, 39 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

struct DtorAt0 { const void* m; };

extern char lbl_80686268[];

extern "C" void fn_803BE90C(void* s, int a);

extern "C" void fn_803CEC78(void* p, int n);

struct EAGLSetC {
    char pad0[0xC];
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

struct EAGLSelfV22 {
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
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22(int, int, float);
};

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

struct EAGLCount {
    unsigned short m0;
};

struct EAGLRecN {
    int m0;
    EAGLCount* m4;
};

struct EAGLSelfN {
    char pad0[0xC];
    EAGLRecN* mC;
    int m10;
    void* m14;
};

struct EAGLObj {
    void* m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
};

extern char lbl_80686110[];

extern "C" EAGLObj* fn_803BE8EC(EAGLObj* s);

struct EAGLInfoM8 {
    char pad0[8];
    unsigned short m8;
};

struct EAGLSelfM8 {
    char pad0[0xC];
    EAGLInfoM8* mC;
};

struct EAGLHeapX {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void* v6(int);
    virtual void v7(void*);
};

extern EAGLHeapX* lbl_80626DD0;

extern "C" void fn_803BE90C(void* self, int chrg);

struct EAGLObjD {
    void* m0;
    int m4;
    int m8;
    int mC;
    int m10;
    void* m14;
    int m18;
    void* m1C;
    int m20;
};

extern char lbl_80685EE8[];

struct EAGLSelfV4 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4(float, float);
};

struct EAGLRecE {
    char pad0[4];
    EAGLCount* m4;
    char pad8[2];
    unsigned short mA[1];
};

struct EAGLSelfE {
    char pad0[0xC];
    EAGLRecE* mC;
};

struct EAGLInfoM {
    char pad0[8];
    unsigned short* m8;
    unsigned short mC;
};

struct EAGLSelfM {
    char pad0[0xC];
    EAGLInfoM* mC;
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

struct EAGLRecF {
    char pad0[4];
    EAGLCount* m4;
    char pad8[6];
    unsigned short mE[1];
};

struct EAGLSelfF {
    char pad0[0xC];
    EAGLRecF* mC;
};

struct EAGLTmp {
    void* m0;
    int m4;
    int m8;
    int mC;
    int m10;
};

extern "C" EAGLTmp* fn_803D5098(EAGLTmp* s);

extern char lbl_80685268[];

extern void (*lbl_8062C66C)(void*, int);

extern "C" void fn_803CEBF8(void* p, int n);

struct EAGLBlendChild {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4(int, float, float);
};

struct EAGLBlend {
    char pad0[0xC];
    EAGLBlendChild* mC;
    EAGLBlendChild* m10;
    float m14;
    float m18;
    float m1C;
    float m20;
    int m24;
    int m28;
};

extern "C" void fn_803CB728(EAGLSetC* s, int v)
{
    s->mC = v;
}

extern "C" int fn_803CB730(EAGLSelfB* s, float* out)
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

extern "C" void fn_803CB794(EAGLSelfV22* s, int a, float x, float y)
{
    s->v22(0, a, y);
}

extern "C" int fn_803CB7D4(EAGLSelfRA* s, unsigned* bits, int* cnt, int* lo, int* hi)
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

extern "C" void fn_803CB8A4(EAGLSelfN* s, EAGLRecN* n)
{
    if (s->m14 != 0) {
        int old = s->mC->m4->m0;
        if (n->m4->m0 >= old) {
            lbl_80626DD0->v7(s->m14);
            s->m14 = lbl_80626DD0->v6(n->m4->m0 * 4);
        }
    } else {
        s->m14 = lbl_80626DD0->v6(n->m4->m0 * 4);
    }
    s->mC = n;
    s->m10 = -1;
}

extern "C" void fn_803CC3E8(EAGLSelfN* s, EAGLRecN* n)
{
    if (s->m14 != 0) {
        int old = s->mC->m4->m0;
        if (n->m4->m0 >= old) {
            lbl_80626DD0->v7(s->m14);
            s->m14 = lbl_80626DD0->v6(n->m4->m0 * 4);
        }
    } else {
        s->m14 = lbl_80626DD0->v6(n->m4->m0 * 4);
    }
    s->mC = n;
    s->m10 = -1;
}

extern "C" EAGLObj* fn_803CD534(EAGLObj* s)
{
    fn_803BE8EC(s);
    s->m0 = lbl_80686110;
    s->m10 = -1;
    s->m14 = 0;
    s->m18 = 0;
    s->m1C = 0;
    s->m20 = 0;
    return s;
}

extern "C" int fn_803CD58C(EAGLSelfM8* s, float* out)
{
    *out = (float)s->mC->m8;
    return 1;
}

extern "C" void fn_803CD5C8(EAGLObjD* s, int chrg)
{
    s->m0 = lbl_80686110;
    if (s->m14 != 0)
        lbl_80626DD0->v7(s->m14);
    if (s->m1C != 0)
        lbl_80626DD0->v7(s->m1C);
    fn_803BE90C(s, 0);
    if (chrg & 1)
        lbl_8062C66C(s, 0x24);
}

extern "C" int fn_803CD688(EAGLSelfV4* s, float x)
{
    s->v4(x, x);
    return 1;
}

extern "C" int fn_803CD6C4(EAGLSelfV4* s, float x)
{
    s->v4(x, x);
    return 1;
}

extern "C" int fn_803CD700(EAGLSelfE* s, unsigned* bits, int* cnt, int* lo, int* hi)
{
    EAGLRecE* r = s->mC;
    int i;
    for (i = r->m4->m0 - 1; i >= 0; i--) {
        int b = (unsigned)r->mA[i] / 12;
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

inline int EAGLCnt(EAGLCount* c) { return c->m0; }

extern "C" int fn_803CD7F0(EAGLSelfE* s, unsigned* bits, int* cnt, int* lo, int* hi)
{
    EAGLRecE* r = s->mC;
    int n = r->m4->m0;
    int i;
    for (i = n / 4 - 1; i >= 0; i--) {
        int b = (unsigned)r->mA[i] / 12;
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

extern "C" void fn_803CD8E4(EAGLObjD* s, int chrg)
{
    s->m0 = lbl_80685EE8;
    if (s->m14 != 0)
        lbl_80626DD0->v7(s->m14);
    if (s->m1C != 0)
        lbl_80626DD0->v7(s->m1C);
    fn_803BE90C(s, 0);
    if (chrg & 1)
        lbl_8062C66C(s, 0x24);
}

extern "C" int fn_803CD9A4(EAGLSelfM* s, float* out)
{
    EAGLInfoM* p = s->mC;
    *out = (float)(p->m8[p->mC - 2] + 1);
    return 1;
}

extern "C" void fn_803CD9F8(Unk803D2B20* self, int a, float x, float y)
{
    self->v7(a, y, 0);
}

extern "C" int fn_803CDA34(EAGLSelfF* s, unsigned* bits, int* cnt, int* lo, int* hi)
{
    EAGLRecF* r = s->mC;
    int i;
    for (i = r->m4->m0 - 1; i >= 0; i--) {
        int b = (unsigned)r->mE[i] / 12;
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

extern "C" void fn_803CDB24(Unk803D2B20* self, int a, float x, float y)
{
    self->v7(a, y, 0);
}

extern "C" int fn_803CDB60(EAGLSelfF* s, unsigned* bits, int* cnt, int* lo, int* hi)
{
    EAGLRecF* r = s->mC;
    int n = r->m4->m0;
    int i;
    for (i = n / 4 - 1; i >= 0; i--) {
        int b = (unsigned)r->mE[i] / 12;
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

extern "C" void fn_803CEA5C(char* p)
{
    EAGLTmp t;
    fn_803D5098(&t);
    *(void**)(p - 0x10) = t.m0;
    fn_803BE90C(&t, 0);
}

extern "C" void fn_803CEAA0(EAGLBlend* s, float x, float y, int a)
{
    if (y <= s->m1C) {
        s->mC->v4(a, x - s->m14, y - s->m14);
    } else if (y >= s->m20) {
        s->m10->v4(a, x - s->m18, y - s->m18);
    } else {
        switch (s->m28) {
        case 0:
            s->mC->v4(a, x - s->m14, y - s->m14);
            break;
        case 1:
            s->m10->v4(a, x - s->m18, y - s->m18);
            break;
        default:
            s->mC->v4(a, x - s->m14, y - s->m14);
            s->m10->v4(a, x - s->m18, y - s->m18);
            break;
        }
    }
}

extern "C" void fn_803CEBC0(DtorAt0* s, int chrg)
{
    s->m = lbl_80685268;
    if (chrg & 1)
        fn_803CEBF8(s, 0x2c);
}

extern "C" void fn_803CEBF8(void* p, int n)
{
    lbl_8062C66C(p, n);
}

extern "C" void fn_803CEC24(DtorAt0* s, int chrg)
{
    s->m = lbl_80686268;
    fn_803BE90C(s, 0);
    if (chrg & 1)
        fn_803CEC78(s, 0x14);
}

extern "C" void fn_803CEC78(void* p, int n)
{
    lbl_8062C66C(p, n);
}
