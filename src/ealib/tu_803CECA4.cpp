/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803CECA4-0x803D2B20 (code-only unit, 44 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

/* Data record at +0xC of the objects. Evidence: fn_803CECA4 (mC), fn_803CFC00 (m4), fn_803CFC3C (m6, m8 flags). */
struct Unk803CECA4Rec {
    int m0;
    unsigned short m4;
    unsigned short m6;
    unsigned char m8;
    char pad9[3];
    int mC;
};

/* Object: vptr at +0, record pointer at +0xC. Evidence: fn_803CFC3C (m10, m12, m14, m18 stores). */
struct Unk803CECA4Obj {
    const void* m0;
    int m4;
    int m8;
    Unk803CECA4Rec* mC;
    int m10;
};

/* Second view of the object (fn_803CFC3C). */
struct Unk803CECA4Obj3 {
    const void* m0;
    int m4;
    int m8;
    Unk803CECA4Rec* mC;
    short m10;
    unsigned short m12;
    int m14;
    unsigned char m18;
};

/* Stack object built and torn down by fn_803CF2EC / fn_803CF37C (ctor fn_803D3754 / fn_803D4818, dtor fn_803D37A0 / fn_803D485C). */
struct Unk803CECA4Tmp {
    const void* m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
};

class Unk803CECA4Vt {
public:
    virtual ~Unk803CECA4Vt();
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
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19(void* p);
};

class Unk80626DD0 {
public:
    virtual ~Unk80626DD0();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7(void* p);
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12(void* p);
};

struct Unk803D1730Obj {
    const void* m0;
    int m4;
    int m8;
    void* mC;
    void* m10;
    void* m14;
    void* m18;
    void* m1C;
    void* m20;
    void* m24;
    float m28;
    void* m2C;
    int m30;
    char pad34[0x14];
    float m48;
    float m4C;
    float m50;
    int m54;
    char pad58[0x10];
    void* m68;
    char pad6C[0x10];
    void* m7C;
};

struct Unk803D1A30Rec {
    char pad0[4];
    unsigned short m4;
};

struct Unk803D1A30Obj {
    char pad0[0x10];
    Unk803D1A30Rec** m10;
    char pad14[0x1C];
    int m30;
};

/* Object built by fn_803D2A14 / fn_803D1730 (vtables lbl_80686478 / lbl_806863D8). */
struct Unk803D2A14Obj {
    const void* m0;
    int m4;
    int m8;
    void* mC;
    void* m10;
    void* m14;
    float m18;
    void* m1C;
    int m20;
    char pad24[0x14];
    float m38;
    float m3C;
    float m40;
    int m44;
    char pad48[0x10];
    void* m58;
};

struct Unk803D1900Quat {
    float x, y, z, w;
};

struct Unk803D1900Self {
    char pad0[0x58];
    Unk803D1900Quat q;
};

struct Unk803D1900Other {
    char pad0[0x10];
    Unk803D1900Quat q;
};

inline void QMul(Unk803D1900Quat* t, Unk803D1900Quat* a, Unk803D1900Quat* b)
{
    t->x = b->w * a->x + (b->z * a->y + (b->x * a->w - b->y * a->z));
    t->y = b->w * a->y + ((b->x * a->z + b->y * a->w) - b->z * a->x);
    t->z = b->w * a->z + (b->z * a->w + (-b->x * a->y + b->y * a->x));
    t->w = ((-b->x * a->x - b->y * a->y) - b->z * a->z) + b->w * a->w;
}

struct Unk803D0EE4Vec4 {
    float x, y, z, w;
};

struct Unk803D0EE4Self {
    char pad0[0x58];
    Unk803D1900Quat q;
};

struct Unk803D240CSelf {
    char pad0[0x48];
    Unk803D1900Quat q;
};

inline void RotVec(Unk803D0EE4Vec4* out, Unk803D0EE4Vec4* v, Unk803D1900Quat* q)
{
    float y2 = q->y + q->y;
    float z2 = q->z + q->z;
    float xy = q->x * y2;
    float zz = q->z * z2;
    float xz = q->x * z2;
    float wy = q->w * y2;
    float x2 = q->x + q->x;
    float wz = q->w * z2;
    float yz = q->y * z2;
    float wx = q->w * x2;
    float xx = q->x * x2;
    float yy = q->y * y2;
    out->x = v->z * (xz + wy) + (v->x * (1.0f - (yy + zz)) + v->y * (xy - wz));
    out->y = v->z * (yz - wx) + (v->x * (xy + wz) + v->y * (1.0f - (xx + zz)));
    out->z = (v->x * (xz - wy) + v->y * (yz + wx)) + v->z * (1.0f - (xx + yy));
    out->w = 1.0f;
}

struct Unk803D0FFCVec2 {
    float x, y;
};

class Unk803D0FFCChan {
public:
    virtual ~Unk803D0FFCChan();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual int v9(Unk803D0FFCVec2* out, float t);
};

struct Unk803D0FFCObj {
    char pad0[0x20];
    Unk803D0FFCChan* m20;
    Unk803D0FFCChan* m24;
    float m28;
};

struct Unk803D1FE8Obj {
    char pad0[0x10];
    Unk803D0FFCChan* m10;
    Unk803D0FFCChan* m14;
    float m18;
};

extern "C" float sqrtf(float x);

extern const void* lbl_80686478[];
extern const void* lbl_806863D8[];
extern const void* lbl_80685268[];
extern Unk80626DD0* lbl_80626DD0;
extern void (*lbl_8062C66C)(void* p, int size);

extern "C" {
void fn_803D14E8(Unk803D1A30Obj* self, float* out, int a, int b, float x, float y);
}

extern "C" {
Unk803CECA4Tmp* fn_803D3754(Unk803CECA4Tmp* p);
void fn_803D37A0(Unk803CECA4Tmp* p, int c);
Unk803CECA4Tmp* fn_803D4818(Unk803CECA4Tmp* p);
void fn_803D485C(Unk803CECA4Tmp* p, int c);
void* fn_803BD8A8(int a);
void FreeBuffer(void* p);
void fn_803CEECC(Unk803CECA4Rec* rec, int a, int b, int c, float t);
}

extern "C" int fn_803CECA4(Unk803CECA4Obj* self, float* out)
{
    *out = (float)self->mC->mC;
    return 1;
}

extern "C" void fn_803CECE4(Unk803CECA4Obj* self, int a, float x, float y)
{
    fn_803CEECC((Unk803CECA4Rec*)self->mC, a, self->m10, 0, y);
}

extern "C" int EvalSQT(Unk803CECA4Obj* self, int a, int b, float t)
{
    fn_803CEECC((Unk803CECA4Rec*)self->mC, a, self->m10, b, t);
    return 1;
}

struct Unk80364310;
void fn_80364310(float** pp, float* out);
void fn_80364358(float** pp, Unk80364310* o);
extern "C" {
void fn_803641CC();
void fn_80364390();
void fn_803646DC();
void fn_803648C0();
int printf(const char* fmt, ...);
}

struct Unk803CED44Obj {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10[1];
};

extern "C" void fn_803CED44(Unk803CED44Obj* s)
{
    int i = 0;
    int n = s->m4;
    int* p = s->m10;
    while (i < n) {
        int c = *p;
        i++;
        p++;
        if (c > 0) {
            int j = c;
            do {
                int v = *p;
                i++;
                switch (v) {
                case 1:
                    *p = (int)fn_803641CC;
                    break;
                case 0:
                    *p = (int)fn_80364310;
                    break;
                case 2:
                    *p = (int)fn_80364358;
                    break;
                default:
                    printf("Bad signature channel type\n");
                    break;
                }
                p++;
                j--;
            } while (j != 0);
        }
    }
    int* q = s->m10;
    p = q + s->m4;
    i = 0;
    while (i < n) {
        int c = *p;
        i++;
        p++;
        if (c > 0) {
            int j = c;
            do {
                int v = *p;
                i++;
                switch (v) {
                case 1:
                    *p = (int)fn_80364390;
                    break;
                case 0:
                    *p = (int)fn_803646DC;
                    break;
                case 2:
                    *p = (int)fn_803648C0;
                    break;
                default:
                    printf("Bad signature channel type\n");
                    break;
                }
                p++;
                j--;
            } while (j != 0);
        }
    }
}

extern "C" void fn_803CF2EC(char* self)
{
    Unk803CECA4Tmp o;
    fn_803D3754(&o);
    *(const void**)(self - 0x18) = o.m0;
    fn_803D37A0(&o, 2);
    char* p = *(char**)(self + 0x10);
    if (p) {
        char* base = p - 0x18;
        fn_803D4818(&o);
        *(const void**)(p - 0x18) = o.m0;
        fn_803D485C(&o, 2);
        ((Unk803CECA4Vt*)base)->v19(p);
        *(char**)(self + 0x10) = base;
    }
}

extern "C" void fn_803CF37C(char* self)
{
    Unk803CECA4Tmp o;
    fn_803D4818(&o);
    *(const void**)(self - 0x18) = o.m0;
    fn_803D485C(&o, 2);
}

extern "C" int fn_803CFBF8(Unk803CECA4Obj* self)
{
    return (int)self->mC;
}

extern "C" int fn_803CFC00(Unk803CECA4Obj* self, float* out)
{
    *out = (float)self->mC->m4;
    return 1;
}

extern "C" void fn_803CFC3C(Unk803CECA4Obj3* self, Unk803CECA4Rec* r)
{
    self->mC = r;
    self->m10 = 0;
    self->m12 = r->m6;
    self->m14 = (r->m8 & 1) ^ 1;
    int f = r->m8;
    unsigned char v;
    if (f & 8) v = 1;
    else if (f & 0x10) v = 2;
    else if (f & 0x20) v = 4;
    else if (f & 0x40) v = 8;
    else v = 1;
    self->m18 = v;
}

extern "C" void fn_803CFCAC(Unk803D1730Obj* self, int c)
{
    self->m0 = lbl_806863D8;
    if (self->m18)
        lbl_80626DD0->v12(self->m18);
    if (self->m1C)
        lbl_80626DD0->v12(self->m1C);
    if (self->m20)
        lbl_80626DD0->v12(self->m20);
    if (self->m24)
        lbl_80626DD0->v12(self->m24);
    if (self->m2C) {
        FreeBuffer(fn_803BD8A8(0));
    }
    if (self->mC)
        lbl_80626DD0->v7(self->mC);
    if (self->m10)
        lbl_80626DD0->v7(self->m10);
    if (self->m14)
        lbl_80626DD0->v7(self->m14);
    if (self->m7C)
        lbl_80626DD0->v7(self->m7C);
    self->m0 = lbl_80685268;
    if (c & 1)
        lbl_8062C66C(self, 0x80);
}

extern "C" void fn_803D0EE4(Unk803D0EE4Self* self, float* v2)
{
    Unk803D0EE4Vec4 in;
    in.z = v2[1];
    in.y = 0.0f;
    in.x = v2[0];
    in.w = 1.0f;
    Unk803D0EE4Vec4 out;
    RotVec(&out, &in, &self->q);
    v2[0] = out.x;
    v2[1] = out.z;
}

extern "C" int fn_803D0FFC(Unk803D0FFCObj* self, Unk803D0FFCVec2* out, float t0, float t1)
{
    Unk803D0FFCVec2 a;
    Unk803D0FFCVec2 b;
    if (!self->m20->v9(&a, t0))
        return 0;
    const float zero = 0.0f;
    if (self->m28 != zero) {
        if (!self->m24->v9(&b, t1))
            return 0;
        float s = 1.0f - self->m28;
        out->x = self->m28 * b.x + s * a.x;
        out->y = self->m28 * b.y + s * a.y;
        float len = sqrtf(out->x * out->x + out->y * out->y);
        if (len != zero) {
            float lb = sqrtf(b.x * b.x + b.y * b.y);
            float la = sqrtf(a.x * a.x + a.y * a.y);
            float k = (self->m28 * lb + s * la) / len;
            out->x = out->x * k;
            out->y = out->y * k;
        }
    } else {
        out->y = a.y;
        out->x = a.x;
    }
    return 1;
}

extern "C" Unk803D1730Obj* fn_803D1730(Unk803D1730Obj* self)
{
    self->m0 = lbl_806863D8;
    self->m4 = 0;
    self->mC = 0;
    self->m10 = 0;
    self->m14 = 0;
    self->m28 = 0.0f;
    self->m2C = 0;
    self->m30 = -100;
    self->m48 = 1.0f;
    self->m4C = 0.0f;
    self->m50 = 0.0f;
    self->m54 = -100;
    self->m68 = 0;
    self->m7C = 0;
    self->m18 = 0;
    self->m1C = 0;
    self->m20 = 0;
    self->m24 = 0;
    self->m8 = 8;
    return self;
}

extern "C" void fn_803D17A8(Unk803CECA4Vt* self, int a, float x, float y)
{
    self->v7(a, y, 0);
}

extern "C" float fn_803D17E4(float x, float lo, float hi)
{
    float range = hi - lo;
    float t;
    int k;
    if (x < lo) {
        t = lo - x;
        float m;
        k = (int)(t / range);
        m = t - (float)k * range;
        return hi - m;
    }
    if (x >= hi) {
        t = x - hi;
        float m;
        k = (int)(t / range);
        m = t - (float)k * range;
        return lo + m;
    }
    return x;
}

extern "C" int fn_803D189C(float x, float lo, float hi)
{
    float range = hi - lo;
    float t;
    if (x < lo) {
        t = lo - x;
        return (int)(t / range);
    }
    if (x >= hi) {
        t = x - hi;
        return (int)(t / range) + 1;
    }
    return 0;
}

extern "C" int fn_803D18F8()
{
    return 0;
}

extern "C" void fn_803D1900(Unk803D1900Self* self, Unk803D1900Other* other)
{
    Unk803D1900Quat t;
    QMul(&t, &self->q, &other->q);
    other->q = t;
}

extern "C" float fn_803D19FC(char* self)
{
    return *(float*)(self + 0x48);
}

extern "C" void fn_803D1A04(Unk803D1A30Obj* self, float* out)
{
    ((void (*)(Unk803D1A30Obj*, float*, float, float))fn_803D14E8)(self, out, 0.0f, 0.0f);
}

struct Unk803D1E54Obj {
    char pad0[0x10];
    void* m10;
    char pad14[0x14];
    float m28;
    float m2C;
    float m30;
    float m34;
    float m38;
    float m3C;
    float m40;
};

extern "C" {
int printf(const char* fmt, ...);
int fn_803D2B64(Unk803D1E54Obj* s, float t, float lo, float hi);
float fn_803D2CBC(Unk803D1E54Obj* s, float t, float lo, float hi);
void fn_803D1C78(Unk803D1E54Obj* s, float t);
void fn_803D2170(Unk803D1E54Obj* s, int n);
int fn_803D1FE8(Unk803D1FE8Obj* self, Unk803D0FFCVec2* out, float t0, float t1);
void fn_803D240C(Unk803D240CSelf* self, float* v2);
}

/* near-miss (99.4%, 7 slots): retail multiplies m38*m28 before m38*m2C (f13/f0/f12 swapped); the scheduler here
   issues the m2C product first. t, c1 and the x/y results are updated in place (two callee-saved floats). */
extern "C" int fn_803D1E54(Unk803D1E54Obj* s, Unk803D0FFCVec2* out, float t)
{
    s->m3C = t;
    if (s->m10 == 0)
        fn_803D1C78(s, 0.0f);
    t += s->m40;
    int n = fn_803D2B64(s, t, 0.0f, 2.0f / s->m38);
    printf("currTime: %g  offset: %g   cycle: %g\n", t, s->m40, 1.0f / s->m38);
    printf("offset0: %g  offset1: %g\n", s->m30, s->m34);
    printf("cycle0: %g  cycle1: %g\n", s->m28, s->m2C);
    float c1 = s->m38 * s->m2C * t;
    t = s->m38 * s->m28 * t;
    printf("before offset t0: %g  t1: %g
", t, c1);
    t = fn_803D2CBC(s, t, 0.0f, 2.0f * s->m28) - s->m30;
    c1 = fn_803D2CBC(s, c1, 0.0f, 2.0f * s->m2C) - s->m34;
    if (fn_803D1FE8((Unk803D1FE8Obj*)s, out, t, c1)) {
        fn_803D2170(s, n);
        fn_803D240C((Unk803D240CSelf*)s, (float*)out);
        return 1;
    }
    return 0;
}

extern "C" int fn_803D1FE8(Unk803D1FE8Obj* self, Unk803D0FFCVec2* out, float t0, float t1)
{
    Unk803D0FFCVec2 a;
    Unk803D0FFCVec2 b;
    if (!self->m10->v9(&a, t0))
        return 0;
    const float zero = 0.0f;
    if (self->m18 != zero) {
        if (!self->m14->v9(&b, t1))
            return 0;
        float s = 1.0f - self->m18;
        out->x = self->m18 * b.x + s * a.x;
        out->y = self->m18 * b.y + s * a.y;
        float len = sqrtf(out->x * out->x + out->y * out->y);
        if (len != zero) {
            float lb = sqrtf(b.x * b.x + b.y * b.y);
            float la = sqrtf(a.x * a.x + a.y * a.y);
            float k = (self->m18 * lb + s * la) / len;
            out->x = out->x * k;
            out->y = out->y * k;
        }
    } else {
        out->y = a.y;
        out->x = a.x;
    }
    return 1;
}

extern "C" void fn_803D240C(Unk803D240CSelf* self, float* v2)
{
    Unk803D0EE4Vec4 in;
    in.z = v2[1];
    in.y = 0.0f;
    in.x = v2[0];
    in.w = 1.0f;
    Unk803D0EE4Vec4 out;
    RotVec(&out, &in, &self->q);
    v2[0] = out.x;
    v2[1] = out.z;
}

extern "C" Unk803D2A14Obj* fn_803D2A14(Unk803D2A14Obj* self)
{
    self->m0 = lbl_80686478;
    self->m4 = 0;
    self->mC = 0;
    self->m18 = 0.0f;
    self->m1C = 0;
    self->m20 = -100;
    self->m38 = 1.0f;
    self->m3C = 0.0f;
    self->m40 = 0.0f;
    self->m44 = -100;
    self->m58 = 0;
    self->m10 = 0;
    self->m14 = 0;
    self->m8 = 9;
    return self;
}

extern "C" void fn_803D2A78(Unk803D2A14Obj* self, int c)
{
    self->m0 = lbl_80686478;
    if (self->m1C) {
        FreeBuffer(fn_803BD8A8(1));
    }
    if (self->mC) {
        lbl_80626DD0->v7(self->mC);
    }
    self->m0 = lbl_80685268;
    if (c & 1)
        lbl_8062C66C(self, 0x5c);
}

/*
 * Not matched yet: fn_803D1A30 (retail passes the 5th argument register on as the 4th
 * argument of fn_803D14E8 without setting the 3rd), fn_803CEECC, fn_803CF14C, fn_803CF3C0,
 * fn_803CF5F4, fn_803CF77C, fn_803CF968, fn_803CFE8C, fn_803D00DC, fn_803D062C, fn_803D0944, fn_803D0C0C,
 * fn_803D1184, fn_803D14E8, fn_803D1AB8, fn_803D1C78, fn_803D1E54, fn_803D2170, fn_803D2524, fn_803D279C.
 * The original code stays linked for these.
 */
