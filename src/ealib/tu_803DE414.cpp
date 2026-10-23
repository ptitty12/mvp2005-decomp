/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803DE414-0x803E097C (code-only unit, 44 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 *
 * 4x4 float matrices (0x40 bytes, row-major, translation in the last row), plus the texture-size
 * query and the two light-like classes derived from the objects in tu_803D8234.
 */

struct Mtx4 {
    float m[16];
};

struct Unk803DFFE0Vec3 {
    float x, y, z;
};

struct Unk803DEE7CVec4 {
    float x, y, z, w;
};

/* Texture wrapper (state pointer at +0x54); the info record has width at +4, height at +6. */
struct Unk803DE414Info {
    char pad0[4];
    short m4;
    short m6;
};
struct Unk803DE414Tex {
    char pad0[0x54];
    void* m54;
    char pad58[0x6C - 0x58];
    int m6C;
};

/* Derived light object: base init fn_803D9B6C, vptr at +0x1C, owner at +0x20. */
struct Unk803DE414Light {
    char pad0[0x1C];
    const void* m1C;
    void* m20;
    int m24;
    int m28;
};

/* Derived object with vptr at +0x10 (base init fn_803D99C8). */
struct Unk803DE414Obj {
    char pad0[0x10];
    const void* m10;
    void* m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
    int m30;
    int m34;
};

/* Screen-size source: three data words, then the vptr at +0xC; slot 3 returns width and height. */
struct Unk803E0690SrcData {
    int m0;
    int m4;
    int m8;
};
struct Unk803E0690Src : Unk803E0690SrcData {
    virtual void v1();
    virtual void v2();
    virtual void v3(float* w, float* h);
};
struct Unk803E0690Obj {
    char pad0[0x14];
    Unk803E0690Src* m14;
};
int fn_803D81FC();
int fn_803D817C();

struct Unk803E032CSub {
    char pad0[0x14];
    Unk803E0690Src* m14;
};
struct Unk803E032CObj {
    char pad0[0x68];
    int m68;
    int m6C;
    int m70;
};
extern int lbl_80627A80;
void fn_803D81B4(int unused, int v);
extern "C" {
int fn_803E0898(Unk803DE414Light* self);
void GXInvalidateVtxCache(void);
void GXInvalidateTexAll(void);
void GXSetPixelFmt(int fmt, int z);
void GXSetDither(int dither);
}

extern "C" {
float sqrtf(float x);
void MultMatrix(Mtx4* a, Mtx4* b, Mtx4* out);
void MEM_copy(void* dst, const void* src, int n);
void fn_803DE7D0(Mtx4* a, Mtx4* b);
void fn_803DF4E4(Mtx4* out);
void fn_803E02E4(Mtx4* dst, Mtx4* other);
void* fn_803DD7C0(void* t);
void fn_803DB4CC(void);
void* fn_803D9B6C(void* self, void* owner);
void fn_803D9BAC(void* self, int c);
void* fn_803D99C8(void* self, void* owner);
void fn_803D99F0(void* self, int c);
void fn_803E0658(int a, int b);
}

extern int lbl_8062C270[];
extern int lbl_80627A84;
extern int lbl_80627324;
extern const void* lbl_80686848[];
extern const void* lbl_80686830[];

/* 803DE000 */
inline void MemCopy(void* d, const void* src, int n)
{
    MEM_copy(d, src, n);
}

/* 803DE000 */
inline void MtxDiag(Mtx4* s, float x, float y, float z, float w)
{
    s->m[0] = x;
    s->m[1] = 0.0f;
    s->m[2] = 0.0f;
    s->m[3] = 0.0f;
    s->m[4] = 0.0f;
    s->m[5] = y;
    s->m[6] = 0.0f;
    s->m[7] = 0.0f;
    s->m[8] = 0.0f;
    s->m[9] = 0.0f;
    s->m[10] = z;
    s->m[11] = 0.0f;
    s->m[12] = 0.0f;
    s->m[13] = 0.0f;
    s->m[14] = 0.0f;
    s->m[15] = w;
}

/* 803DE000 */
inline void MtxTrans(Mtx4* m, float x, float y, float z)
{
    m->m[0] = 1.0f;
    m->m[1] = 0.0f;
    m->m[2] = 0.0f;
    m->m[3] = 0.0f;
    m->m[4] = 0.0f;
    m->m[5] = 1.0f;
    m->m[6] = 0.0f;
    m->m[7] = 0.0f;
    m->m[8] = 0.0f;
    m->m[9] = 0.0f;
    m->m[10] = 1.0f;
    m->m[11] = 0.0f;
    m->m[12] = x;
    m->m[13] = y;
    m->m[14] = z;
    m->m[15] = 1.0f;
}

/* near-miss (98.6%): matrix to quaternion through a 3x3 stack copy read via a saved pointer. Retail keeps the
   axis-selection index in r0 (li after the compare) and loads the y-branch operands in a different order;
   ours puts the index in r9. Store order per branch was permuted exhaustively; index type/ternary variants tried. */
extern "C" void fn_803DE51C(Mtx4* m, float* q, float* trans)
{
    float t[9];
    float* p = t;
    float a = m->m[0];
    t[0] = a;
    t[1] = m->m[1];
    t[2] = m->m[2];
    t[3] = m->m[4];
    t[4] = m->m[5];
    t[5] = m->m[6];
    t[6] = m->m[8];
    t[7] = m->m[9];
    t[8] = m->m[10];
    const float zero = 0.0f;
    float s = a + p[4] + p[8];
    if (s > zero) {
        s = sqrtf(s + 1.0f);
        q[3] = s * 0.5f;
        s = 0.5f / s;
        q[0] = (p[5] - p[7]) * s;
        q[1] = (p[6] - p[2]) * s;
        q[2] = (p[1] - p[3]) * s;
    } else {
        int i = 0;
        if (p[4] > a)
            i = 4;
        if (p[8] > p[i]) {
            s = sqrtf(p[8] - (a + p[4]) + 1.0f);
            q[2] = s * 0.5f;
            if (s != zero)
                s = 0.5f / s;
            q[3] = (p[1] - p[3]) * s;
            q[0] = (p[6] + p[2]) * s;
            q[1] = (p[7] + p[5]) * s;
        } else if (i != 0) {
            s = sqrtf(p[4] - (p[8] + a) + 1.0f);
            q[1] = s * 0.5f;
            if (s != zero)
                s = 0.5f / s;
            q[3] = (p[6] - p[2]) * s;
            q[0] = (p[3] + p[1]) * s;
            q[2] = (p[5] + p[7]) * s;
        } else {
            s = sqrtf(a - (p[4] + p[8]) + 1.0f);
            q[0] = s * 0.5f;
            if (s != zero)
                s = 0.5f / s;
            q[3] = (p[5] - p[7]) * s;
            q[1] = (p[1] + p[3]) * s;
            q[2] = (p[2] + p[6]) * s;
        }
    }
    trans[0] = m->m[12];
    trans[1] = m->m[13];
    trans[2] = m->m[14];
    trans[3] = m->m[15];
}

extern "C" void fn_803DED40(Mtx4* m, Unk803DFFE0Vec3* in, Unk803DFFE0Vec3* out)
{
    if (in != out) {
        out->x = in->x * m->m[0] + in->y * m->m[4] + in->z * m->m[8] + m->m[12];
        out->y = in->x * m->m[1] + in->y * m->m[5] + in->z * m->m[9] + m->m[13];
        out->z = in->x * m->m[2] + in->y * m->m[6] + in->z * m->m[10] + m->m[14];
    } else {
        Unk803DFFE0Vec3 t;
        t.x = in->x * m->m[0] + in->y * m->m[4] + in->z * m->m[8] + m->m[12];
        t.y = in->x * m->m[1] + in->y * m->m[5] + in->z * m->m[9] + m->m[13];
        t.z = in->x * m->m[2] + in->y * m->m[6] + in->z * m->m[10] + m->m[14];
        *in = t;
    }
}

extern "C" void fn_803DEE7C(Mtx4* m, Unk803DEE7CVec4* in, Unk803DEE7CVec4* out)
{
    if (in != out) {
        out->x = in->x * m->m[0] + in->y * m->m[4] + in->z * m->m[8] + in->w * m->m[12];
        out->y = in->x * m->m[1] + in->y * m->m[5] + in->z * m->m[9] + in->w * m->m[13];
        out->z = in->x * m->m[2] + in->y * m->m[6] + in->z * m->m[10] + in->w * m->m[14];
        out->w = in->x * m->m[3] + in->y * m->m[7] + in->z * m->m[11] + in->w * m->m[15];
    } else {
        Unk803DEE7CVec4 t;
        t.x = in->x * m->m[0] + in->y * m->m[4] + in->z * m->m[8] + in->w * m->m[12];
        t.y = in->x * m->m[1] + in->y * m->m[5] + in->z * m->m[9] + in->w * m->m[13];
        t.z = in->x * m->m[2] + in->y * m->m[6] + in->z * m->m[10] + in->w * m->m[14];
        t.w = in->x * m->m[3] + in->y * m->m[7] + in->z * m->m[11] + in->w * m->m[15];
        *in = t;
    }
}

extern "C" void fn_803DF028(Mtx4* m, int count, char* in, int inStride, char* out, int outStride)
{
    if (count != 0) {
        if (inStride == 0)
            inStride = 12;
        if (outStride == 0)
            outStride = 12;
        if (in == out) {
            do {
                Unk803DFFE0Vec3* p = (Unk803DFFE0Vec3*)in;
                Unk803DFFE0Vec3 t;
                t.x = p->x * m->m[0] + p->y * m->m[4] + p->z * m->m[8] + m->m[12];
                t.y = p->x * m->m[1] + p->y * m->m[5] + p->z * m->m[9] + m->m[13];
                t.z = p->x * m->m[2] + p->y * m->m[6] + p->z * m->m[10] + m->m[14];
                in += inStride;
                *(Unk803DFFE0Vec3*)out = t;
                out += outStride;
            } while (--count != 0);
        } else {
            do {
                Unk803DFFE0Vec3* p = (Unk803DFFE0Vec3*)in;
                Unk803DFFE0Vec3* q = (Unk803DFFE0Vec3*)out;
                q->x = p->x * m->m[0] + p->y * m->m[4] + p->z * m->m[8] + m->m[12];
                q->y = p->x * m->m[1] + p->y * m->m[5] + p->z * m->m[9] + m->m[13];
                q->z = p->x * m->m[2] + p->y * m->m[6] + p->z * m->m[10] + m->m[14];
                in += inStride;
                out += outStride;
            } while (--count != 0);
        }
    }
}

/* 803DE000 */
inline void MtxMul(Mtx4* a, Mtx4* b, Mtx4* out)
{
    MultMatrix(a, b, out);
}

extern "C" void MultMatrix(Mtx4* a, Mtx4* b, Mtx4* out)
{
    out->m[0] = a->m[0] * b->m[0] + a->m[1] * b->m[4] + a->m[2] * b->m[8] + a->m[3] * b->m[12];
    out->m[1] = a->m[0] * b->m[1] + a->m[1] * b->m[5] + a->m[2] * b->m[9] + a->m[3] * b->m[13];
    out->m[2] = a->m[0] * b->m[2] + a->m[1] * b->m[6] + a->m[2] * b->m[10] + a->m[3] * b->m[14];
    out->m[3] = a->m[0] * b->m[3] + a->m[1] * b->m[7] + a->m[2] * b->m[11] + a->m[3] * b->m[15];
    out->m[4] = a->m[4] * b->m[0] + a->m[5] * b->m[4] + a->m[6] * b->m[8] + a->m[7] * b->m[12];
    out->m[5] = a->m[4] * b->m[1] + a->m[5] * b->m[5] + a->m[6] * b->m[9] + a->m[7] * b->m[13];
    out->m[6] = a->m[4] * b->m[2] + a->m[5] * b->m[6] + a->m[6] * b->m[10] + a->m[7] * b->m[14];
    out->m[7] = a->m[4] * b->m[3] + a->m[5] * b->m[7] + a->m[6] * b->m[11] + a->m[7] * b->m[15];
    out->m[8] = a->m[8] * b->m[0] + a->m[9] * b->m[4] + a->m[10] * b->m[8] + a->m[11] * b->m[12];
    out->m[9] = a->m[8] * b->m[1] + a->m[9] * b->m[5] + a->m[10] * b->m[9] + a->m[11] * b->m[13];
    out->m[10] = a->m[8] * b->m[2] + a->m[9] * b->m[6] + a->m[10] * b->m[10] + a->m[11] * b->m[14];
    out->m[11] = a->m[8] * b->m[3] + a->m[9] * b->m[7] + a->m[10] * b->m[11] + a->m[11] * b->m[15];
    out->m[12] = a->m[12] * b->m[0] + a->m[13] * b->m[4] + a->m[14] * b->m[8] + a->m[15] * b->m[12];
    out->m[13] = a->m[12] * b->m[1] + a->m[13] * b->m[5] + a->m[14] * b->m[9] + a->m[15] * b->m[13];
    out->m[14] = a->m[12] * b->m[2] + a->m[13] * b->m[6] + a->m[14] * b->m[10] + a->m[15] * b->m[14];
    out->m[15] = a->m[12] * b->m[3] + a->m[13] * b->m[7] + a->m[14] * b->m[11] + a->m[15] * b->m[15];
}

extern "C" void PostMult(Mtx4* a, Mtx4* b)
{
    Mtx4 t;
    MultMatrix(a, b, &t);
    MEM_copy(a, &t, 0x40);
}

extern "C" void fn_803DF6AC(Mtx4* m, float x, float y, float z, float w)
{
    m->m[0] = x;
    m->m[1] = 0.0f;
    m->m[2] = 0.0f;
    m->m[3] = 0.0f;
    m->m[4] = 0.0f;
    m->m[5] = y;
    m->m[6] = 0.0f;
    m->m[7] = 0.0f;
    m->m[8] = 0.0f;
    m->m[9] = 0.0f;
    m->m[10] = z;
    m->m[11] = 0.0f;
    m->m[12] = 0.0f;
    m->m[13] = 0.0f;
    m->m[14] = 0.0f;
    m->m[15] = w;
}

extern "C" void fn_803DF6F8(Mtx4* m, float x, float y, float z)
{
    m->m[0] = 1.0f;
    m->m[1] = 0.0f;
    m->m[2] = 0.0f;
    m->m[3] = 0.0f;
    m->m[4] = 0.0f;
    m->m[5] = 1.0f;
    m->m[6] = 0.0f;
    m->m[7] = 0.0f;
    m->m[8] = 0.0f;
    m->m[9] = 0.0f;
    m->m[10] = 1.0f;
    m->m[11] = 0.0f;
    m->m[12] = x;
    m->m[13] = y;
    m->m[14] = z;
    m->m[15] = 1.0f;
}

extern "C" void fn_803DF74C(Mtx4* m)
{
    m->m[0] = 1.0f;
    m->m[1] = 0.0f;
    m->m[2] = 0.0f;
    m->m[3] = 0.0f;
    m->m[4] = 0.0f;
    m->m[5] = 1.0f;
    m->m[6] = 0.0f;
    m->m[7] = 0.0f;
    m->m[8] = 0.0f;
    m->m[9] = 0.0f;
    m->m[10] = 1.0f;
    m->m[11] = 0.0f;
    m->m[12] = 0.0f;
    m->m[13] = 0.0f;
    m->m[14] = 0.0f;
    m->m[15] = 1.0f;
}

extern "C" void mload44(Mtx4* dst, float* rot, float* trans)
{
    dst->m[0] = rot[0];
    dst->m[1] = rot[1];
    dst->m[2] = rot[2];
    dst->m[3] = 0.0f;
    dst->m[4] = rot[3];
    dst->m[5] = rot[4];
    dst->m[6] = rot[5];
    dst->m[7] = 0.0f;
    dst->m[8] = rot[6];
    dst->m[9] = rot[7];
    dst->m[10] = rot[8];
    dst->m[11] = 0.0f;
    dst->m[12] = trans[0];
    dst->m[13] = trans[1];
    dst->m[14] = trans[2];
    dst->m[15] = 1.0f;
}

extern "C" void ExtractRotTrans(Mtx4* m, float* rot, float* trans)
{
    rot[0] = m->m[0];
    rot[1] = m->m[1];
    rot[2] = m->m[2];
    rot[3] = m->m[4];
    rot[4] = m->m[5];
    rot[5] = m->m[6];
    rot[6] = m->m[8];
    rot[7] = m->m[9];
    rot[8] = m->m[10];
    trans[0] = m->m[12];
    trans[1] = m->m[13];
    trans[2] = m->m[14];
}

extern "C" void fn_803DFA2C(Mtx4* dst, Mtx4* src)
{
    *dst = *src;
}

extern "C" void fn_803DFA94(Mtx4* a, Mtx4* b)
{
    fn_803DE7D0(a, b);
}

extern "C" void fn_803DFAB4(Mtx4* m)
{
    Mtx4 t = *m;
    fn_803DE7D0(&t, m);
}

/* near-miss (same length): pairwise transpose; retail interleaves the loads and stores pair by pair
   through f8, ours hoists the loads. Tried a hill-climb over the statement order with predeclared temps. */
extern "C" void fn_803DFB48(Mtx4* m)
{
    float t4 = m->m[4];
    m->m[4] = m->m[1];
    float t8 = m->m[8];
    m->m[8] = m->m[2];
    float t12 = m->m[12];
    m->m[12] = m->m[3];
    float t9 = m->m[9];
    m->m[9] = m->m[6];
    float t13 = m->m[13];
    m->m[13] = m->m[7];
    float t14 = m->m[14];
    m->m[14] = m->m[11];
    m->m[1] = t4;
    m->m[2] = t8;
    m->m[3] = t12;
    m->m[6] = t9;
    m->m[7] = t13;
    m->m[11] = t14;
}

extern "C" void fn_803DFBAC(Mtx4* a, Mtx4* b)
{
    Mtx4 t;
    Mtx4* d = b;
    if (a == b)
        d = &t;
    d->m[0] = a->m[0];
    d->m[1] = a->m[4];
    d->m[2] = a->m[8];
    d->m[3] = a->m[12];
    d->m[4] = a->m[1];
    d->m[5] = a->m[5];
    d->m[6] = a->m[9];
    d->m[7] = a->m[13];
    d->m[8] = a->m[2];
    d->m[9] = a->m[6];
    d->m[10] = a->m[10];
    d->m[11] = a->m[14];
    d->m[12] = a->m[3];
    d->m[13] = a->m[7];
    d->m[14] = a->m[11];
    d->m[15] = a->m[15];
    if (a == b)
        *a = t;
}

extern "C" void fn_803DFCB0(Mtx4* dst, float x, float y, float z, float w)
{
    Mtx4 s;
    MtxDiag(&s, x, y, z, w);
    Mtx4 t;
    MtxMul(dst, &s, &t);
    MEM_copy(dst, &t, 0x40);
}

extern "C" void fn_803DFD40(Mtx4* dst)
{
    Mtx4 a;
    fn_803DF4E4(&a);
    Mtx4 b;
    MtxMul(dst, &a, &b);
    MEM_copy(dst, &b, 0x40);
}

extern "C" void fn_803DFD90(Mtx4* dst, float x, float y, float z)
{
    Mtx4 s;
    MtxTrans(&s, x, y, z);
    Mtx4 t;
    MtxMul(dst, &s, &t);
    MEM_copy(dst, &t, 0x40);
}

extern "C" void fn_803DFE28(Mtx4* dst, Mtx4* other)
{
    Mtx4 c = *other;
    Mtx4 t;
    MtxMul(dst, &c, &t);
    MemCopy(dst, &t, 0x40);
}

extern "C" void fn_803DFEE0(Mtx4* dst, float x, float y, float z, float w)
{
    Mtx4 s;
    MtxDiag(&s, x, y, z, w);
    fn_803E02E4(dst, &s);
}

extern "C" void fn_803DFF50(Mtx4* dst, Mtx4* other)
{
    Mtx4 c = *other;
    fn_803E02E4(dst, &c);
}

extern "C" void TransformVector(Mtx4* m, Unk803DFFE0Vec3* in, Unk803DFFE0Vec3* out)
{
    if (in != out) {
        out->x = in->x * m->m[0] + in->y * m->m[4] + in->z * m->m[8];
        out->y = in->x * m->m[1] + in->y * m->m[5] + in->z * m->m[9];
        out->z = in->x * m->m[2] + in->y * m->m[6] + in->z * m->m[10];
    } else {
        Unk803DFFE0Vec3 t;
        t.x = in->x * m->m[0] + in->y * m->m[4] + in->z * m->m[8];
        t.y = in->x * m->m[1] + in->y * m->m[5] + in->z * m->m[9];
        t.z = in->x * m->m[2] + in->y * m->m[6] + in->z * m->m[10];
        *in = t;
    }
}

/* near-miss (2 instructions longer): quaternion plus translation to matrix; ours needs a second
   callee-saved float register. */
extern "C" void fn_803E01F4(Mtx4* out, float x, float y, float z, float w, float tx, float ty, float tz)
{
    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;
    float xx = x * x2;
    float yy = y * y2;
    float zz = z * z2;
    float xy = x * y2;
    float xz = x * z2;
    float yz = y * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;
    out->m[13] = ty;
    out->m[14] = tz;
    out->m[11] = 0.0f;
    out->m[3] = 0.0f;
    out->m[7] = 0.0f;
    out->m[15] = 1.0f;
    out->m[12] = tx;
    out->m[1] = xy + wz;
    out->m[2] = xz - wy;
    out->m[0] = 1.0f - (yy + zz);
    out->m[5] = 1.0f - (xx + zz);
    out->m[6] = yz + wx;
    out->m[10] = 1.0f - (xx + yy);
    out->m[4] = xy - wz;
    out->m[8] = xz + wy;
    out->m[9] = yz - wx;
}

extern "C" void fn_803E02B8(Mtx4* a, Mtx4* b, Mtx4* out)
{
    MultMatrix(b, a, out);
}

extern "C" void fn_803E02E4(Mtx4* dst, Mtx4* other)
{
    Mtx4 t;
    MultMatrix(other, dst, &t);
    MEM_copy(dst, &t, 0x40);
}

extern "C" void fn_803E032C(Unk803E032CObj* self)
{
    Unk803E032CSub* sub = (Unk803E032CSub*)((char*)self + 0x3C);
    float w = 640.0f;
    float h = 480.0f;
    float w2;
    float h2;
    fn_803D81FC();
    Unk803E0690Src* p = (Unk803E0690Src*)fn_803D817C();
    if (p)
        p->v3(&w, &h);
    sub->m14->v3(&w2, &h2);
    fn_803D81B4(fn_803D81FC() + 4, (int)self);
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    lbl_80627A80 = self->m68;
    lbl_80627A84 = self->m6C;
    fn_803DB4CC();
    int r = fn_803E0898((Unk803DE414Light*)((char*)self + 0x10));
    if (r != lbl_80627324) {
        if (r != 0)
            GXSetPixelFmt(1, 0);
        else
            GXSetPixelFmt(0, 0);
    }
    if (r != 0) {
        int d = 0;
        if (self->m70 != 0)
            d = 1;
        GXSetDither(d);
    }
}

struct Unk803E0458Info {
    char pad0[4];
    unsigned short m4;
    unsigned short m6;
    char pad8[4];
    unsigned int mC;
    int m10;
};
struct Unk803E0458Obj {
    char pad0[0x54];
    Unk803E0458Info** m54;
};
extern "C" {
void GXSetTexCopySrc(unsigned short l, unsigned short t, unsigned short w, unsigned short h);
void GXSetTexCopyDst(unsigned short w, unsigned short h, int fmt, int mipmap);
void GXSetCopyFilter(unsigned char aa, unsigned char* pattern, unsigned char vf, unsigned char* vfilter);
void GXCopyTex(void* dest, int clear);
void fn_803D9E00(void* a, int* b, unsigned char** c);
void fn_803D9268(void* p);
void GXSetPixelFmt(int fmt, int z);
void GXSetZMode(int a, int b, int c);
void GXSetColorUpdate(int a);
}

extern "C" void fn_803E0458(Unk803E0458Obj* self)
{
    Unk803E032CSub* sub = (Unk803E032CSub*)((char*)self + 0x3C);
    fn_803D81B4(fn_803D81FC() + 4, 0);
    GXSetZMode(1, 3, 1);
    GXSetColorUpdate(1);
    unsigned short w = ((Unk803E0458Info*)fn_803DD7C0(self->m54))->m4;
    unsigned short h = ((Unk803E0458Info*)fn_803DD7C0(self->m54))->m6;
    float a = 640.0f;
    float b = 480.0f;
    float c;
    float d;
    fn_803D81FC();
    Unk803E0690Src* p = (Unk803E0690Src*)fn_803D817C();
    if (p)
        p->v3(&a, &b);
    sub->m14->v3(&c, &d);
    float r;
    if (a + c <= 640.0f)
        r = a;
    else
        r = 0.0f;
    GXSetTexCopySrc((unsigned short)(int)r, 0, w, h);
    GXSetTexCopyDst(w, h, ((int*)self->m54)[3], 0);
    int x;
    unsigned char* y;
    fn_803D81FC();
    fn_803D9E00((char*)fn_803D817C() + 0x10, &x, &y);
    GXSetCopyFilter(y[0x19], y + 0x1A, 0, y + 0x32);
    Unk803E0458Info* t = (Unk803E0458Info*)fn_803DD7C0(self->m54);
    void* dst;
    if (t->mC & 0x10000000)
        dst = (char*)t + t->m10;
    else
        dst = (char*)t + 0x10;
    GXCopyTex(dst, 1);
    GXSetCopyFilter(y[0x19], y + 0x1A, 1, y + 0x32);
    int r2 = fn_803E0898((Unk803DE414Light*)((char*)self + 0x10));
    if (r2 != lbl_80627324) {
        if (lbl_80627324 != 0)
            GXSetPixelFmt(1, 0);
        else
            GXSetPixelFmt(0, 0);
    }
    fn_803D81FC();
    fn_803D9268((char*)fn_803D817C() + 0x38);
}

extern "C" void fn_803E0658(int a, int b)
{
    if (b == 0xFFFF && a != 0) {
        lbl_8062C270[0] = 4;
        lbl_8062C270[1] = 4;
        lbl_8062C270[2] = 1;
    }
}

extern "C" float fn_803E0690(Unk803E0690Obj* self)
{
    float w = 640.0f;
    float h = 480.0f;
    float w2;
    float h2;
    fn_803D81FC();
    Unk803E0690Src* p = (Unk803E0690Src*)fn_803D817C();
    if (p)
        p->v3(&w, &h);
    self->m14->v3(&w2, &h2);
    float r = w;
    if (!(w + w2 <= 640.0f))
        r = 0.0f;
    return r;
}

extern "C" void fn_803E074C(Unk803DE414Tex* self, float* w, float* h)
{
    *w = (float)((Unk803DE414Info*)fn_803DD7C0(self->m54))->m4;
    *h = (float)((Unk803DE414Info*)fn_803DD7C0(self->m54))->m6;
}

extern "C" int fn_803E07E0(Unk803DE414Tex* self, int v)
{
    self->m6C = v;
    lbl_80627A84 = v;
    fn_803DB4CC();
    return 1;
}

extern "C" int fn_803E0810(Unk803DE414Tex* self, void* v)
{
    self->m54 = v;
    return 1;
}

extern "C" Unk803DE414Light* fn_803E081C(Unk803DE414Light* self, void* owner)
{
    fn_803D9B6C(self, owner);
    self->m20 = owner;
    self->m1C = lbl_80686848;
    self->m24 = 0;
    return self;
}

extern "C" void fn_803E0868(Unk803DE414Light* self, int c)
{
    self->m1C = lbl_80686848;
    fn_803D9BAC(self, c);
}

extern "C" int fn_803E0898(Unk803DE414Light* self)
{
    if (self->m24 != 0)
        return self->m28;
    return lbl_80627324;
}

extern "C" Unk803DE414Obj* fn_803E08B8(Unk803DE414Obj* self, void* owner)
{
    fn_803D99C8(self, owner);
    self->m10 = lbl_80686830;
    self->m14 = owner;
    self->m1C = 0;
    self->m20 = 0;
    self->m24 = 0;
    self->m28 = 0;
    self->m2C = 0;
    self->m30 = 1;
    self->m34 = 1;
    return self;
}

extern "C" void fn_803E0920(Unk803DE414Obj* self, int c)
{
    self->m10 = lbl_80686830;
    fn_803D99F0(self, c);
}

extern "C" void fn_803E0950()
{
    fn_803E0658(1, 0xFFFF);
}

/*
 * Not matched yet: fn_803DE414 (quaternion to 3x3 temporary to matrix, stack slot and loads right, float
 * schedule 49 slots off), fn_803E00EC (scaled quaternion to matrix, needs a fourth saved float register),
 * fn_803DE7D0, fn_803DF4E4 (axis-angle to matrix), fn_803DF824 (euler angles to matrix).
 * The original code stays linked for these.
 */
