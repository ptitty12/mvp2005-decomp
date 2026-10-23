/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803BA9DC-0x803BD944 (code-only unit, 26 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

struct EAGLHeapY {
    virtual void v1();
    virtual void v2(int);
    virtual void v3();
    virtual int v4();
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
};

struct EAGLSlot {
    void* m0;
    int m4;
    int m8;
};

struct DtorAt0 { const void* m; };

extern EAGLHeapY* lbl_80626DD0;
extern void (*lbl_8062C66C)(void*, int);
extern EAGLSlot lbl_806AA8BC[];
extern char lbl_80685268[];

struct EAGLNameEnt {
    const char* name;
    int val;
};

struct EAGLNameTab {
    short pad0;
    unsigned short m2;
    EAGLNameEnt* m4;
    void* m8;
    int mC;
};

struct EAGLDelObj {
    virtual void v1(int);
};

extern "C" int strcmp(const char*, const char*);

extern EAGLDelObj* lbl_80626DCC;
extern void* lbl_80627284;
extern void* (*lbl_8062C668)(int, const char*);
extern char lbl_806AAB80[];
extern char lbl_8060D1B4[];
extern char lbl_80685688[];

extern "C" int strlen(const char*);
extern "C" void fn_803E7074(void* a, void* b);
extern "C" void fn_803E70B0(void* a, void* b, void (*c)(void*), void (*d)());
extern "C" void fn_803BD8BC();
extern "C" void Constructor(void*);
extern "C" void fn_803BE7B0();

inline int EAGLTestBit(unsigned* bits, int idx)
{
    unsigned* q = &bits[idx >> 5];
    return (*q & (1 << (idx & 31))) != 0 ? 1 : 0;
}

struct EAGLSk {
    char pad0[8];
    int m8;
};

extern void (*lbl_80626DC8)(void*, void*, void*);
extern "C" void fn_803DFB48(void*);
extern "C" void PoseSQTToGlobal(EAGLSk* sk, char* a, char* out, unsigned* mask);
extern "C" void fn_803E00EC(void* p, float a, float b, float c, float d, float e, float f, float g, float h, float i, float j);

extern "C" void PostMult(void* a, void* b);
extern "C" void fn_803BCE38(void* p, int n);
extern "C" void fn_803BD800(int a, int b);

extern "C" void PoseSQTToGlobal(EAGLSk* sk, char* a, char* out, unsigned* mask)
{
    if (mask != 0) {
        int n = sk->m8;
        int i;
        float* m = (float*)a;
        for (i = 0; i < n; i++) {
            if (EAGLTestBit(mask, i)) {
                float* o = (float*)(out + i * 0x40);
                int* pp = (int*)((char*)sk + 0x1C + i * 0x70);
                fn_803E00EC(o, m[0], m[1], m[2], m[4], m[5], m[6], m[7], m[8], m[9], m[10]);
                o[0] = o[0] * m[3];
                o[4] = o[4] * m[3];
                o[8] = o[8] * m[3];
                int par = *pp;
                if (par >= 0)
                    lbl_80626DC8(o, out + (par << 6), o);
            }
            m += 12;
        }
    } else {
        int n = sk->m8;
        int i;
        float* m = (float*)a;
        for (i = 0; i < n; i++) {
            float* o = (float*)(out + i * 0x40);
            int* pp = (int*)((char*)sk + 0x1C + i * 0x70);
            fn_803E00EC(o, m[0], m[1], m[2], m[4], m[5], m[6], m[7], m[8], m[9], m[10]);
            o[0] = o[0] * m[3];
            o[4] = o[4] * m[3];
            o[8] = o[8] * m[3];
            int par = *pp;
            if (par >= 0)
                lbl_80626DC8(o, out + (par << 6), o);
            m += 12;
        }
    }
}

extern "C" void fn_803BB21C(EAGLSk* sk, char* a, char* out, unsigned* mask)
{
    if (mask != 0) {
        int n = sk->m8;
        int i;
        PoseSQTToGlobal(sk, a, out, 0);
        for (i = 0; i < n; i++) {
            if (EAGLTestBit(mask, i)) {
                lbl_80626DC8(out + i * 0x40, out + i * 0x40, (char*)sk + 0x40 + i * 0x70);
                fn_803DFB48(out + i * 0x40);
            }
        }
    } else {
        int n = sk->m8;
        int i;
        PoseSQTToGlobal(sk, a, out, 0);
        for (i = 0; i < n; i++) {
            lbl_80626DC8(out + i * 0x40, out + i * 0x40, (char*)sk + 0x40 + i * 0x70);
            fn_803DFB48(out + i * 0x40);
        }
    }
}

struct EAGLSk2 {
    char pad0[8];
    int m8;
    float* mC;
};

extern "C" void GetStillPose(EAGLSk2* sk, float* out, unsigned* mask)
{
    int n = sk->m8;
    int i;
    if (mask != 0) {
        if (sk->mC != 0) {
            for (i = 0; i < n; i++) {
                if (EAGLTestBit(mask, i)) {
                    float* m = (float*)((char*)sk + 0x10 + i * 0x70);
                    out[0] = m[0];
                    out[1] = m[1];
                    out[2] = m[2];
                    out[3] = sk->mC[i];
                    out[4] = m[4];
                    out[5] = m[5];
                    out[6] = m[6];
                    out[7] = m[7];
                    out[8] = m[8];
                    out[9] = m[9];
                    out[10] = m[10];
                    out[11] = 1.0f;
                }
                out += 12;
            }
        } else {
            for (i = 0; i < n; i++) {
                if (EAGLTestBit(mask, i)) {
                    float* m = (float*)((char*)sk + 0x10 + i * 0x70);
                    out[0] = m[0];
                    out[1] = m[1];
                    out[2] = m[2];
                    out[3] = 1.0f;
                    out[4] = m[4];
                    out[5] = m[5];
                    out[6] = m[6];
                    out[7] = m[7];
                    out[8] = m[8];
                    out[9] = m[9];
                    out[10] = m[10];
                    out[11] = 1.0f;
                }
                out += 12;
            }
        }
    } else if (sk->mC != 0) {
        for (i = 0; i < n; i++) {
            float* m = (float*)((char*)sk + 0x10 + i * 0x70);
            out[0] = m[0];
            out[1] = m[1];
            out[2] = m[2];
            out[3] = sk->mC[i];
            out[4] = m[4];
            out[5] = m[5];
            out[6] = m[6];
            out[7] = m[7];
            out[8] = m[8];
            out[9] = m[9];
            out[10] = m[10];
            out[11] = 1.0f;
            out += 12;
        }
    } else {
        for (i = 0; i < n; i++) {
            float* m = (float*)((char*)sk + 0x10 + i * 0x70);
            out[0] = m[0];
            out[1] = m[1];
            out[2] = m[2];
            out[3] = 1.0f;
            out[4] = m[4];
            out[5] = m[5];
            out[6] = m[6];
            out[7] = m[7];
            out[8] = m[8];
            out[9] = m[9];
            out[10] = m[10];
            out[11] = 1.0f;
            out += 12;
        }
    }
}

extern "C" void PoseGlobalToSkin(EAGLSk* sk, char* a, char* out, unsigned* mask)
{
    int n = sk->m8;
    if (mask != 0) {
        int i;
        for (i = 0; i < n; i++) {
            if (EAGLTestBit(mask, i)) {
                lbl_80626DC8(out + i * 0x40, a + i * 0x40, (char*)sk + 0x40 + i * 0x70);
                fn_803DFB48(out + i * 0x40);
            }
        }
    } else {
        int i;
        for (i = 0; i < n; i++) {
            lbl_80626DC8(out + i * 0x40, a + i * 0x40, (char*)sk + 0x40 + i * 0x70);
            fn_803DFB48(out + i * 0x40);
        }
    }
}

extern "C" void fn_803BB708(char* base, int idx, float* out)
{
    int off = idx * 0x70 + 0x10;
    float* m = (float*)(base + off);
    out[0] = m[0];
    out[1] = m[1];
    out[2] = m[2];
    out[3] = 1.0f;
    out[4] = m[4];
    out[5] = m[5];
    out[6] = m[6];
    out[7] = m[7];
    out[8] = m[8];
    out[9] = m[9];
    out[10] = m[10];
    out[11] = 1.0f;
}

extern "C" void fn_803BB778(char* base, int idx, void* b)
{
    PostMult((char*)(idx * 0x70 + (int)base) + 0x40, b);
}

extern "C" void fn_803BCE00(DtorAt0* s, int chrg)
{
    s->m = lbl_80685268;
    if (chrg & 1)
        fn_803BCE38(s, 0xa0);
}

extern "C" void fn_803BCE38(void* p, int n)
{
    lbl_8062C66C(p, n);
}

extern "C" void fn_803BD1F4(EAGLNameTab* t)
{
    if (t->m4 != 0) {
        int i;
        for (i = t->m2 - 1; i >= 0; i--) {
            const char* nm = t->m4[i].name;
            if (nm != 0)
                lbl_8062C66C((void*)nm, strlen(nm) + 1);
        }
        lbl_8062C66C(t->m4, t->mC * 8);
    }
    if (t->m8 != 0)
        lbl_8062C66C(t->m8, t->mC * 4);
}

extern "C" int fn_803BD2B4(EAGLNameTab* t, const char* name, int* ins)
{
    *ins = 0;
    int hi = t->m2 - 1;
    while (*ins <= hi) {
        int mid = (*ins + hi) >> 1;
        int c = strcmp(name, t->m4[mid].name);
        if (c > 0)
            *ins = mid + 1;
        else if (c < 0)
            hi = mid - 1;
        else
            return t->m4[mid].val;
    }
    return -1;
}

struct Mtx4B { float m[16]; };

extern "C" void Multiply(Mtx4B* out, Mtx4B* a, Mtx4B* b)
{
    Mtx4B t;
    t.m[0] = b->m[0] * a->m[0] + b->m[1] * a->m[4] + b->m[2] * a->m[8] + b->m[3] * a->m[12];
    t.m[1] = b->m[0] * a->m[1] + b->m[1] * a->m[5] + b->m[2] * a->m[9] + b->m[3] * a->m[13];
    t.m[2] = b->m[0] * a->m[2] + b->m[1] * a->m[6] + b->m[2] * a->m[10] + b->m[3] * a->m[14];
    t.m[3] = b->m[0] * a->m[3] + b->m[1] * a->m[7] + b->m[2] * a->m[11] + b->m[3] * a->m[15];
    t.m[4] = b->m[4] * a->m[0] + b->m[5] * a->m[4] + b->m[6] * a->m[8] + b->m[7] * a->m[12];
    t.m[5] = b->m[4] * a->m[1] + b->m[5] * a->m[5] + b->m[6] * a->m[9] + b->m[7] * a->m[13];
    t.m[6] = b->m[4] * a->m[2] + b->m[5] * a->m[6] + b->m[6] * a->m[10] + b->m[7] * a->m[14];
    t.m[7] = b->m[4] * a->m[3] + b->m[5] * a->m[7] + b->m[6] * a->m[11] + b->m[7] * a->m[15];
    t.m[8] = b->m[8] * a->m[0] + b->m[9] * a->m[4] + b->m[10] * a->m[8] + b->m[11] * a->m[12];
    t.m[9] = b->m[8] * a->m[1] + b->m[9] * a->m[5] + b->m[10] * a->m[9] + b->m[11] * a->m[13];
    t.m[10] = b->m[8] * a->m[2] + b->m[9] * a->m[6] + b->m[10] * a->m[10] + b->m[11] * a->m[14];
    t.m[11] = b->m[8] * a->m[3] + b->m[9] * a->m[7] + b->m[10] * a->m[11] + b->m[11] * a->m[15];
    t.m[12] = b->m[12] * a->m[0] + b->m[13] * a->m[4] + b->m[14] * a->m[8] + b->m[15] * a->m[12];
    t.m[13] = b->m[12] * a->m[1] + b->m[13] * a->m[5] + b->m[14] * a->m[9] + b->m[15] * a->m[13];
    t.m[14] = b->m[12] * a->m[2] + b->m[13] * a->m[6] + b->m[14] * a->m[10] + b->m[15] * a->m[14];
    t.m[15] = b->m[12] * a->m[3] + b->m[13] * a->m[7] + b->m[14] * a->m[11] + b->m[15] * a->m[15];
    *out = t;
}

extern "C" void InitInternal(int arg)
{
    void* (**al)(int, const char*) = &lbl_8062C668;
    EAGLDelObj** pcc = &lbl_80626DCC;
    DtorAt0* o = (DtorAt0*)(*al)(4, 0);
    o->m = lbl_80685688;
    *pcc = (EAGLDelObj*)o;
    if (lbl_80626DD0 == 0)
        lbl_80626DD0 = (EAGLHeapY*)o;
    lbl_80626DD0->v2(arg);
    void** p84 = &lbl_80627284;
    *p84 = (*al)(1, 0);
    fn_803E70B0(lbl_806AAB80, lbl_8060D1B4, Constructor, fn_803BE7B0);
}

extern "C" int fn_803BD6E4()
{
    return lbl_80626DD0->v4();
}

extern "C" void fn_803BD720()
{
    lbl_80626DD0->v14();
}

extern "C" void fn_803BD75C()
{
    fn_803E7074(lbl_806AAB80, lbl_8060D1B4);
    fn_803BD8BC();
    if (lbl_80627284 != 0)
        lbl_8062C66C(lbl_80627284, 1);
    lbl_80626DD0->v3();
    if (lbl_80626DCC != 0)
        lbl_80626DCC->v1(3);
}

extern "C" void fn_803BD800(int a, int b)
{
    if (b != 0xFFFF)
        return;
    if (a == 0)
        return;
    int i;
    for (i = 0; i < 3; i++) {
        lbl_806AA8BC[i].m0 = 0;
        lbl_806AA8BC[i].m4 = 0;
        lbl_806AA8BC[i].m8 = 0;
    }
}

extern "C" void FreeBuffer(EAGLSlot* e)
{
    e->m8--;
    if (e->m0 != 0 && e->m8 <= 0) {
        lbl_8062C66C(e->m0, e->m4);
        e->m0 = 0;
    }
}

extern "C" EAGLSlot* fn_803BD8A8(int i)
{
    return &lbl_806AA8BC[i];
}

extern "C" void fn_803BD8BC()
{
    int i;
    for (i = 0; i < 3; i++) {
        if (lbl_806AA8BC[i].m0 != 0)
            lbl_8062C66C(lbl_806AA8BC[i].m0, lbl_806AA8BC[i].m4);
    }
}

extern "C" void fn_803BD918()
{
    fn_803BD800(1, 0xFFFF);
}
