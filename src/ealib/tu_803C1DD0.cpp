/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803C1DD0-0x803CB728 (code-only unit, 34 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

struct EAGLInit81F4 {
    char pad0[0xC];
    int mC;
    int m10;
    char pad14[8];
    int m1C;
};

struct EAGLKey3 {
    float a[3];
    float b[3];
    unsigned short c[3];
    unsigned short d[3];
};

struct EAGLDst3 {
    float x[3];
    float y[3];
    float z[3];
    float w[3];
};

struct EAGLRecD {
    char pad0[0xE];
    unsigned short mE;
    char pad10[4];
    EAGLKey3 m14[1];
};

struct EAGLSelfD3 {
    char pad0[0xC];
    EAGLRecD* mC;
    int m10;
    void* m14;
    void* m18;
    int m1C;
    void* m20;
    void* m24;
    int m28;
    EAGLDst3* m2C;
};

/* generic EAGL object data view (vptr at +0, data to +0x3C) */
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
    int m24;
    int m28;
    int m2C;
    int m30;
    int m34;
    int m38;
    int m3C;
};

extern char lbl_80685A88[];
extern char lbl_80685B40[];

extern "C" EAGLObj* fn_803BE8EC(EAGLObj* s);

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
extern void (*lbl_8062C66C)(void*, int);

extern "C" void fn_803BE90C(void* self, int chrg);

struct EAGLInfoU {
    char pad0[8];
    unsigned short* m8;
    unsigned short mC;
};

struct EAGLSelfU {
    char pad0[0xC];
    EAGLInfoU* mC;
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

struct EAGLSelfV4 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4(float, float);
};

struct EAGLRecB {
    char pad0[4];
    unsigned short* m4;
    char pad8[6];
    unsigned short mE;
};

struct EAGLSelfRB {
    char pad0[0xC];
    EAGLRecB* mC;
};

struct EAGLSetMC {
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

struct EAGLKey {
    float m0;
    float m4;
    unsigned short m8;
    unsigned short mA;
};

struct EAGLDst {
    float m0;
    float m4;
    float m8;
    float mC;
};

struct EAGLRecC {
    char pad0[0xE];
    unsigned short mE;
    char pad10[4];
    EAGLKey m14[1];
};

struct EAGLSelfC {
    char pad0[0xC];
    EAGLRecC* mC;
    int m10;
    void* m14;
    void* m18;
    int m1C;
    void* m20;
    void* m24;
    int m28;
    EAGLDst* m2C;
};

extern char lbl_80685C00[];
extern char lbl_80685CB8[];

extern "C" void SetBone(unsigned* set, int k, int v);

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

extern "C" void fn_803C3A38(EAGLSelfD3* s)
{
    EAGLRecD* r = s->mC;
    if (r->mE != 0) {
        s->m14 = s->m18 = lbl_80626DD0->v6(r->mE * 16);
        s->m20 = s->m24 = lbl_80626DD0->v6(r->mE * 16);
        s->m2C = (EAGLDst3*)lbl_80626DD0->v6(r->mE * 0x30);
        int i;
        EAGLKey3* k = r->m14;
        for (i = 0; i < r->mE; k++, i++) {
            int j;
            for (j = 0; j < 3; j++) {
                s->m2C[i].x[j] = k->a[j];
                s->m2C[i].y[j] = k->b[j];
                s->m2C[i].y[j] *= 1.5259022e-05f;
                s->m2C[i].z[j] = 2.0f * ((float)(int)k->c[j] * 1.5259022e-05f) * k->b[j] - k->b[j];
                s->m2C[i].w[j] = 2.0f * ((float)(int)k->d[j] * 1.5259022e-05f) * k->b[j];
                s->m2C[i].w[j] *= 0.003921569f;
            }
        }
    }
}

extern "C" EAGLObj* fn_803C3C38(EAGLObj* s)
{
    fn_803BE8EC(s);
    s->m10 = -1;
    s->m0 = lbl_80685A88;
    s->m1C = -1;
    s->m14 = 0;
    s->m18 = 0;
    s->m20 = 0;
    s->m24 = 0;
    s->m8 = 0x14;
    return s;
}

extern "C" void fn_803C3C9C(EAGLObj* s, int chrg)
{
    s->m0 = lbl_80685A88;
    if (s->m18 != 0) {
        lbl_80626DD0->v7((void*)s->m14);
        lbl_80626DD0->v7((void*)s->m20);
        lbl_80626DD0->v7((void*)s->m2C);
    }
    fn_803BE90C(s, 0);
    if (chrg & 1)
        lbl_8062C66C(s, 0x30);
}

extern "C" void fn_803C3D70(EAGLInit81F4* s, int v)
{
    s->mC = v;
    s->m10 = -1;
    s->m1C = -1;
}

extern "C" int fn_803C3D84(EAGLSelfU* s, float* out)
{
    EAGLInfoU* p = s->mC;
    int v;
    if (p->m8 == 0)
        v = p->mC;
    else
        v = p->m8[p->mC - 2] + 1;
    *out = (float)(unsigned short)v;
    return 1;
}

extern "C" void fn_803C3DE8(EAGLSelfV7* s, int a, float x, float y)
{
    s->v7(a, y, 0);
}

extern "C" int fn_803C3E24(EAGLSelfV4* s, float x)
{
    s->v4(x, x);
    return 1;
}

extern "C" int fn_803C3E60(EAGLSelfV4* s, float x)
{
    s->v4(x, x);
    return 1;
}

extern "C" int fn_803C3E9C(EAGLSelfRB* s, unsigned* bits, int* cnt, int* lo, int* hi)
{
    EAGLRecB* r = s->mC;
    int i;
    for (i = r->mE - 1; i >= 0; i--) {
        int b = (unsigned)r->m4[i] / 12;
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

extern "C" EAGLObj* fn_803C6894(EAGLObj* s)
{
    fn_803BE8EC(s);
    s->m10 = 0;
    s->m14 = 0;
    s->m18 = -1;
    s->m0 = lbl_80685B40;
    s->m20 = 0;
    s->m24 = 0;
    s->m1C = -1;
    s->m28 = 0;
    s->m2C = 0;
    s->m8 = 0x11;
    return s;
}

extern "C" void fn_803C6900(EAGLObj* s, int chrg)
{
    s->m0 = lbl_80685B40;
    if (s->m20 != 0)
        lbl_80626DD0->v7((void*)s->m20);
    fn_803BE90C(s, 0);
    if (chrg & 1)
        lbl_8062C66C(s, 0x30);
}

extern "C" void fn_803C6994(EAGLSetMC* s, int v)
{
    s->mC = v;
}

extern "C" int fn_803C699C(EAGLSelfB* s, float* out)
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

extern "C" void fn_803C6A00(EAGLSelfV22* s, int a, float x, float y)
{
    s->v22(0, a, y);
}

extern "C" int fn_803C6A40(EAGLSelfRA* s, unsigned* bits, int* cnt, int* lo, int* hi)
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

extern "C" void fn_803C7F00(EAGLSelfC* s)
{
    EAGLRecC* r = s->mC;
    if (r->mE != 0) {
        s->m14 = s->m18 = lbl_80626DD0->v6(r->mE * 4);
        s->m20 = s->m24 = lbl_80626DD0->v6(r->mE * 4);
        s->m2C = (EAGLDst*)lbl_80626DD0->v6(r->mE * 16);
        int i;
        EAGLKey* k = r->m14;
        for (i = 0; i < r->mE; i++, k++) {
            s->m2C[i].m0 = k->m0;
            s->m2C[i].m4 = k->m4;
            s->m2C[i].m4 *= 1.5259022e-05f;
            s->m2C[i].m8 = 2.0f * ((float)(int)k->m8 * 1.5259022e-05f) * k->m4 - k->m4;
            s->m2C[i].mC = 2.0f * ((float)(int)k->mA * 1.5259022e-05f) * k->m4;
            s->m2C[i].mC *= 0.003921569f;
        }
    }
}

extern "C" EAGLObj* fn_803C80BC(EAGLObj* s)
{
    fn_803BE8EC(s);
    s->m10 = -1;
    s->m0 = lbl_80685C00;
    s->m1C = -1;
    s->m14 = 0;
    s->m18 = 0;
    s->m20 = 0;
    s->m24 = 0;
    s->m8 = 0x15;
    return s;
}

extern "C" void fn_803C8120(EAGLObj* s, int chrg)
{
    s->m0 = lbl_80685C00;
    if (s->m18 != 0) {
        lbl_80626DD0->v7((void*)s->m14);
        lbl_80626DD0->v7((void*)s->m20);
        lbl_80626DD0->v7((void*)s->m2C);
    }
    fn_803BE90C(s, 0);
    if (chrg & 1)
        lbl_8062C66C(s, 0x30);
}

extern "C" void fn_803C81F4(EAGLInit81F4* s, int v)
{
    s->mC = v;
    s->m10 = -1;
    s->m1C = -1;
}

extern "C" int fn_803C8208(EAGLSelfU* s, float* out)
{
    EAGLInfoU* p = s->mC;
    int v;
    if (p->m8 == 0)
        v = p->mC;
    else
        v = p->m8[p->mC - 2] + 1;
    *out = (float)(unsigned short)v;
    return 1;
}

extern "C" void fn_803C826C(EAGLSelfV7* s, int a, float x, float y)
{
    s->v7(a, y, 0);
}

extern "C" int fn_803C82A8(EAGLSelfV4* s, float x)
{
    s->v4(x, x);
    return 1;
}

extern "C" int fn_803C82E4(EAGLSelfV4* s, float x)
{
    s->v4(x, x);
    return 1;
}

extern "C" int fn_803C8320(EAGLSelfRB* s, unsigned* bits, int* cnt, int* lo, int* hi)
{
    EAGLRecB* r = s->mC;
    int i;
    for (i = r->mE - 1; i >= 0; i--) {
        int b = (unsigned)r->m4[i] / 12;
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

extern "C" EAGLObj* fn_803CB5E8(EAGLObj* s)
{
    fn_803BE8EC(s);
    s->m10 = 0;
    s->m14 = 0;
    s->m18 = -1;
    s->m0 = lbl_80685CB8;
    s->m20 = 0;
    s->m24 = 0;
    s->m1C = -1;
    s->m28 = 0;
    s->m2C = 0;
    s->m8 = 0x13;
    return s;
}

extern "C" void fn_803CB654(EAGLObj* s, int chrg)
{
    s->m0 = lbl_80685CB8;
    if (s->m20 != 0) {
        lbl_80626DD0->v7((void*)s->m20);
        lbl_80626DD0->v7((void*)s->m28);
        lbl_80626DD0->v7((void*)s->m2C);
    }
    fn_803BE90C(s, 0);
    if (chrg & 1)
        lbl_8062C66C(s, 0x30);
}
